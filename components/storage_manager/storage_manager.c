#include "storage_manager.h" // Incluido o arquivo de cabeçalho


esp_err_t init_storage_manager(void){ // Função que inicializa o storage manager
    ESP_LOGI(StorageTAG, "Iniciando NVS");

    esp_err_t ret = nvs_flash_init(); // Inicialização do armazenamento de dados persistentes
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) { // Verificação de erro
        ESP_ERROR_CHECK(nvs_flash_erase()); // Erro de erase do armazenamento de dados persistentes
        ret = nvs_flash_init(); // Inicialização do armazenamento de dados persistentes
    }
    ESP_ERROR_CHECK(ret); // Verificação de erro

    ESP_LOGI(StorageTAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;

    ret = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (ret != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
        return ret;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    int32_t boots = 0; // Variável que armazena o número de boots

    nvs_get_i32(my_handle, "boots", &boots); // Recupera o número de boots

    if(boots == 0){ // Verificação de erro
        ESP_LOGI(StorageTAG, "Contador de boots não encontrado");
    } else {
        boots++; // Incrementa o número de boots
        ESP_LOGI(StorageTAG, "Boots: %d", boots); // Imprime o número de boots
        nvs_set_i32(my_handle, "boots", boots); // Armazena o número de boots
    }

    nvs_commit(my_handle); // Armazena as alterações no storage

    nvs_close(my_handle); // Fecha o handle do armazenamento de dados persistentes

    ESP_LOGI(StorageTAG, "Iniciando SPIFFS"); // Log de inicialização do storage manager

    esp_vfs_spiffs_conf_t conf = { // Define a configuração do storage manager
      .base_path = "/spiffs", // Caminho base
      .partition_label = NULL, // Nome da partição
      .max_files = 12, // Número máximo de arquivos
      .format_if_mount_failed = true // Formatar se falhar a montagem
    };

    // Use as configurações definidas acima para inicializar e montar o sistema de arquivos SPIFFS.
    // Nota: esp_vfs_spiffs_register é uma função de conveniência multifuncional.
    ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) { // Verifica se o sistema de arquivos SPIFFS foi inicializado com sucesso
        if (ret == ESP_FAIL) { // Se o sistema de arquivos SPIFFS não foi inicializado com sucesso
            ESP_LOGE(StorageTAG, "Falha ao montar ou formatar o sistema de arquivos"); // Log de erro
        } else if (ret == ESP_ERR_NOT_FOUND) { // Se o sistema de arquivos SPIFFS não foi inicializado com sucesso
            ESP_LOGE(StorageTAG, "Falha ao encontrar partição SPIFFS"); // Log de erro
        } else { // Se o sistema de arquivos SPIFFS não foi inicializado com sucesso
            ESP_LOGE(StorageTAG, "Falha ao inicializar SPIFFS (%s)", esp_err_to_name(ret)); // Log de erro
        }
        return ESP_FAIL; // Finaliza a função
    }

    size_t total = 0, used = 0; // Variáveis de controle do tamanho total e usado do SPIFFS
    ret = esp_spiffs_info(conf.partition_label, &total, &used); // Obtém o tamanho total e usado do SPIFFS
    if (ret != ESP_OK) { // Verifica se o tamanho total e usado do SPIFFS foi obtido com sucesso
        ESP_LOGE(StorageTAG, "Falha ao obter informações da partição SPIFFS (%s)", esp_err_to_name(ret)); // Log de erro
        return ESP_FAIL; // Finaliza a função
    } else { // Se o tamanho total e usado do SPIFFS foi obtido com sucesso
        ESP_LOGI(StorageTAG, "Tamanho da partição: total: %d, usado: %d", total, used); // Log de informações
    }

    return ESP_OK; // Retorna sem erro
}


