#include "web_server.h"


void generate_token(){
    mbedtls_sha512_context token_ctx;

    const char *letras = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    u_int8_t max_len = 128, res = 0;
    unsigned char hash[64];
    char *input;

    srand(time(NULL));

    char *time = (char *) malloc(sizeof(char) * 31);
    get_time_str(time);
    get_time(&token_time);

    size_t password_len = 0;

    read_string_size_from_nvs("Server_PASSWORD", &password_len);

    char *password = (char *) malloc(sizeof(char) * password_len+1);

    read_string_from_nvs("Server_PASSWORD", password, &password_len);  

    input = (char *)calloc(1, sizeof(char) * (max_len+1));

    res = max_len - (strlen(time) + strlen(password));

    sprintf(input, "%s%s", time, password);

    for (size_t i = (strlen(time) + strlen(password)); i < max_len; i++){
        input[i] = letras[rand() % 61];
    }

    mbedtls_sha512_init(&token_ctx);
    mbedtls_sha512_starts_ret(&token_ctx, 0);

    mbedtls_sha512_update_ret(&token_ctx, (unsigned char *)input, max_len);

    mbedtls_sha512_finish_ret(&token_ctx, hash);

    for(int i = 0; i < 64; i++){
        sprintf(token + (i * 2), "%02x", hash[i]);
    }

    mbedtls_sha512_free(&token_ctx);
}

bool check_token(char *token_received){

    if(strcmp(token_received, token) == 0){
        time_t now;

        get_time(&now);

        double dif = difftime(now, token_time);

        if(dif > 1800){
            printf("Token expirado\n");
            return false;
        }
        return true;
    }
    return false;
}

esp_err_t handleAPILogin(httpd_req_t *req){
    int total_len = req->content_len;
    int cur_len = 0;

    char *buf = (char *)malloc(total_len+1);

    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        int received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    cJSON *root = cJSON_Parse(buf);

    if(root == NULL){
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");

    if(!cJSON_HasObjectItem(root, "password") || !cJSON_HasObjectItem(root, "user")){
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Missing password or user");

        return ESP_FAIL;
    }

    char * user = cJSON_GetObjectItem(root, "user")->valuestring;
    char * password = cJSON_GetObjectItem(root, "password")->valuestring;

    size_t password_len = 0;
    size_t user_len = 0;

    read_string_size_from_nvs("Server_USER", &user_len);
    read_string_size_from_nvs("Server_PASSWORD", &password_len);

    char *user_stored = (char *) malloc(sizeof(char) * user_len+1);
    char *password_stored = (char *) malloc(sizeof(char) * password_len+1);

    read_string_from_nvs("Server_USER", user_stored, &user_len);
    read_string_from_nvs("Server_PASSWORD", password_stored, &password_len);

    char *buffer = (char *)calloc(1, sizeof(char) * (200));

    if(strcmp(user, user_stored) == 0 && strcmp(password, password_stored) == 0){
        generate_token();
        sprintf(buffer, "{\"token\":\"%s\",\"status\":1,\"message\":\"Usuario Logado com Sucesso!\"}", token);
    }else{
        sprintf(buffer, "{\"token\":\"%s\",\"status\":0,\"error\":\"Usuario ou senha incorretos!\"}", token);
    }

    httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);

    cJSON_Delete(root);

    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(WebServerTAG, "Request headers lost");
    }

    return ESP_OK; 
}


esp_err_t handleAPIInfo(httpd_req_t *req){
    char *received_token = (char *)calloc(1, sizeof(char) * 129);

    size_t token_len = httpd_req_get_hdr_value_len(req, "token");

    if(token_len == 0){
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Token não informado");
        return ESP_FAIL;
    }

    httpd_req_get_hdr_value_str(req, "token", received_token, token_len+1);

    if(!check_token(received_token)){
        httpd_resp_set_type(req, "application/json");

        char *buffer = (char *)calloc(1, sizeof(char) * (200));

        sprintf(buffer, "{\"status\":0,\"error\":\"Token expirado ou invalido!\"}");

        httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);

        return ESP_OK;
    }

    free(received_token);

    int32_t boots = 0;
    int16_t interval = 0;
    int8_t uptime = 23;
    char *now = (char *)malloc(sizeof(char) * 20);

    get_time_str(now);
    read_int_from_nvs("boots", &boots);
    interval = 15;

    char *buffer = (char *)calloc(1, sizeof(char) * (200));

    sprintf(buffer, "{\"boots\":%d,\"upinterval\":%d,\"date\":\"%s\",\"uptime\":%d}", boots, interval, now, uptime);

    httpd_resp_set_type(req, "application/json");

    httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);

    free(buffer);

    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(WebServerTAG, "Request headers lost");
    }

    return ESP_OK;
}


