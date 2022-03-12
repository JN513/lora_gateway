#include "communication.h" // Incluindo o arquivo de cabeçalho

communication_config_t communication_config; // Configurações da comunicação

esp_mqtt_client_handle_t global_client; // Cliente MQTT

int s_retry_num = 0; // Contador de tentativas de conexão

EventGroupHandle_t s_wifi_event_group; // Grupo de eventos do Wifi

//communication_config_t communication_config; // Configurações da comunicação
void (*callback)(char*, char*, unsigned int, unsigned int); // Função de callback


void event_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data) // Função de eventos do Wi-Fi
{

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) { // Se o evento for o início de conexão com Wi-Fi
        esp_wifi_connect(); // Conectando Wi-Fi
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) { // Se o evento for a desconexão do Wi-Fi
        if (s_retry_num < maximum_retry) { // Se o contador de tentativas de conexão ainda estiver abaixo da quantidade máxima
            esp_wifi_connect(); // Conectando Wi-Fi
            s_retry_num++; // Incrementa o contador de tentativas de conexão
            ESP_LOGI(CommunicationTAG, "Tentando conectar ao ponto de acesso"); // Log de debug
        } else {
            ESP_LOGE(CommunicationTAG, "Não foi possível conectar ao ponto de acesso"); // Log de debug
            write_bool_to_nvs("system_mode", 1);
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT); // Se o contador de tentativas de conexão exceder a quantidade máxima, seta o bit de falha
        }
        ESP_LOGI(CommunicationTAG,"Erro ao conectar ao ponto de acesso"); // Log de debug
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) { // Se o evento for a conexão do Wi-Fi
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data; // Obtém o evento de conexão
        ESP_LOGI(CommunicationTAG, "Endereço ip:" IPSTR, IP2STR(&event->ip_info.ip)); // Log de debug com o endereço IP
        s_retry_num = 0; // Zera o contador de tentativas de conexão
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT); // Se o evento for o início de conexão com Wi-Fi, seta o bit de conexão
    }

    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(CommunicationTAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(CommunicationTAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

bool init_wifi(){ // Inicialização do Wi-Fi
    s_wifi_event_group = xEventGroupCreate(); // Cria o grupo de eventos do Wi-Fi

    ESP_ERROR_CHECK(esp_netif_init()); // Inicializa o Wi-Fi
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Inicializa o loop de eventos
    if(communication_config.wifi_mode == 1){ // Se o modo do Wi-Fi for 1
        esp_netif_create_default_wifi_ap(); // Cria o Wi-Fi AP
    } else {
        esp_netif_create_default_wifi_sta(); // Cria o Wi-Fi STA
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // Configurações iniciais do Wi-Fi

    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // Inicializa o Wi-Fi com as configurações iniciais

    esp_event_handler_instance_t instance_any_id; // Instância de evento
    esp_event_handler_instance_t instance_got_ip; // Instância de evento
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, // Evento
                                                        ESP_EVENT_ANY_ID, // Evento de qualquer tipo
                                                        &event_handler, // Função de evento
                                                        NULL, // Parâmetros de evento
                                                        &instance_any_id)); // Registra o evento de início de conexão com Wi-Fi
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, // Evento
                                                        IP_EVENT_STA_GOT_IP, // Evento de qualquer tipo
                                                        &event_handler, // Função de evento
                                                        NULL, // Parâmetros de evento
                                                        &instance_got_ip)); // Registra o evento de conexão do Wi-Fi


    if(communication_config.wifi_mode == 1){ // Se o modo do Wi-Fi for 1
        wifi_config_t wifi_config = { // Configurações do Wi-Fi
            .ap = {
                .ssid_len = strlen(communication_config.ap_ssid),
                .max_connection = 4,
                .authmode = WIFI_AUTH_WPA_WPA2_PSK,
                .channel = 1,
            },
        };

        if (strlen(communication_config.ap_password) == 0) {
            wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        }

        strcpy((char *)wifi_config.ap.ssid, communication_config.ap_ssid); // Configurações do Wi-Fi AP
        strcpy((char *)wifi_config.ap.password, communication_config.ap_password); // Configurações do Wi-Fi AP

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP)); // Define o modo do Wi-Fi
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config)); // Define as configurações do Wi-Fi

        ESP_ERROR_CHECK(esp_wifi_start());

        ESP_LOGI(CommunicationTAG, "wifi_init_softap finished. SSID:%s channel:%d",
            communication_config.ap_ssid, 1);

        return 1;
    }

    wifi_config_t wifi_config = { // Configurações do Wi-Fi
        .sta = { // Configurações do Wi-Fi STA
            .threshold.authmode = WIFI_AUTH_WPA2_PSK, // Autenticação
            .pmf_cfg = { // Configurações do Wi-Fi STA
                .capable = true, // Capabilidade de PMF
                .required = false // Requerido
            },
        },
    };


    strcpy((char *)wifi_config.sta.ssid, communication_config.ssid); // Define o SSID do Wi-Fi
    strcpy((char *)wifi_config.sta.password, communication_config.password); // Define a senha do Wi-Fi

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); // Define o modo de operação do Wi-Fi como STA
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config)); // Define as configurações do Wi-Fi STA

    ESP_ERROR_CHECK(esp_wifi_start()); // Inicia o Wi-Fi

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, // Espera o evento de início de conexão com Wi-Fi
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, // Se o evento for o início de conexão com Wi-Fi ou se o Wi-Fi falhar, seta o bit de conexão
            pdFALSE, // Se o evento for o início de conexão com Wi-Fi ou se o Wi-Fi falhar, não espera o evento de conexão
            pdFALSE, // Se o evento for o início de conexão com Wi-Fi ou se o Wi-Fi falhar, não espera o evento de falha
            portMAX_DELAY // Timeout de espera
        ); // Espera o evento de conexão ou falha

    bool sucess = 0; // Flag de sucesso

    if (bits & WIFI_CONNECTED_BIT) { // Se o evento for o início de conexão com Wi-Fi
        ESP_LOGI(CommunicationTAG, "Conectado ao Wi-Fi %s com sucesso\n", communication_config.ssid); // Log de debug
        sucess = 1; // Se o evento for o início de conexão com Wi-Fi, define a flag de sucesso
    } else if(bits & WIFI_FAIL_BIT){ // Se o evento for a falha de conexão com Wi-Fi
        ESP_LOGI(CommunicationTAG, "Erro ao se conectar ao Wi-Fi %s\n", communication_config.ssid); // Log de debug
    } else { // Se o evento não for o início de conexão com Wi-Fi ou a falha de conexão com Wi-Fi
        ESP_LOGI(CommunicationTAG, "Erro desconhecido ao tentar realizar conexão Wi-Fi\n"); // Log de debug
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip)); // Desregistra o evento de conexão do Wi-Fi
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id)); // Desregistra o evento de início de conexão com Wi-Fi
    vEventGroupDelete(s_wifi_event_group); // Deleta o grupo de eventos do Wi-Fi
    return sucess; // Retorna o sucesso
}