esp_err_t read_file_size(const char *filename, size_t *size){
    if(filename == NULL || size == NULL){ // Verificação de erro
        ESP_LOGE(StorageTAG, "Erro: argumentos inválidos"); // Log de erro
        return ESP_ERR_INVALID_ARG; // Finaliza a função
    }

    char *file_path = (char*)malloc(8+strlen(filename) + 1); // Cria uma string do caminho do arquivo

    // Define o caminho do arquivo

    sprintf(file_path, "/spiffs/%s", filename); // Caminho do arquivo

    FILE* fp = fopen(file_path, "r"); // Abre o arquivo de leitura

    if (fp == NULL) { // Verifica se o arquivo de leitura foi aberto com sucesso
        ESP_LOGE(StorageTAG, "Falha ao abrir o arquivo %s", filename); // Log de erro
        return ESP_FAIL; // Finaliza a função
    }

    fseek(fp, 0, SEEK_END); // Posiciona o ponteiro do arquivo no final do arquivo

    *size = ftell(fp); // Obtém o tamanho do arquivo

    fclose(fp); // Fecha o arquivo de leitura

    free(file_path); // Libera a memória alocada para a string do caminho do arquivo

    return ESP_OK; // Retorna sem erro
}


esp_err_t read_file(const char* file_name, char* buffer, size_t buffer_size){ // Função que lê um arquivo
    // Verifica se o buffer de leitura e o buffer de escrita são válidos
    if (file_name == NULL || buffer == NULL || buffer_size == 0) { // Se não, retorna erro
        return ESP_ERR_INVALID_ARG; // Retorna erro
    }

    char *file_path = (char*)malloc(8+strlen(file_name) + 1); // Cria uma string do caminho do arquivo

    // Define o caminho do arquivo

    sprintf(file_path, "/spiffs/%s", file_name); // Caminho do arquivo

    FILE* fp = fopen(file_path, "r"); // Abre o arquivo de leitura

    free(file_path); // Libera a memória alocada para a string do caminho do arquivo

    if (fp == NULL) { // Verifica se o arquivo de leitura foi aberto com sucesso
        ESP_LOGE(StorageTAG, "Falha ao abrir o arquivo %s", file_name); // Log de erro
        return ESP_FAIL; // Finaliza a função
    }

    fread(buffer, sizeof(char), buffer_size, fp); // Lê o arquivo

    fclose(fp); // Fecha o arquivo de leitura

    return ESP_OK; // Retorna sem erro
}

esp_err_t write_file(const char* file_name, const char* buffer, size_t buffer_size){ // Função que escreve o buffer de leitura no arquivo de configuração
    if (file_name == NULL) { // Se não, retorna erro
        return ESP_ERR_INVALID_ARG; // Retorna erro
    }

    char *file_path = (char*)malloc(8+strlen(file_name) + 1); // Cria uma string do caminho do arquivo

    // Define o caminho do arquivo

    sprintf(file_path, "/spiffs/%s", file_name); // Caminho do arquivo

    FILE* fp = fopen(file_path, "w+"); // Abre o arquivo de configuração

    free(file_path); // Libera a memória alocada para a string do caminho do arquivo

    if (fp == NULL) { // Verifica se o arquivo de configuração foi aberto com sucesso
        ESP_LOGE(StorageTAG, "Falha ao abrir o arquivo %s", file_name); // Log de erro
        return ESP_FAIL; // Finaliza a função
    }

    fputs(buffer, fp); // Escreve o buffer no arquivo de configuração

    fclose(fp); // Fecha o arquivo de configuração

    return ESP_OK; // Retorna sem erro
}

esp_err_t delete_file(const char* file_name){
    if (file_name == NULL) { // Se não, retorna erro
        return ESP_ERR_INVALID_ARG; // Retorna erro
    }

    char *file_path = (char*)malloc(8+strlen(file_name) + 1); // Cria uma string do caminho do arquivo

    // Define o caminho do arquivo

    sprintf(file_path, "/spiffs/%s", file_name); // Caminho do arquivo

    int ret = remove(file_name); // Remove o arquivo de configuração

    if (ret != 0) { // Verifica se o arquivo de configuração foi removido com sucesso
        ESP_LOGE(StorageTAG, "Falha ao remover o arquivo"); // Log de erro
        return ESP_FAIL; // Finaliza a função
    }

    return ESP_OK;
}


