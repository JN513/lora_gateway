#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#ifdef __cplusplus
extern "C" {
#endif
// Incluindo Bibliotecas
#include "freertos/FreeRTOS.h" // FreeRTOS
#include "freertos/task.h" // Task
#include "freertos/event_groups.h" // Event Groups

#include "esp_system.h" // ESP32
#include "esp_wifi.h" // Biblioteca do Wifi
#include "esp_event.h" // Biblioteca de Eventos
#include "esp_log.h" // Biblioteca de Log

#include "lwip/err.h" // Biblioteca de Erros
#include "lwip/sys.h" // Biblioteca de Sistema

#include "mqtt_client.h" // Biblioteca MQTT

#include "storage_manager.h"

// Variaveis e objetos globais

#define WIFI_CONNECTED_BIT BIT0 // Bit 0 - Wifi conectado
#define WIFI_FAIL_BIT      BIT1 // Bit 1 - Falha na conexão
#define maximum_retry 50 // Quantidade máxima de tentativas de conexão

#define AP_MODE 1 // Modo AP
#define STATION_MODE 2 // Modo Station

typedef struct {
    int8_t wifi_mode; // Modo do Wi-Fi
    char ap_ssid[32]; // SSID do AP
    char ap_password[64]; // Senha do AP
    char ssid[32]; // SSID da rede
    char password[64]; // Senha da rede
    char mqtt_server_uri[128]; // Endereço do servidor MQTT
    char mqtt_server_ip[18]; // Endereço IP do servidor MQTT
    char mqtt_user[32]; // Usuário do servidor MQTT
    char mqtt_password[64]; // Senha do servidor MQTT
    int16_t mqtt_server_port;
    char mqtt_topic[128]; // Tópico do servidor MQTT
    bool use_uri;

} communication_config_t;

static const char * CommunicationTAG = "communication"; // Tag usada para Logs

static bool MQTT_CONNECTED; // Estado da conexão MQTT

// Funções

// Função: event_handler
// Descrição: Função para tratar os Eventos do Wifi
// Parâmetros:
//		event_base: Evento
//		event_id: ID do Evento
//		event_data: Dados do Evento
// Retorno: Nenhum

void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

// Função: init_wifi
// Descrição: Função para Inicializar o Wifi
// Parâmetros: Nenhum
// Retorno: true se conseguir inicializar o Wifi, false caso contrário

bool init_wifi(); // Função para Inicializar o Wifi

// Função: setcallback
// Descrição: Função para setar o Callback do MQTT
// Parâmetros:
//		callback: Callback
// Retorno: Nenhum

void setcallback (void (*_callback)(char*, char*, unsigned int, unsigned int));

// Função: log_error_if_nonzero
// Descrição: Função para Logar Erros
// Parâmetros:
//		message: Mensagem
//		error_code: Codigo de Erro
// Retorno: Nenhum

void log_error_if_nonzero(const char * message, int error_code); 

// Função: mqtt_event_handler
// Descrição: Função para tratar os Eventos do MQTT
// Parâmetros:
//		base: Evento
//		event_id: ID do Evento
//		event_data: Dados do Evento
// Retorno: Nenhum

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

// Função: mqtt_start
// Descrição: Função para Iniciar o MQTT
// Parâmetros: Nenhum
// Retorno: Nenhum

void mqtt_start(void);

// Função: send_message
// Descrição: Função para enviar dados via MQTT
// Parâmetros:
//		topic: Topico MQTT
//		message: Mensagem a ser enviada
// Retorno: Nenhum

void send_message(char *topic, char message[]);

// Função: configure_communication_struct
// Descrição: Função para Configurar a Comunicação através de uma Struct
// Parâmetros:
//		config: Struct com as configurações da comunicação
//      mqtt_on: Ponteiro da varivale global que vai receber o estado da conexão MQTT
// Retorno: ESṔ_OK se tudo ocorreu bem, ESP_FAIL caso contrário

esp_err_t configure_communication_struct(communication_config_t config);

// Função: get_ip_address
// Descrição: Função para obter o IP do Wifi
// Parâmetros: Nenhum
// Retorno: IP do Wifi

char * get_ip_address(void);

// Função: get_mac_address
// Descrição: Função para obter o MAC do Dispositivo
// Parâmetros: Nenhum
// Retorno: Endereço MAC do Wifi

char * get_mac_address(void);

#ifdef __cplusplus
}
#endif

#endif