esp_err_t handleAPIWifi(httpd_req_t *req){
    char *received_token = (char *)calloc(1, sizeof(char) * 129);

    size_t token_len = httpd_req_get_hdr_value_len(req, "token");

    if(token_len == 0){
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Token não informado");
        return ESP_FAIL;
    }

    httpd_req_get_hdr_value_str(req, "token", received_token, token_len+1);

    if(!check_token(received_token)){
        httpd_resp_set_type(req, "application/json");

        char *buffer = (char *)calloc(1, sizeof(char) * (200));

        sprintf(buffer, "{\"status\":0,\"error\":\"Token expirado ou invalido!\"}");

        httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);

        free(buffer);

        if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
            ESP_LOGI(WebServerTAG, "Request headers lost");
        }

        return ESP_OK;
    }

    free(received_token);

    char *ip = get_ip_address();
    char *mac = get_mac_address();
    int8_t status;
    read_bool_from_nvs("system_mode", (int8_t *)&status);

    size_t sta_ssid_size = 0;
    size_t ap_ssid_size = 0;

    read_string_size_from_nvs("AP_SSID", &sta_ssid_size);
    read_string_size_from_nvs("Wifi_SSID", &ap_ssid_size);

    char *sta_ssid = (char *)calloc(1, sizeof(char) * (sta_ssid_size+1));
    char *ap_ssid = (char *)calloc(1, sizeof(char) * (ap_ssid_size+1));

    read_string_from_nvs("AP_SSID", ap_ssid, &sta_ssid_size);
    read_string_from_nvs("Wifi_SSID", sta_ssid, &ap_ssid_size);

    char *buffer = (char *)calloc(1, sizeof(char) * (200));

    if(status == 1){
        sprintf(buffer, "{\"wifi_status\": \"Desligado\",\"wifi_ssid\":\"%s\",\"wifi_ip\":\"%s\",\"wifi_mac\":\"%s\",\"wifi_ap_status\":\"Ligado\",\"wifi_ap_ssid\":\"%s\",\"wifi_ap_ip\":\"%s\",\"wifi_ap_mac\":\"%s\"}", 
        sta_ssid, "null", "null", ap_ssid, ip, mac);
    }else{
        sprintf(buffer, "{\"wifi_status\":\"Ligado\",\"wifi_ssid\":\"%s\",\"wifi_ip\":\"%s\",\"wifi_mac\":\"%s\",\"wifi_ap_status\":\"Desligado\",\"wifi_ap_ssid\":\"%s\",\"wifi_ap_ip\":\"%s\",\"wifi_ap_mac\":\"%s\"}", 
        sta_ssid, ip, mac, ap_ssid, "null", "null");
    }

    httpd_resp_set_type(req, "application/json");

    httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);

    free(buffer);

    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(WebServerTAG, "Request headers lost");
    }

    return ESP_OK;
}