system_config_t get_config_struct(){
    ESP_LOGI(StorageTAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READONLY, &my_handle));

    system_config_t config_st = {0};

    nvs_get_str(my_handle, "AP_SSID", NULL, &config_st.AP_SSID_length); // Lê o SSID do AP
    nvs_get_str(my_handle, "AP_Password", NULL, &config_st.AP_Password_length); // Lê a senha do AP
    nvs_get_str(my_handle, "Wifi_SSID", NULL, &config_st.Wifi_SSID_length); // Lê o SSID do Wifi
    nvs_get_str(my_handle, "Wifi_Password", NULL, &config_st.Wifi_Password_length); // Lê a senha do Wifi
    nvs_get_i8(my_handle, "MQTT_Use_URI", (int8_t *)&config_st.MQTT_Use_URI); 
    nvs_get_str(my_handle, "MQTT_Server_URI", NULL, &config_st.MQTT_Server_URI_length); 
    nvs_get_str(my_handle, "MQTT_Server_IP", NULL, &config_st.MQTT_Server_IP_length); 
    nvs_get_str(my_handle, "MQTT_USER", NULL, &config_st.MQTT_Server_USER_length); 
    nvs_get_str(my_handle, "MQTT_PASSWORD", NULL, &config_st.MQTT_Server_PASSWORD_length); 
    nvs_get_str(my_handle, "MQTT_Topic", NULL, &config_st.MQTT_Server_Topic_length); 
    nvs_get_i16(my_handle, "MQTT_Server_PORT", &config_st.MQTT_Server_PORT); 
    nvs_get_str(my_handle, "Server_USER", NULL, &config_st.Server_USER_length); 
    nvs_get_str(my_handle, "Server_PASSWORD", NULL, &config_st.Server_PASSWORD_length); 
    nvs_get_i8(my_handle, "system_mode", &config_st.system_mode); 
    nvs_get_i32(my_handle, "boots", &config_st.boots); 
    nvs_get_i8(my_handle, "update", (int8_t *)&config_st.update); 
    nvs_get_i8(my_handle, "update_hour", &config_st.update_hour);
    nvs_get_i8(my_handle, "update_interval", &config_st.update_interval); 
    nvs_get_str(my_handle, "update_url", NULL, &config_st.update_url_length);



    config_st.AP_SSID = malloc(config_st.AP_SSID_length);
    config_st.AP_Password = malloc(config_st.AP_Password_length);
    config_st.Wifi_SSID = malloc(config_st.Wifi_SSID_length);
    config_st.Wifi_Password = malloc(config_st.Wifi_Password_length);
    config_st.MQTT_Server_URI = malloc(config_st.MQTT_Server_URI_length);
    config_st.MQTT_Server_IP = malloc(config_st.MQTT_Server_IP_length);
    config_st.MQTT_Server_USER = malloc(config_st.MQTT_Server_USER_length);
    config_st.MQTT_Server_PASSWORD = malloc(config_st.MQTT_Server_PASSWORD_length);
    config_st.MQTT_Server_Topic = malloc(config_st.MQTT_Server_Topic_length);
    config_st.Server_USER = malloc(config_st.Server_USER_length);
    config_st.Server_PASSWORD = malloc(config_st.Server_PASSWORD_length);
    config_st.update_url = malloc(config_st.update_url_length);


    nvs_get_str(my_handle, "AP_SSID", config_st.AP_SSID, &config_st.AP_SSID_length); // Lê o SSID do AP
    nvs_get_str(my_handle, "AP_Password", config_st.AP_Password, &config_st.AP_Password_length); // Lê a senha do AP
    nvs_get_str(my_handle, "Wifi_SSID", config_st.Wifi_SSID, &config_st.Wifi_SSID_length); // Lê o SSID do Wifi
    nvs_get_str(my_handle, "Wifi_Password", config_st.Wifi_Password, &config_st.Wifi_Password_length); // Lê a senha do Wifi
    nvs_get_str(my_handle, "MQTT_Server_URI", config_st.MQTT_Server_URI, &config_st.MQTT_Server_URI_length); 
    nvs_get_str(my_handle, "MQTT_Server_IP", config_st.MQTT_Server_IP, &config_st.MQTT_Server_IP_length); 
    nvs_get_str(my_handle, "MQTT_USER", config_st.MQTT_Server_USER, &config_st.MQTT_Server_USER_length); 
    nvs_get_str(my_handle, "MQTT_PASSWORD", config_st.MQTT_Server_PASSWORD, &config_st.MQTT_Server_PASSWORD_length); 
    nvs_get_str(my_handle, "MQTT_Topic", config_st.MQTT_Server_Topic, &config_st.MQTT_Server_Topic_length); 
    nvs_get_str(my_handle, "Server_USER", config_st.Server_USER, &config_st.Server_USER_length); 
    nvs_get_str(my_handle, "Server_PASSWORD", config_st.Server_PASSWORD, &config_st.Server_PASSWORD_length); 
    nvs_get_str(my_handle, "update_url", config_st.update_url, &config_st.update_url_length); 

    nvs_close(my_handle);

    return config_st;
}

