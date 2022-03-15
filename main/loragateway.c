#include <stdio.h>

#include <stdio.h>
#include <string.h> // Inclui biblioteca de funções de manipulação de strings

#include "esp_system.h" // Biblioteca de funções do sistema
#include "esp_event.h" // Biblioteca de eventos
#include "esp_log.h" // Biblioteca de log

#include "freertos/FreeRTOS.h" // Biblioteca de sistema de tarefas
#include "freertos/task.h" // Biblioteca de tarefas
#include "freertos/semphr.h" // Biblioteca de sistema de mutex
#include "freertos/queue.h" // Biblioteca de filas

#include "soc/soc.h" // Biblioteca de configuração do chip

#include "esp_task_wdt.h" // Biblioteca de tarefas do WDT

#include "driver/gpio.h"

#include "sdkconfig.h"

#include "cJSON.h"

#include "storage_manager.h" // Biblioteca de armazenamento e gerenciamento de dados persistentes
#include "communication.h" // Biblioteca de comunicação
#include "web_server.h" // Biblioteca de servidor web
#include "time_manager.h" // Biblioteca de gerenciamento de tempo


#include "ssd1306.h"

#include "lora.h"



#define LORA_BAND 915e6 // 915 MHz
#define MAX_LORA_PAYLOAD_SIZE 255 // tamanho maximo da mensagem
#define LORA_RECEIVE_TIMEOUT 1000 // tempo de espera para receber uma mensagem
#define NETWORK_SYNC_WORD 0x12 // palavra de sincronização


static const char *TAG = "ESP"; // Tag usada para log

static xQueueHandle send_lora_queue; // Mensagens para enviar
static xQueueHandle send_mqtt_queue; // Mensagens para enviar


static bool sending = false;

typedef struct {
    uint8_t data[MAX_LORA_PAYLOAD_SIZE];
    uint8_t size;
    uint8_t node_address;
} message_t;

system_config_t system_config; // Configuração do sistema

#ifdef CONFIG_ENABLE_DISPLAY

SSD1306_t dev;
int8_t display_menu = 0;

#endif

#ifdef CONFIG_IDF_TARGET_ESP32S2

const gpio_num_t menu_pin1 = GPIO_NUM_15;
const gpio_num_t menu_pin2 = GPIO_NUM_16;

#elif CONFIG_IDF_TARGET_ESP32

const gpio_num_t menu_pin1 = GPIO_NUM_34;
const gpio_num_t menu_pin2 = GPIO_NUM_35;


#endif