esp_err_t handleAPIConfig(httpd_req_t *req){
    char *received_token = (char *)calloc(1, sizeof(char) * 129);

    size_t token_len = httpd_req_get_hdr_value_len(req, "token");

    if(token_len == 0){
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Token não informado");
        return ESP_FAIL;
    }

    httpd_req_get_hdr_value_str(req, "token", received_token, token_len+1);

    if(!check_token(received_token)){
        httpd_resp_set_type(req, "application/json");

        char *buffer = (char *)calloc(1, sizeof(char) * (200));

        sprintf(buffer, "{\"status\":0,\"error\":\"Token expirado ou invalido!\"}");

        httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);

        free(buffer);

        return ESP_OK;
    }

    free(received_token);

    int total_len = req->content_len;
    int cur_len = 0;

    char *buf = (char *)malloc(total_len+1);

    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        int received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    cJSON *root = cJSON_Parse(buf);

    if(root == NULL){
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");

    if(cJSON_HasObjectItem(root, "ap_ssid")){
        char *ap_ssid = cJSON_GetObjectItem(root, "ap_ssid")->valuestring;

        if(ap_ssid == NULL){
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON");
            return ESP_FAIL;
        }

        write_string_to_nvs("AP_SSID", ap_ssid);

        free(ap_ssid);
    }

    if(cJSON_HasObjectItem(root, "ap_password")){
        char *ap_password = cJSON_GetObjectItem(root, "ap_password")->valuestring;

        if(ap_password == NULL){
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON");
            return ESP_FAIL;
        }

        write_string_to_nvs("AP_Password", ap_password);

        free(ap_password);
    }

    if(cJSON_HasObjectItem(root, "sta_ssid")){
        char *sta_ssid = cJSON_GetObjectItem(root, "sta_ssid")->valuestring;

        if(sta_ssid == NULL){
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON");
            return ESP_FAIL;
        }

        write_string_to_nvs("Wifi_SSID", sta_ssid);

        free(sta_ssid);
    }

    if(cJSON_HasObjectItem(root, "sta_password")){
        char *sta_password = cJSON_GetObjectItem(root, "sta_password")->valuestring;

        if(sta_password == NULL){
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON");
            return ESP_FAIL;
        }

        write_string_to_nvs("Wifi_Password", sta_password);

        free(sta_password);
    }

    if(cJSON_HasObjectItem(root, "mqtt_use_uri")){
        int mqtt_use_uri = cJSON_GetObjectItem(root, "mqtt_use_uri")->valueint;

        write_bool_to_nvs("MQTT_Use_URI", mqtt_use_uri);
    }

    if(cJSON_HasObjectItem(root, "mqtt_uri")){
        char *mqtt_uri = cJSON_GetObjectItem(root, "mqtt_uri")->valuestring;

        if(mqtt_uri == NULL){
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON");
            return ESP_FAIL;
        }

        write_string_to_nvs("MQTT_Server_URI", mqtt_uri);

        free(mqtt_uri);
    }

    if(cJSON_HasObjectItem(root, "mqtt_ip")){
        char *mqtt_ip = cJSON_GetObjectItem(root, "mqtt_ip")->valuestring;

        if(mqtt_ip == NULL){
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON");
            return ESP_FAIL;
        }

        write_string_to_nvs("MQTT_Server_IP", mqtt_ip);

        free(mqtt_ip);
    }

    if(cJSON_HasObjectItem(root, "mqtt_port")){
        int mqtt_port = cJSON_GetObjectItem(root, "mqtt_port")->valueint;

        write_int16_to_nvs("MQTT_Server_Port", mqtt_port);
    }

    if(cJSON_HasObjectItem(root, "mqtt_username")){

        char *mqtt_username = cJSON_GetObjectItem(root, "mqtt_username")->valuestring;

        if(mqtt_username == NULL){
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON");
            return ESP_FAIL;
        }

        write_string_to_nvs("MQTT_USER", mqtt_username);

        free(mqtt_username);
    }

    if(cJSON_HasObjectItem(root, "mqtt_password")){
        char *mqtt_password = cJSON_GetObjectItem(root, "mqtt_password")->valuestring;

        if(mqtt_password == NULL){
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON");
            return ESP_FAIL;
        }

        write_string_to_nvs("MQTT_PASSWORD", mqtt_password);

        free(mqtt_password);
    }

    if(cJSON_HasObjectItem(root, "mqtt_topic")){
        char *mqtt_topic = cJSON_GetObjectItem(root, "mqtt_topic")->valuestring;

        if(mqtt_topic == NULL){
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON");
            return ESP_FAIL;
        }

        write_string_to_nvs("MQTT_PASSWORD", mqtt_topic);

        free(mqtt_topic);
    }

    if(cJSON_HasObjectItem(root, "update")){
        int update = cJSON_GetObjectItem(root, "update")->valueint;

        write_bool_to_nvs("update", update);
    }

    if(cJSON_HasObjectItem(root, "update_url")){
        char *update_url = cJSON_GetObjectItem(root, "update_url")->valuestring;

        if(update_url == NULL){
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON");
            return ESP_FAIL;
        }

        write_string_to_nvs("update_url", update_url);

        free(update_url);
    }

    if(cJSON_HasObjectItem(root, "update_hour")){
        int8_t update_hour = cJSON_GetObjectItem(root, "update_hour")->valueint;

        write_bool_to_nvs("update_hour", update_hour);
    }

    if(cJSON_HasObjectItem(root, "update_interval")){
        int16_t update_interval = cJSON_GetObjectItem(root, "update_interval")->valueint;

        write_bool_to_nvs("update_interval", update_interval);
    }

    httpd_resp_set_type(req, "application/json");

    httpd_resp_send(req, "{\"status\":1}", HTTPD_RESP_USE_STRLEN);

    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(WebServerTAG, "Request headers lost");
    }

    return ESP_OK;
}