esp_err_t write_config_struct(system_config_t config_st){
    // Open
    ESP_LOGI(StorageTAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    // Write

    ESP_LOGI(StorageTAG, "Writing AP_SSID... ");
    nvs_set_str(my_handle, "AP_SSID", config_st.AP_SSID);
    nvs_set_str(my_handle, "AP_Password", config_st.AP_Password);
    nvs_set_str(my_handle, "Wifi_SSID", config_st.Wifi_SSID);
    nvs_set_str(my_handle, "Wifi_Password", config_st.Wifi_Password);
    nvs_set_str(my_handle, "MQTT_Server_URI", config_st.MQTT_Server_URI);
    nvs_set_str(my_handle, "MQTT_Server_IP", config_st.MQTT_Server_IP);
    nvs_set_str(my_handle, "MQTT_USER", config_st.MQTT_Server_USER);
    nvs_set_str(my_handle, "MQTT_PASSWORD", config_st.MQTT_Server_PASSWORD);
    nvs_set_str(my_handle, "MQTT_Topic", config_st.MQTT_Server_Topic);
    nvs_set_str(my_handle, "Server_USER", config_st.Server_USER);
    nvs_set_str(my_handle, "Server_PASSWORD", config_st.Server_PASSWORD);
    nvs_set_str(my_handle, "update_url", config_st.update_url);
    nvs_set_i8(my_handle, "MQTT_Use_URI", config_st.MQTT_Use_URI);
    nvs_set_i16(my_handle, "MQTT_Server_PORT", config_st.MQTT_Server_PORT);
    nvs_set_i8(my_handle, "update", config_st.update);
    nvs_set_i8(my_handle, "update_hour", config_st.update_hour);
    nvs_set_i8(my_handle, "update_interval", config_st.update_interval);

    // Commit written value.
    
    nvs_commit(my_handle);

    nvs_close(my_handle);

    return ESP_OK;
}

esp_err_t delete_config(bool is_first_boot){
 
    ESP_LOGI(StorageTAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    nvs_set_str(my_handle, "AP_SSID", "Estufa Alfa-ARI");
    nvs_set_str(my_handle, "AP_Password", "1234567890");
    nvs_set_str(my_handle, "Wifi_SSID", ".");
    nvs_set_str(my_handle, "Wifi_Password", ".");
    nvs_set_i8(my_handle, "MQTT_Use_URI", 1);
    nvs_set_str(my_handle, "MQTT_Server_URI", "mqtt://broker.hivemq.com");
    nvs_set_str(my_handle, "MQTT_Server_IP", "0.0.0.0");
    nvs_set_str(my_handle, "MQTT_USER", "admin");
    nvs_set_str(my_handle, "MQTT_PASSWORD", "admin");
    nvs_set_i16(my_handle, "MQTT_Server_PORT", 1883);
    nvs_set_str(my_handle, "Server_USER", "admin");
    nvs_set_str(my_handle, "Server_PASSWORD", "admin");
    nvs_set_i8(my_handle, "system_mode", 1); // 2 = Modo STA, 1 = Modo AP, 0 = Modo STA
    nvs_set_i8(my_handle, "update", 0);
    nvs_set_str(my_handle, "update_url", "http://192.168.0.106:8000/update/1/release.esp32.bin");
    nvs_set_i8(my_handle, "update_hour", 0);
    nvs_set_i8(my_handle, "update_interval", 15);
    nvs_set_str(my_handle, "MQTT_Topic", "aridevice/data/");


    if(is_first_boot == true){
        nvs_set_i32(my_handle, "boots", 1);
    }

    ESP_ERROR_CHECK(nvs_commit(my_handle));

    nvs_close(my_handle);

    return ESP_OK;
}

esp_err_t read_string_size_from_nvs(const char *key, size_t *length){ // Lê o tamanho de uma string do NVS
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    err = nvs_get_str(my_handle, key, NULL, length);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) reading NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    nvs_close(my_handle);

    return ESP_OK;
}

esp_err_t read_string_from_nvs(const char *key, char *value, size_t *lenght){ // Lê uma string do NVS
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    err = nvs_get_str(my_handle, key, value, lenght);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) reading NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    nvs_close(my_handle);

    return ESP_OK;
}

