#include <stdio.h>
#include "time_manager.h"

    
void time_sync_notification_cb(struct timeval *tv){ // Notificação de sincronização de tempo
    ESP_LOGI(TimeTAG, "Tempo sinclonizado com sucesso."); // Log de sincronização de tempo
}

static void initialize_sntp(void) { // Inicialização do SNTP
    ESP_LOGI(TimeTAG, "Iniciando SNTP"); // Log de inicialização do SNTP
    sntp_setoperatingmode(SNTP_OPMODE_POLL); // Modo de operação do SNTP
    sntp_setservername(0, "pool.ntp.org"); // Adicionando servidor de nome do SNTP
    sntp_set_time_sync_notification_cb(time_sync_notification_cb); // Adicionando callback de sincronização de tempo
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH); // Modo de sincronização do SNTP

    sntp_init(); // Inicializando o SNTP
}

static void obtain_time(void){ // Obtendo o tempo

    initialize_sntp(); // Inicializando o SNTP

    time_t now = 0; // Variável de tempo
    struct tm timeinfo = { 0 }; // Struct de tempo
    int retry = 0; // Contador de tentativas
    const int retry_count = 10; // Maximo de tentativas

    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) { // Enquanto o SNTP não estiver sincronizado e o contador de tentativas não atingir o máximo
        ESP_LOGI(TimeTAG, "Esperando que a hora do sistema seja definida ... (%d/%d)", retry, retry_count); // Log de espera de sincronização do SNTP
        vTaskDelay(2000 / portTICK_PERIOD_MS); // Esperando 2 segundos
    }

    time(&now); // Obtendo o tempo atual
    localtime_r(&now, &timeinfo); // Obtendo a hora local

    ESP_LOGI(TimeTAG, "A data/hora atual é: %04d-%02d-%02d %02d:%02d:%02d", // Log de data e hora atual
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, // Ano, mês e dia
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec); // Hora, minuto e segundo
}

esp_err_t time_manager_init(void){ // Inicialização do time_manager
    ESP_LOGI(TimeTAG, "Iniciando configuração de data e horario."); // Log de inicialização do time_manager

    time_t now; // Variável de tempo
    struct tm timeinfo; // Struct de tempo

    time(&now); // Obtendo o tempo atual
    localtime_r(&now, &timeinfo); // Obtendo a hora local

    if (timeinfo.tm_year < (2016 - 1900)) { // Se o ano for menor que o ano de 2016
        ESP_LOGI(TimeTAG, "O tempo ainda não foi definido. Conectando-se ao Wi-Fi e obtendo tempo via NTP."); // Log de tempo ainda não definido
        obtain_time(); // Obtendo o tempo

        time(&now); // Obtendo o tempo atual
    } else { // Se o ano for maior que o ano de 2016
        ESP_LOGI(TimeTAG, "O tempo já foi definido. Não é necessário obter tempo via NTP."); // Log de tempo já definido

        ESP_LOGI(TimeTAG, "Ajustando tempo"); // Log de ajuste de tempo
        obtain_time(); // Obtendo o tempo

        time(&now); // Obtendo o tempo atual
    }

    if (sntp_get_sync_mode() == SNTP_SYNC_MODE_SMOOTH) { // Se o modo de sincronização do SNTP for SMOOTH

        struct timeval outdelta; // Variável de tempo
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS) { // Enquanto o SNTP estiver sincronizando
            adjtime(NULL, &outdelta); // Ajustando o tempo
            ESP_LOGI(TimeTAG, "Esperando pelo tempo de ajuste ... outdelta = %li sec: %li ms: %li us", // Log de espera de ajuste de tempo
                        (long)outdelta.tv_sec, // Segundos e microsegundos
                        outdelta.tv_usec/1000, // Microsegundos
                        outdelta.tv_usec%1000); // Microsegundos
            vTaskDelay(2000 / portTICK_PERIOD_MS); // Esperando 2 segundos
        }
    }
    
    return ESP_OK; // Retornando o sucesso
}

esp_err_t sync_time(void){ // Sincronizando o tempo
    obtain_time(); // Obtendo o tempo
    return ESP_OK; // Retornando o sucesso
}

esp_err_t get_time(time_t *time_p){ // Obtendo o tempo
    if(time_p == NULL){ // Se o ponteiro for nulo
        return ESP_ERR_INVALID_ARG; // Retornando erro de argumento inválido
    }

    time_t now; // Variável de tempo

    time(&now); // Obtendo o tempo atual

    *time_p = now; // Atribuindo o tempo atual

    if(time_p == NULL){ // Se o ponteiro for nulo
        return ESP_FAIL; // Retornando erro de falha ao obter o tempo
    }

    return ESP_OK; // Retornando o sucesso
}

esp_err_t get_time_str(char *time_str){ // Obtendo o tempo em string
    if(time_str == NULL){ // Se o ponteiro for nulo
        return ESP_ERR_INVALID_ARG; // Retornando erro de argumento inválido
    }

    time_t now; // Variável de tempo

    time(&now); // Obtendo o tempo atual

    struct tm timeinfo; // Struct de tempo

    localtime_r(&now, &timeinfo); // Obtendo a hora local

    sprintf(time_str, "%04d-%02d-%02d %02d:%02d:%02d", // Formando a string
            timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, // Ano, mês e dia
            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec); // Hora, minuto e segundo
    
    if(time_str == NULL){ // Se o ponteiro for nulo
        return ESP_FAIL; // Retornando erro de falha ao obter o tempo
    }

    return ESP_OK; // Retornando o sucesso
}