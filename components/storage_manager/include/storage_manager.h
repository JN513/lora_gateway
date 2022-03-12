#ifndef STORAGE_MANAGER_H_
#define STORAGE_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

// Incluindo Bibliotecas
#include <stdio.h>
#include <string.h>

#include <sys/unistd.h>
#include <sys/stat.h>

#include "esp_system.h" // Incluindo a biblioteca de configuração do sistema
#include "esp_log.h" // Incluindo a biblioteca de log

#include "esp_spiffs.h" // Incluindo a biblioteca de SPIFFS
#include "nvs_flash.h" 
#include "nvs.h"

typedef struct {
    char *AP_SSID; // Nome da rede
    char *AP_Password; // Senha da rede

    char *Wifi_SSID; // Nome da rede
    char *Wifi_Password; // Senha da rede

    bool MQTT_Use_URI; // Indica se o URI será usado para conexão MQTT
    char *MQTT_Server_URI; // URI do servidor MQTT
    char *MQTT_Server_IP; // IP do servidor MQTT
    char *MQTT_Server_USER; // Usuário do servidor MQTT
    char *MQTT_Server_PASSWORD; // Senha do servidor MQTT
    char *MQTT_Server_Topic;
    int16_t MQTT_Server_PORT; // Porta do servidor 

    char *Server_USER; // Usuário do servidor
    char *Server_PASSWORD; // Senha do servidor

    int8_t system_mode; // Modo do sistema
    int32_t boots; // Contador de inicializações do sistema

    bool update; // Indica se o sistema deve atualizar
    int8_t update_hour; // Hora de atualização
    char *update_url; // URL de atualização
    int8_t update_interval; // Intervalo de atualização

    size_t AP_SSID_length; // Tamanho da string de nome da rede
    size_t AP_Password_length; // Tamanho da string de senha da rede
    size_t Wifi_SSID_length; // Tamanho da string de nome da rede de conexão
    size_t Wifi_Password_length; // Tamanho da string de senha da rede de conexão
    size_t MQTT_Server_URI_length; // Tamanho da string de URI do servidor MQTT
    size_t MQTT_Server_IP_length; // Tamanho da string de IP do servidor MQTT
    size_t MQTT_Server_USER_length; // Tamanho da string de usuário do servidor MQTT
    size_t MQTT_Server_PASSWORD_length; // Tamanho da string de senha do servidor MQTT
    size_t MQTT_Server_Topic_length; // Tamanho da string de tópico do servidor MQTT
    size_t Server_USER_length; // Tamanho da string de usuário do servidor
    size_t Server_PASSWORD_length; // Tamanho da string de senha do servidor
    size_t update_url_length; // Tamanho da string de URL de atualização

} system_config_t;

static const char* StorageTAG = "storage_manager"; // Tag para Log

// Função: init_storage_manager
// Descrição: Inicializa o Storage Manager (SPIFFS e NVS)
// Parametros: Nenhum
// Retorno: ESP_OK - Inicializado com sucesso | ESP_FAIL - Erro ao inicializar

esp_err_t init_storage_manager(void);

// Função: read_file_size
// Descrição: Lê o tamanho um arquivo do SPIFFS
// Parametros:
// 		- file_name: Nome do arquivo a ser lido
// 		- size: Ponteiro para o tamanho do arquivo
// Retorno: ESP_OK - Tamanho do arquivo lido com sucesso | ESP_FAIL - Erro ao ler o tamanho arquivo

esp_err_t read_file_size(const char *filename, size_t *size);

// Função: read_file
// Descrição: Lê um arquivo do SPIFFS
// Parametros:
// 		- file_name: Nome do arquivo a ser lido
// 		- buffer: Ponteiro para a string onde sera armazenado o conteudo do arquivo
//      - buffer_size: Tamanho do buffer
// Retorno: ESP_OK - Arquivo lido com sucesso | ESP_FAIL - Erro ao ler o arquivo

esp_err_t read_file(const char* file_name, char* buffer, size_t buffer_size);

// Função: write_file
// Descrição: Escreve um arquivo no SPIFFS
// Parametros:
// 		- file_name: Nome do arquivo a ser escrito
// 		- buffer: Ponteiro para a string que sera escrita no arquivo
//      - buffer_size: Tamanho do buffer
// Retorno: ESP_OK - Arquivo escrito com sucesso | ESP_FAIL - Erro ao escrever o arquivo

esp_err_t write_file(const char* file_name, const char* buffer, size_t buffer_size);

// Função: delete_file
// Descrição: Deleta um arquivo do SPIFFS
// Parametros:
// 		- file_name: Nome do arquivo a ser deletado
// Retorno: ESP_OK - Arquivo deletado com sucesso | ESP_FAIL - Erro ao deletar o arquivo

esp_err_t delete_file(const char* file_name); 

// Função: get_config_struct
// Descrição: Le as configurações e retorna em um struct
// Parametros: Nenhum
// Retorno: Ponteiro para o struct com as configurações do sistema | NULL - Erro ao ler as configurações

system_config_t get_config_struct();

// Função: write_config_struct
// Descrição: Le grava as configurações atraves de um struct
// Parametros:
// 		- storage_manager: Struct com os valores que seram gravados
// Retorno: ESP_OK - Configurações gravadas com sucesso | ESP_FAIL - Erro ao gravar as configurações