esp_err_t handleAPIConfigs(httpd_req_t *req){
    char *received_token = (char *)calloc(1, sizeof(char) * 129);

    size_t token_len = httpd_req_get_hdr_value_len(req, "token");

    if(token_len == 0){
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Token não informado");
        return ESP_FAIL;
    }

    httpd_req_get_hdr_value_str(req, "token", received_token, token_len+1);

    httpd_resp_set_type(req, "application/json");

    if(!check_token(received_token)){

        char *buffer = (char *)calloc(1, sizeof(char) * (200));

        sprintf(buffer, "{\"status\":0,\"error\":\"Token expirado ou invalido!\"}");

        httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);

        free(buffer);

        return ESP_OK;
    }

    free(received_token);

    system_config_t config = get_config_struct();

    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "ap_ssid", config.AP_SSID);
    cJSON_AddStringToObject(root, "ap_password", config.AP_Password);
    cJSON_AddStringToObject(root, "sta_ssid", config.Wifi_SSID);
    cJSON_AddStringToObject(root, "sta_password", config.Wifi_Password);
    cJSON_AddBoolToObject(root, "mqtt_use_uri", config.MQTT_Use_URI);
    cJSON_AddStringToObject(root, "mqtt_uri", config.MQTT_Server_URI);
    cJSON_AddStringToObject(root, "mqtt_ip", config.MQTT_Server_IP);
    cJSON_AddNumberToObject(root, "mqtt_port", config.MQTT_Server_PORT);
    cJSON_AddStringToObject(root, "mqtt_username", config.MQTT_Server_USER);
    cJSON_AddStringToObject(root, "mqtt_password", config.MQTT_Server_PASSWORD);
    cJSON_AddStringToObject(root, "mqtt_topic", config.MQTT_Server_Topic);
    cJSON_AddNumberToObject(root, "update", config.update);
    cJSON_AddNumberToObject(root, "update_hour", config.update_hour);
    cJSON_AddStringToObject(root, "update_url", config.update_url);
    cJSON_AddNumberToObject(root, "update_interval", config.update_interval);

    httpd_resp_send(req, cJSON_Print(root), HTTPD_RESP_USE_STRLEN);

    cJSON_Delete(root);

    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(WebServerTAG, "Request headers lost");
    }

    return ESP_OK;
}

esp_err_t handleAPICheckUpdate(httpd_req_t *req){
    char *received_token = (char *)calloc(1, sizeof(char) * 129);

    size_t token_len = httpd_req_get_hdr_value_len(req, "token");

    if(token_len == 0){
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Token não informado");
        return ESP_FAIL;
    }

    httpd_req_get_hdr_value_str(req, "token", received_token, token_len+1);

    if(!check_token(received_token)){
        httpd_resp_set_type(req, "application/json");

        char *buffer = (char *)calloc(1, sizeof(char) * (200));

        sprintf(buffer, "{\"status\":0,\"error\":\"Token expirado ou invalido!\"}");

        httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);

        free(buffer);

        return ESP_OK;
    }

    free(received_token);

    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(WebServerTAG, "Request headers lost");
    }

    return ESP_OK;
}