// MQTT

void setcallback (void (*_callback)(char*, char*, unsigned int, unsigned int)){ // Função para definir o callback de recebimento de mensagem){
    callback = _callback; // Define o callback de recebimento de mensagem
}

void log_error_if_nonzero(const char * message, int error_code)  // Função que loga erros
{
    if (error_code != 0) { // Se o erro não for zero
        ESP_LOGE(CommunicationTAG, "Last error %s: 0x%x", message, error_code); // Log de debug com o erro e o código
    }
}

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(CommunicationTAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(CommunicationTAG, "MQTT_EVENT_CONNECTED");
        MQTT_CONNECTED = true; // Define a flag de conexão com o MQTT como true

        msg_id = esp_mqtt_client_publish(client, communication_config.mqtt_topic, "data_3", 0, 1, 0);
        ESP_LOGI(CommunicationTAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, communication_config.mqtt_topic, 0); // Subscrição ao topico
        ESP_LOGI(CommunicationTAG, "enviado inscrição com sucesso, msg_id=%d", msg_id); // Log de debug com o id da mensagem

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(CommunicationTAG, "MQTT_EVENT_DISCONNECTED");

        MQTT_CONNECTED = false; // Define a flag de conexão com o MQTT como false

        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(CommunicationTAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id); // Log de debug com o id da mensagem
            msg_id = esp_mqtt_client_publish(client, communication_config.mqtt_topic, "conectado", 0, 0, 0); // Publica mensagem no topico
            ESP_LOGI(CommunicationTAG, "sent publish successful, msg_id=%d", msg_id); // Log de debug com o id da mensagem
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(CommunicationTAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(CommunicationTAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(CommunicationTAG, "MQTT_EVENT_DATA");
        //printf("TOPIC=%.*s\r\n", event->topic_len, event->topic); // Log de debug com o topico
        //printf("DATA=%.*s\r\n", event->data_len, event->data); // Log de debug com o payload

        callback(event->topic, event->data, event->topic_len, event->data_len); // Chama o callback de recebimento de mensagem
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(CommunicationTAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(CommunicationTAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(CommunicationTAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_start(void) // Inicia o MQTT
{

    if(communication_config.use_uri){ // Se o uso de URI
        ESP_LOGI(CommunicationTAG, "MQTT: Connecting to %s", communication_config.mqtt_server_uri); // Log de debug

        esp_mqtt_client_config_t mqtt_cfg = {
            .uri = communication_config.mqtt_server_uri, // URI do servidor
            .username = communication_config.mqtt_user, // Usuário do servidor
            .password = communication_config.mqtt_password, // Senha do servidor
        }; // Configurações do MQTT

        global_client = esp_mqtt_client_init(&mqtt_cfg); // Inicializa o cliente MQTT
        esp_mqtt_client_register_event(global_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL); // Registra o callback de eventos
        esp_mqtt_client_start(global_client); // Inicia o cliente MQTT
    } else { // Se o uso de endereço IP
        ESP_LOGI(CommunicationTAG, "MQTT: Connecting to %s:%d", communication_config.mqtt_server_ip, communication_config.mqtt_server_port); // Log de debug
        esp_mqtt_client_config_t mqtt_cfg = { 
            .host = communication_config.mqtt_server_ip, // Endereço IP do servidor
            .port = communication_config.mqtt_server_port, // Porta do servidor
            .username = communication_config.mqtt_user, // Usuário do servidor
            .password = communication_config.mqtt_password, // Senha do servidor
        }; // Configurações do MQTT

        global_client = esp_mqtt_client_init(&mqtt_cfg); // Inicializa o cliente MQTT
        esp_mqtt_client_register_event(global_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL); // Registra o callback de eventos
        esp_mqtt_client_start(global_client); // Inicia o cliente MQTT
    }
}

void send_message(char *topic, char *message){ // Envia mensagem
    ESP_LOGI(CommunicationTAG, "Enviando mensagem ao topico %s: %s", topic, message); // Log de debug

    char temp_topic[100]; // Variável temporária para o topico

    strcpy(temp_topic, communication_config.mqtt_topic); // Concatena o topico
    strcat(temp_topic, topic); // Concatena o topico

    int msg_id = esp_mqtt_client_publish(global_client, temp_topic, message, 0, 0, 0); // Envia a mensagem

    ESP_LOGI(CommunicationTAG, "Mensagem enviada com sucesso, msg_id=%d", msg_id); // Log de debug
}


esp_err_t configure_communication_struct(communication_config_t config){ // Configura a comunicação

    communication_config.wifi_mode = config.wifi_mode; // Copia o modo de conexão
    strcpy(communication_config.ap_ssid, config.ap_ssid); // Copia o SSID do AP
    strcpy(communication_config.ap_password, config.ap_password); // Copia a senha do AP
    strcpy(communication_config.ssid, config.ssid); // Copia o SSID
    strcpy(communication_config.password, config.password); // Copia a senha
    strcpy(communication_config.mqtt_server_ip, config.mqtt_server_ip); // Copia o IP do servidor MQTT
    communication_config.mqtt_server_port = config.mqtt_server_port; // Copia a porta do servidor MQTT
    strcpy(communication_config.mqtt_server_uri, config.mqtt_server_uri); // Copia o URI do servidor MQTT
    strcpy(communication_config.mqtt_user, config.mqtt_user); // Copia o usuário do servidor MQTT
    strcpy(communication_config.mqtt_password, config.mqtt_password); // Copia a senha do servidor MQTT
    strcpy(communication_config.mqtt_topic, config.mqtt_topic); // Copia o topico do servidor MQTT
    communication_config.use_uri = config.use_uri; // Copia o uso de URI

    ESP_LOGI(CommunicationTAG, "Configuração de comunicação realizada com sucesso"); // Log de debug

    return ESP_OK; // Retorna o sucesso
}

char * get_ip_address(void) { // Retorna o endereço IP
    
    char *ip_address = (char *)malloc(sizeof(char) * 20); // Endereço IP

    tcpip_adapter_ip_info_t ip; // Variável para o endereço IP

    if(communication_config.wifi_mode == 1){
        ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip)); // Pega o endereço IP   
    } else {
        ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip)); // Pega o endereço IP
    }

    sprintf(ip_address, IPSTR, IP2STR(&ip.ip)); // Formata o endereço IP

    ESP_LOGI(CommunicationTAG, "IP: %s", ip_address); // Log do endereço IP

    return ip_address; // Retorna o endereço IP
}

char * get_mac_address(void) { // Retorna o endereço MAC
    char *mac_address = (char *)malloc(sizeof(char) + 20); // Endereço MAC

    uint8_t mac[6]; // Endereço MAC

    if(communication_config.wifi_mode == 1){
        ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_AP, mac)); // Obtém o endereço MAC
    } else {
        ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, mac)); // Obtém o endereço MAC
    }

    sprintf(mac_address, MACSTR, MAC2STR(mac)); // Formata o endereço MAC

    ESP_LOGI(CommunicationTAG, "MAC: %s", mac_address); // Log do endereço MAC

    return mac_address; // Retorna o endereço MAC
}