void init_lora();
void start_display();
void start_wifi(); // Função de configuração do wifi
void config_time(); // Função de configuração do sistema de tempo
static void print_data_in_display_task(void *pvParameters); // Função de thread do Display
static void mqtt_task(void *pvParameters); // Função de thread do MQTT
void send_lora_task(void *pvParameters); // Função de thread do LoRa
void receive_lora_task(void *pvParameters); // Função de thread do LoRa
static void gpio_task(void *pvParameters); // Função de thread do GPIO

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO); // Define o nível de log do ESP
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_LOGI(TAG, "Iniciando Sistema"); // Log de inicialização do sistema
    ESP_LOGI(TAG, "Memoria livre: %d bytes", esp_get_free_heap_size()); // Log da quantidade de memória livre
    ESP_LOGI(TAG, "versão do IDF: %s", esp_get_idf_version()); // Log da versão do IDF

    vTaskDelay(10 / portTICK_PERIOD_MS); // Delay de 10 milissegundos

    ESP_LOGI(TAG, "Iniciando o WDT"); // Log de inicialização do WDT

    //esp_task_wdt_init(1, true); // Inicializa o WDT com 1 segundo de timeout e com reset automático

    ESP_LOGI(TAG, "Iniciando o Display"); // Log de inicialização do Display

    #ifdef CONFIG_ENABLE_DISPLAY
        start_display();
    #endif

    ESP_LOGI(TAG, "Iniciando sistema de arquivos");

    ESP_ERROR_CHECK(init_storage_manager()); // Inicializa o gerenciador de armazenamento de dados persistentes

    if(storage_manager_is_first_boot()){ // Se não for a primeira vez que o sistema é inicializado
        ESP_LOGI(TAG, "Primeira inicialização do sistema");

        #ifdef CONFIG_ENABLE_DISPLAY
            ssd1306_clear_screen(&dev, 0);
            ssd1306_display_text(&dev, 0, (char *)"Primeira", 9, 0);
            ssd1306_display_text(&dev, 1, (char *)"inicialização", 13, 0);
            ssd1306_display_text(&dev, 2, (char *)"do sistema", 11, 0);
        #endif

        ESP_ERROR_CHECK(delete_config(true)); // Apaga todas as configurações do sistema
    }

    system_config = get_config_struct(); // Obtém as configurações do sistema

    #ifdef CONFIG_IDF_TARGET_ESP32 // Se o target for ESP32
        xTaskCreatePinnedToCore(gpio_task, "GPIO", 4096, NULL, 15, NULL, tskNO_AFFINITY); // Cria a tarefa de leitura das GPIOs
    #elif CONFIG_IDF_TARGET_ESP32S2 // Se o target for ESP32S2
        xTaskCreate(gpio_task, "GPIO", 4096, NULL, 15, NULL); // Cria a tarefa de leitura das GPIOs
    #endif

    if(system_config.system_mode != 1){
        if(strcmp("_", system_config.Wifi_SSID) == 0 || strcmp("", system_config.Wifi_SSID) == 0 || system_config.Wifi_SSID == NULL){ // Se o nome da rede for igual a "_"
            system_config.system_mode = 1;
        }
    }

    ESP_LOGI(TAG, "Memoria livre apos inicialização dos sensores e leitura das configurações: %d bytes", esp_get_free_heap_size()); // Log da quantidade de memória livre

    ESP_LOGI(TAG, "Iniciando sistema de comunicação");

    communication_config_t communication_config; // Configuração de comunicação

    communication_config.wifi_mode = system_config.system_mode; // Define o modo de comunicação
    strcpy(communication_config.ap_ssid, system_config.AP_SSID); // Copia o nome da rede do AP
    strcpy(communication_config.ap_password, system_config.AP_Password); // Copia a senha da rede do AP
    strcpy(communication_config.ssid, system_config.Wifi_SSID); // Copia o nome da rede Wifi
    strcpy(communication_config.password, system_config.Wifi_Password); // Copia a senha da rede Wifi
    strcpy(communication_config.mqtt_server_uri, system_config.MQTT_Server_URI); // Copia o endereço do servidor MQTT
    strcpy(communication_config.mqtt_server_ip, system_config.MQTT_Server_IP); // Copia o endereço IP do servidor MQTT
    strcpy(communication_config.mqtt_user, system_config.MQTT_Server_USER); // Copia o usuário do servidor MQTT
    strcpy(communication_config.mqtt_password, system_config.MQTT_Server_PASSWORD); // Copia a senha do servidor MQTT
    communication_config.mqtt_server_port = system_config.MQTT_Server_PORT; // Copia a porta do servidor MQTT
    strcpy(communication_config.mqtt_topic, system_config.MQTT_Server_Topic); // Copia o tópico do servidor MQTT
    communication_config.use_uri = system_config.MQTT_Use_URI; // Define se o endereço do servidor MQTT é o URI ou o IP

    configure_communication_struct(communication_config);

    start_wifi(); // Inicialização do wifi

    ESP_LOGI(TAG, "Memoria livre apos inicialização do wifi: %d bytes", esp_get_free_heap_size()); // Log da quantidade de memória livre

    vTaskDelay(pdMS_TO_TICKS(100)); // Delay de 100ms

    ESP_LOGI(TAG, "Iniciando Web Server"); // Log de inicialização do servidor web

    ESP_ERROR_CHECK(init_web_server()); // Inicialização do servidor web

    ESP_LOGI(TAG, "Web Server Iniciado"); // Log de inicialização do servidor web

    vTaskDelay(pdMS_TO_TICKS(100)); // Delay de 100ms

    ESP_LOGI(TAG, "Memoria livre apos inicialização do sistema: %d bytes", esp_get_free_heap_size()); // Log da quantidade de memória livre

    #ifdef CONFIG_ENABLE_DISPLAY
    ssd1306_clear_screen(&dev, 0);
    ssd1306_display_text(&dev, 0, "Endereco IP:", 14, 0);
    char * ip_address = get_ip_address();
    ssd1306_display_text(&dev, 1, ip_address, strlen(ip_address), 0);

    if(system_config.system_mode == 1){
        ssd1306_display_text(&dev, 2, "Modo AP", 7, 0);
        ssd1306_display_text(&dev, 3, "SSID do AP:", 12, 0);
        ssd1306_display_text(&dev, 4, system_config.AP_SSID, strlen(system_config.AP_SSID), 0);
    }
    #endif

    send_mqtt_queue = xQueueCreate(10, sizeof(message_t)); // Cria a fila de envio de mensagens
    send_lora_queue = xQueueCreate(10, sizeof(message_t)); // Cria a fila de envio de mensagens

    if(system_config.system_mode != 1){
        config_time(); // Configuração do relógio

        #ifdef CONFIG_ENABLE_DISPLAY
        ssd1306_clear_screen(&dev, 0);
        ssd1306_display_text(&dev, 0, "Configurando", 12, 0);
        ssd1306_display_text(&dev, 1, "relógio", 7, 0);

        ssd1306_clear_screen(&dev, 0);
        ssd1306_display_text(&dev, 0, "Iniciando o MQTT", 16, 0);
        #endif

        #ifdef CONFIG_IDF_TARGET_ESP32 // Se o target for ESP32
            xTaskCreatePinnedToCore(mqtt_task, "MQTT", 4096, NULL, 10, NULL, tskNO_AFFINITY); // Cria a thread do MQTT
        #elif CONFIG_IDF_TARGET_ESP32S2 // Se o target for ESP32S2
            xTaskCreate(mqtt_task, "MQTT", 4096, NULL, 10, NULL); // Cria a tarefa de controle dos pinos GPIO
        #endif
    }

    #ifdef CONFIG_ENABLE_DISPLAY
        #ifdef CONFIG_IDF_TARGET_ESP32 // Se o target for ESP32
            xTaskCreatePinnedToCore(print_data_in_display_task, "Print Display", 4096, NULL, 8, NULL, tskNO_AFFINITY); // Cria a thread do MQTT
        #elif CONFIG_IDF_TARGET_ESP32S2 // Se o target for ESP32S2
            xTaskCreate(print_data_in_display_task, "Print Display", 4096, NULL, 8, NULL); // Cria a tarefa de controle dos pinos GPIO
        #endif
    #endif

    ESP_LOGI(TAG, "Iniciando Lora");

    init_lora(); // Inicialização do LoRa

    xTaskCreatePinnedToCore(receive_lora_task, "Receive Lora Task", 8192, NULL, 5, NULL, tskNO_AFFINITY);

    ESP_LOGI(TAG, "Memoria livre apos inicialização do sistema: %d bytes", esp_get_free_heap_size()); // Log da quantidade de memória livre
}