esp_err_t handleAPIUser(httpd_req_t *req){
    char *received_token = (char *)calloc(1, sizeof(char) * 129);

    size_t token_len = httpd_req_get_hdr_value_len(req, "token");

    if(token_len == 0){
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Token não informado");
        return ESP_FAIL;
    }

    httpd_req_get_hdr_value_str(req, "token", received_token, token_len+1);

    if(!check_token(received_token)){
        httpd_resp_set_type(req, "application/json");

        char *buffer = (char *)calloc(1, sizeof(char) * (200));

        sprintf(buffer, "{\"status\":0,\"error\":\"Token expirado ou invalido!\"}");

        httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);

        free(buffer);

        return ESP_OK;
    }

    free(received_token);

    size_t user_name_size = 0;

    read_string_size_from_nvs("Server_USER", &user_name_size);

    char *user_name = (char *)calloc(1, sizeof(char) * (user_name_size+1));

    read_string_from_nvs("Server_USER", user_name, &user_name_size);

    httpd_resp_set_type(req, "application/json");

    char *buffer = (char *)calloc(1, sizeof(char) * (200));

    sprintf(buffer, "{\"status\":1,\"user\":\"%s\"}", user_name);

    httpd_resp_send(req, buffer , HTTPD_RESP_USE_STRLEN);

    free(buffer);

    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(WebServerTAG, "Request headers lost");
    }

    return ESP_OK;
}

esp_err_t handleAPIChangeSecrets(httpd_req_t *req){
    char *received_token = (char *)calloc(1, sizeof(char) * 129);

    size_t token_len = httpd_req_get_hdr_value_len(req, "token");

    if(token_len == 0){
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Token não informado");
        return ESP_FAIL;
    }

    httpd_req_get_hdr_value_str(req, "token", received_token, token_len+1);

    if(!check_token(received_token)){
        httpd_resp_set_type(req, "application/json");

        char *buffer = (char *)calloc(1, sizeof(char) * (200));

        sprintf(buffer, "{\"status\":0,\"error\":\"Token expirado ou invalido!\"}");

        httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);

        free(buffer);

        return ESP_OK;
    }

    free(received_token);

    size_t user_name_size = 0;
    size_t password_size = 0;

    read_string_size_from_nvs("Server_USER", &user_name_size);
    read_string_size_from_nvs("Server_PASSWORD", &password_size);

    char *user_name = (char *)calloc(1, sizeof(char) * (user_name_size+1));
    char *password = (char *)calloc(1, sizeof(char) * (password_size+1));

    read_string_from_nvs("Server_USER", user_name, &user_name_size);
    read_string_from_nvs("Server_PASSWORD", password, &password_size);

    int total_len = req->content_len;
    int cur_len = 0;

    char *buf = (char *)malloc(total_len+1);

    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        int received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    cJSON *root = cJSON_Parse(buf);

    if(!root){
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON");
        return ESP_FAIL;
    }

    if(cJSON_HasObjectItem(root, "user")){
        char *user_name_new = cJSON_GetObjectItem(root, "user")->valuestring;

        write_string_to_nvs("Server_USER", user_name_new);
    }

    httpd_resp_set_type(req, "application/json");

    if(cJSON_HasObjectItem(root, "password")){
        if(cJSON_HasObjectItem(root, "old_password")){
            char *password_old = cJSON_GetObjectItem(root, "old_password")->valuestring;

            if(strcmp(password_old, password) != 0){
                httpd_resp_send(req, "{\"status\":0,\"error\":\"Senha antiga incorreta!\"}", HTTPD_RESP_USE_STRLEN);
                return ESP_OK;
            }
        }

        char *password_new = cJSON_GetObjectItem(root, "password")->valuestring;

        write_string_to_nvs("Server_PASSWORD", password_new);
    }

    httpd_resp_send(req, "{\"status\":1,\"message\":\"Credenciais alteradas com sucesso.\"}", HTTPD_RESP_USE_STRLEN);

    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(WebServerTAG, "Request headers lost");
    }

    return ESP_OK;
}


esp_err_t handleAPIIsESP32(httpd_req_t *req){
    httpd_resp_set_type(req, "application/json");

    httpd_resp_send(req, "{\"status\":1}", HTTPD_RESP_USE_STRLEN);

    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(WebServerTAG, "Request headers lost");
    }

    return ESP_OK;
}