esp_err_t write_string_to_nvs(const char *key, char *value){ // Escreve uma string no NVS
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    err = nvs_set_str(my_handle, key, value);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) writing NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) committing NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    nvs_close(my_handle);

    return ESP_OK;
}

esp_err_t read_int_from_nvs(const char *key, int32_t *value){ // Lê um inteiro do NVS
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    err = nvs_get_i32(my_handle, key, value);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) reading NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    nvs_close(my_handle);

    return ESP_OK;
}

esp_err_t write_int_to_nvs(const char *key, int32_t value){ // Escreve um inteiro no NVS
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    err = nvs_set_i32(my_handle, key, value);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) writing NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) committing NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    nvs_close(my_handle);

    return ESP_OK;
}

// int 16

esp_err_t read_int16_from_nvs(const char *key, int16_t *value){ // Lê um inteiro do NVS
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    err = nvs_get_i16(my_handle, key, value);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) reading NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    nvs_close(my_handle);

    return ESP_OK;
}

esp_err_t write_int16_to_nvs(const char *key, int16_t value){ // Escreve um inteiro no NVS
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    err = nvs_set_i16(my_handle, key, value);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) writing NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) committing NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    nvs_close(my_handle);

    return ESP_OK;
}


esp_err_t read_bool_from_nvs(const char *key, int8_t *value){ // Lê um bool do NVS
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    err = nvs_get_i8(my_handle, key, (int8_t *)value);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) reading NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    nvs_close(my_handle);

    return ESP_OK;
}

esp_err_t write_bool_to_nvs(const char *key, int8_t value){ // Escreve um bool no NVS
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    err = nvs_set_i8(my_handle, key, (int8_t)value);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) writing NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) committing NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    nvs_close(my_handle);

    return ESP_OK;
}

esp_err_t delete_item_from_nvs(const char *key){ // Deleta um bool do NVS
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    err = nvs_erase_key(my_handle, key);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) erasing NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(StorageTAG, "Error (%s) committing NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(StorageTAG, "Done");
    }

    nvs_close(my_handle);

    return ESP_OK;
}

bool storage_manager_is_first_boot(){ // Verifica se e a primeira vez que o sistema e inciado
    ESP_LOGI(StorageTAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle; // Handle para o NVS
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle); // Abre o handle
    if (err != ESP_OK) { // Verifica se ocorreu algum erro
        ESP_LOGI(StorageTAG, "Error (%s) opening NVS handle!", esp_err_to_name(err)); // Se ocorreu, imprime o erro
        return err; // Retorna o erro
    } else {
        ESP_LOGI(StorageTAG, "Done"); // Se nao ocorreu, imprime que foi feito com sucesso
    }

    int32_t boots = 0; // Numero de boot do sistema

    nvs_get_i32(my_handle, "boots", &boots); // Lê o número de vezes que o sistema foi iniciado

    nvs_close(my_handle); // Fecha o handle

    if(boots == 0){ // Se o número de boot for igual a 0, então é a primeira vez que o sistema é iniciado
        return true; // Retorna true
    } else { // Se não for a primeira vez que o sistema é iniciado
        return false; // Retorna false
    }
}