void init_lora(){
    lora_init();
    lora_set_frequency(LORA_BAND);
    lora_enable_crc();
    lora_set_sync_word(NETWORK_SYNC_WORD);

    ESP_LOGI(TAG, "Lora Initialized: %d", lora_initialized());

    while (!lora_initialized()) {
        printf(".");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    printf("\n");
}


void start_display(){
    #ifdef CONFIG_ENABLE_DISPLAY
    #ifdef CONFIG_IDF_TARGET_ESP32 // Se o target for ESP32
        i2c_master_init(&dev, 4, 15, 16);
    #elif CONFIG_IDF_TARGET_ESP32S2 // Se o target for ESP32S2
        i2c_master_init(&dev, 21, 26, -1);
    #endif

    ssd1306_init(&dev, 128, 64); // Inicializa o display

    ssd1306_clear_screen(&dev, false); // Limpa o display

    ESP_LOGI(TAG, "Display inicializado");
    #endif
}

void start_wifi(){
    ESP_LOGI(TAG, "Iniciando Wi-Fi"); // Log de inicialização da rede Wi-Fi

    #ifdef CONFIG_ENABLE_DISPLAY
    ssd1306_clear_screen(&dev, 0);
    ssd1306_display_text(&dev, 0, "Iniciando Wi-Fi", 16, 0);
    #endif

    while (!init_wifi()) { // Inicialização da rede Wi-Fi
        ESP_LOGI("", "."); // Log de inicialização da rede Wi-Fi

        #ifdef CONFIG_ENABLE_DISPLAY
        ssd1306_display_text(&dev, 1, ".", 1, 0); // Log de inicialização da rede Wi-Fi
        vTaskDelay(50 / portTICK_PERIOD_MS);
        ssd1306_display_text(&dev, 1, "..", 2, 0); // Log de inicialização da rede Wi-Fi
        vTaskDelay(50 / portTICK_PERIOD_MS);
        ssd1306_display_text(&dev, 1, "...", 3, 0); // Log de inicialização da rede Wi-Fi
        vTaskDelay(50 / portTICK_PERIOD_MS);
        #endif

        vTaskDelay(500 / portTICK_PERIOD_MS); // Espera 500ms

        #ifdef CONFIG_ENABLE_DISPLAY
        ssd1306_clear_line(&dev, 1, 0); // Limpa a linha do display
        #endif
    }

    ESP_LOGI(TAG, "Wi-Fi Iniciado"); // Log de inicialização da rede Wi-Fi
}

void config_time(){
    ESP_LOGI(TAG, "Configurando Time"); // Log de configuração do time

    time_manager_init(); // Inicialização do time
}


void callback (char* topic, char* payload, unsigned int topic_lenght, unsigned int length){ // Callback de recebimento de mensagem
    ESP_LOGI(TAG, "TOPIC: %.*s, PAYLOAD: %.*s", topic_lenght, topic, length, payload); // Log de debug com o topico e o payload

    char *payload_copy = (char *) malloc(sizeof(char) * (length + 1)); // Aloca memória para o payload

    sprintf(payload_copy, "%.*s", length, payload); // Copia o payload para a variável payload_copy

    if(strcmp(payload_copy, "reboot") == 0){ // Se o payload for reboot
        ESP_LOGI(TAG, "Reiniciando"); // Log de reinicialização
        esp_restart(); // Reinicia o ESP
    } else if(strcmp(payload_copy, "ping") == 0){ // Se o payload for shutdown
        send_message("ping/", "pong"); // Envia a mensagem pong
    } else {
        message_t message; // Cria a estrutura de mensagem

        message.size = length; // Define o tamanho da mensagem

        memcpy(message.data, payload, length);

        xQueueSend(send_lora_queue, &message, portMAX_DELAY); // Envia a mensagem para a fila de envio do LoRa
    }


    free(payload_copy); // Libera a memória alocada para o payload_copy
}

void on_receive(){
    u_int8_t message[MAX_LORA_PAYLOAD_SIZE];
    u_int8_t c = 0;

    while(lora_received()){
        c = lora_receive_packet(message, MAX_LORA_PAYLOAD_SIZE);
        message[c] = 0;
        lora_receive();
    }

    printf("Recebido: %s\n", message);

    message_t message_temp;

    memcpy(message_temp.data, message, c);

    if(MQTT_CONNECTED){
        xQueueSend(send_mqtt_queue, &message_temp, portMAX_DELAY);
    }
}

static void print_data_in_display_task(void *pvParameters){
    #ifdef CONFIG_ENABLE_DISPLAY
    for(;;) { // Loop Infinito
        esp_task_wdt_reset(); // Reset do WDT

        if(display_menu == 1){
            ssd1306_clear_screen(&dev, 0);
            ssd1306_display_text(&dev, 0, "Endereco IP:", 14, 0);
            char * ip_address = get_ip_address();
            ssd1306_display_text(&dev, 1, ip_address, strlen(ip_address), 0);
            char * mac_address = get_mac_address();

            if(system_config.system_mode == 1){
                ssd1306_display_text(&dev, 2, "Modo AP", 7, 0);
                ssd1306_display_text(&dev, 3, "SSID do AP:", 12, 0);
                ssd1306_display_text(&dev, 4, system_config.AP_SSID, system_config.AP_SSID_length, 0);
                ssd1306_display_text(&dev, 5, "Endereço MAC:", 15, 0);
                ssd1306_display_text(&dev, 6, mac_address, strlen(mac_address), 0);
            } else {
                ssd1306_display_text(&dev, 2, "Endereço MAC:", 15, 0);
                ssd1306_display_text(&dev, 3, mac_address, strlen(mac_address), 0);
            }

            ssd1306_display_text(&dev, 6, "Wifi Mode:", 12, 0);
            if(system_config.system_mode == 1){
                ssd1306_display_text(&dev, 7, "AP", 2, 0);
            } else {
                ssd1306_display_text(&dev, 7, "STA", 3, 0);
            }

        } else {

        }

        vTaskDelay(5000 / portTICK_PERIOD_MS); // Espera 1 segundo
    }

    #endif

    vTaskDelete(NULL); // Deleta a tarefa
}

static void mqtt_task(void *pvParameters){
    ESP_LOGI(TAG, "Iniciando Task MQTT"); // Log de inicialização da task MQTT

    ESP_LOGI(TAG, "Iniciando MQTT"); // Log de inicialização do servidor MQTT

    setcallback(callback); // Seta o callback de recebimento de mensagens

    mqtt_start(); // Inicialização do servidor MQTT

    ESP_LOGI(TAG, "MQTT Iniciado"); // Log de inicialização do servidor MQTT

    ESP_LOGI(TAG, "Iniciando Loop de envio de dados."); // Log de inicialização dos sensores

    for(;;){ // Loop Infinito
        ESP_LOGI(TAG, "Enviando dados"); // Log de envio de dados
        esp_task_wdt_reset(); // Reset do WDT

        message_t message;

        if(xQueueReceive(send_mqtt_queue, &message, portMAX_DELAY)){ // Espera receber uma mensagem na fila de envio de mensagens
            ESP_LOGI(TAG, "Enviando mensagem"); // Log de envio de mensagem

            char temp_data[message.size + 1];

            sprintf(temp_data, "%s", message.data); // Copia o payload para a variável payload_copy

            send_message("", temp_data); // Envia a mensagem
        }
        
        vTaskDelay(10000 / portTICK_PERIOD_MS); // Delay de 2s
    }

    vTaskDelete(NULL); // Deleta a task
}


void send_lora_task(void *pvParameters){
    while(1){
        esp_task_wdt_reset(); // Reset do WDT

        message_t message;

        if(xQueueReceive(send_mqtt_queue, &message, portMAX_DELAY)){ // Recebe a mensagem da fila
            ESP_LOGI(TAG, "Enviando mensagem"); // Log de envio de mensagem
            
            lora_send_packet(message.data, message.size); // Envia o pacote
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

void receive_lora_task(void *pvParameters){
    while(1){
        if(sending == false){
            lora_receive();

            if(lora_received()){
                on_receive();
            }
        } else {
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }

        vTaskDelay(100);
    }

    vTaskDelete(NULL);
}

static void gpio_task(void *pvParameters){
    ESP_LOGI(TAG, "Iniciando Task GPIO");
    #ifdef CONFIG_ENABLE_DISPLAY

    gpio_pad_select_gpio(menu_pin1);
    gpio_pad_select_gpio(menu_pin2);

    gpio_set_direction(menu_pin1, GPIO_MODE_INPUT);
    gpio_set_direction(menu_pin2, GPIO_MODE_INPUT);

    #endif

    for(;;){
        esp_task_wdt_reset(); // Reset do WDT
        #ifdef CONFIG_ENABLE_DISPLAY
        if(gpio_get_level(menu_pin1) == 1 && gpio_get_level(menu_pin2) == 1){
            if(system_config.system_mode == 1){
                write_bool_to_nvs("system_mode", 2);
            } else {
                write_bool_to_nvs("system_mode", 1);
            }

            ESP_LOGI(TAG, "Modo de rede alterado");
            esp_restart();
        }

        if(gpio_get_level(menu_pin1) == 1){
            ESP_LOGI("GPIO", "Menu 1");
            ESP_LOGI(TAG, "Display alterado");
            if(display_menu == 1){
                display_menu = 0;
            } else {
                display_menu = 1;
            }
        }
        if(gpio_get_level(menu_pin2) == 1){
            ESP_LOGI("GPIO", "Menu 2");
            
        }
        #endif

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay de 100ms
    }

    vTaskDelete(NULL); // Deleta a task
}