esp_err_t write_config_struct(system_config_t storage_manager);

// Função: delete_config
// Descrição: Deleta as configurações e retorna a configuração para o padrão de fabrica
// Parametros: Nenhum
// Retorno: ESP_OK - Tamanho lido com sucesso | ESP_FAIL - Erro ao ler o tamanho

esp_err_t delete_config(bool is_first_boot); // Apagando o arquivo de configuração

// Função: storage_manager_is_first_boot
// Descrição: Verifica se e a primeira vez que o sistema é iniciado
// Parametros:
// 		- is_first_boot: Ponteiro para a variavel que indica se é a primeira vez que o sistema é iniciado
// Retorno: true - Primeira vez que o sistema é iniciado | false - Não é a primeira vez que o sistema é iniciado

esp_err_t read_string_size_from_nvs(const char *key, size_t *length);

// Função: read_string_size_from_nvs
// Descrição: Lê o tamanho de uma string salva na NVS
// Parametros:
// 		- key: Ponteiro para a string que indica o nome da chave
//      - length: Ponteiro para a variavel que recebera o tamanho da string
// Retorno: ESP_OK - Tamanho lido com sucesso | ESP_FAIL - Erro ao ler o tamanho da string

esp_err_t read_string_from_nvs(const char *key, char *value, size_t *lenght);

// Função: read_string_from_nvs
// Descrição: Lê uma string salva na NVS
// Parametros:
// 		- key: Ponteiro para a string que indica o nome da chave
//      - value: Ponteiro para a string que recebera a string salva na NVS
//      - lenght: Ponteiro para a variavel com o tamanho da string
// Retorno: ESP_OK - String lida com sucesso | ESP_FAIL - Erro ao ler a string

esp_err_t write_string_to_nvs(const char *key, char *value);

// Função: write_string_to_nvs
// Descrição: Escreve uma string na NVS
// Parametros:
// 		- key: Ponteiro para a string que indica o nome da chave
//      - value: Ponteiro para a string que sera salva na NVS
// Retorno: ESP_OK - String escrita com sucesso | ESP_FAIL - Erro ao escrever a string

esp_err_t read_int_from_nvs(const char *key, int32_t *value);

// Função: read_int_from_nvs
// Descrição: Lê um inteiro salvo na NVS
// Parametros:
// 		- key: Ponteiro para a string que indica o nome da chave
//      - value: Ponteiro para a variavel que recebera o inteiro salvo na NVS
// Retorno: ESP_OK - Inteiro lido com sucesso | ESP_FAIL - Erro ao ler o inteiro

esp_err_t write_int_to_nvs(const char *key, int32_t value);

// Função: write_int_to_nvs
// Descrição: Escreve um inteiro na NVS
// Parametros:
// 		- key: Ponteiro para a string que indica o nome da chave
//      - value: Valor que sera salvo na NVS
// Retorno: ESP_OK - Inteiro escrito com sucesso | ESP_FAIL - Erro ao escrever o inteiro

esp_err_t read_int16_from_nvs(const char *key, int16_t *value);

// Função: read_int_from_nvs
// Descrição: Lê um inteiro salvo na NVS
// Parametros:
// 		- key: Ponteiro para a string que indica o nome da chave
//      - value: Ponteiro para a variavel que recebera o inteiro salvo na NVS
// Retorno: ESP_OK - Inteiro lido com sucesso | ESP_FAIL - Erro ao ler o inteiro

esp_err_t write_int16_to_nvs(const char *key, int16_t value);

// Função: write_int_to_nvs
// Descrição: Escreve um inteiro na NVS
// Parametros:
// 		- key: Ponteiro para a string que indica o nome da chave
//      - value: Valor que sera salvo na NVS
// Retorno: ESP_OK - Inteiro escrito com sucesso | ESP_FAIL - Erro ao escrever o inteiro

esp_err_t read_bool_from_nvs(const char *key, int8_t *value);

// Função: read_bool_from_nvs
// Descrição: Lê um booleano salvo na NVS
// Parametros:
// 		- key: Ponteiro para a string que indica o nome da chave
//      - value: Ponteiro para a variavel que recebera o booleano salvo na NVS
// Retorno: ESP_OK - Booleano lido com sucesso | ESP_FAIL - Erro ao ler o booleano

esp_err_t write_bool_to_nvs(const char *key, int8_t value);

// Função: write_bool_to_nvs
// Descrição: Escreve um booleano na NVS
// Parametros:
// 		- key: Ponteiro para a string que indica o nome da chave
//      - value: Valor que sera salvo na NVS
// Retorno: ESP_OK - Booleano escrito com sucesso | ESP_FAIL - Erro ao escrever o booleano

esp_err_t delete_item_from_nvs(const char *key);

// Função: delete_item_from_nvs
// Descrição: Deleta um valor salvo na NVS
// Parametros:
// 		- key: Ponteiro para a string que indica o nome da chave
// Retorno: ESP_OK - Item deletado com sucesso | ESP_FAIL - Erro ao deletar o item

bool storage_manager_is_first_boot();

// Função: storage_manager_is_first_boot
// Descrição: Verifica se e a primeira vez que o sistema é iniciado
// Parametros: Nenhum
// Retorno: true - Primeira vez que o sistema é iniciado | false - Não é a primeira vez que o sistema é iniciado

#ifdef __cplusplus
}
#endif

#endif