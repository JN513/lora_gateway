#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h> // Inclui biblioteca de funções de tempo
#include <esp_sntp.h> // Inclui biblioteca de funções de sistema de tempo
#include <sys/time.h> // Inclui biblioteca de funções de tempo
#include "esp_system.h" // Inclui biblioteca de funções de sistema
#include "esp_event.h" // Inclui biblioteca de funções de eventos
#include "esp_log.h" // Inclui biblioteca de funções de log


static const char* TimeTAG = "TIME_MANAGER"; // Tag de log
    
// Função: time_manager_init
// Descrição: Inicializa o sistema de tempo
// Retorno: ESP_OK - Inicialização bem sucedida ou ESP_FAIL - Erro na inicialização

esp_err_t time_manager_init(void);

// Função: sync_time
// Descrição: Sincroniza o sistema de tempo com o servidor NTP
// Retorno: ESP_OK - Sincronização bem sucedida ou ESP_FAIL - Erro na sincronização

esp_err_t sync_time(void);

// Função: get_time
// Descrição: Obtém a hora atual do sistema
// Parametros: time_t *time - Ponteiro para o tempo atual
// Retorno: ESP_OK - Inicialização bem sucedida ou ESP_FAIL - Erro na inicialização

esp_err_t get_time(time_t *time_p);

// Função: get_time_str
// Descrição: Obtém a hora atual do sistema em formato string
// Parametros: char *time_str - Ponteiro para a string de hora
// Retorno: ESP_OK - Inicialização bem sucedida ou ESP_FAIL - Erro na inicialização

esp_err_t get_time_str(char *time_str);

#ifdef __cplusplus
}
#endif

#endif
