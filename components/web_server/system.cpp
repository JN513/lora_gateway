#include "web_server.h"


esp_err_t handleReboot(httpd_req_t *req){
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

        return ESP_OK;
    }

    free(received_token);

    httpd_resp_send(req, "{\"status\":1}", HTTPD_RESP_USE_STRLEN);

    esp_restart(); // Reinicia o sistema

    return ESP_OK;
}

esp_err_t handleQuit(httpd_req_t *req){
    void generate_token();

    httpd_resp_set_type(req, "application/json");

    httpd_resp_send(req, "{\"status\":1,\"message\":\"Usuario deslogado com sucesso.\"}", HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

esp_err_t handleInitAPmode(httpd_req_t *req){
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

        return ESP_OK;
    }

    free(received_token);

    httpd_resp_send(req, "{\"status\":1}", HTTPD_RESP_USE_STRLEN);

    write_bool_to_nvs("system_mode", 1);

    esp_restart(); // Reinicia o sistema

    return ESP_OK;
}

esp_err_t handleInitSTAmode(httpd_req_t *req){
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

        return ESP_OK;
    }

    free(received_token);

    httpd_resp_send(req, "{\"status\":1}", HTTPD_RESP_USE_STRLEN);

    write_bool_to_nvs("system_mode", 0);

    esp_restart(); // Reinicia o sistema

    return ESP_OK;
}


esp_err_t handleReconfig(httpd_req_t *req){
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

        return ESP_OK;
    }

    free(received_token);

    httpd_resp_send(req, "{\"status\":1}", HTTPD_RESP_USE_STRLEN);

    delete_config(false); // Deleta as configurações do sistema

    esp_restart(); // Reinicia o sistema

    return ESP_OK;
}

esp_err_t handleCSS(httpd_req_t *req){
    httpd_resp_set_type(req, "text/css");

    char *buffer = (char *)malloc(sizeof(char) * (1300)); // Aloca memória para o buffer

    read_file("style/style.css", buffer, 1300); // Lê o arquivo style.css

    httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN); // Envia o arquivo style.css

    return ESP_OK;
}

esp_err_t handleJS(httpd_req_t *req){
    httpd_resp_set_type(req, "text/javascript");

    size_t buf_len;

    read_file_size("js/main.js", &buf_len); // Lê o tamanho do arquivo script.js

    char *buffer = (char *)calloc(1, buf_len+1); // Aloca memória para o buffer

    read_file("js/main.js", buffer, buf_len); // Lê o arquivo style.css

    httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN); // Envia o arquivo style.css

    return ESP_OK;
}

esp_err_t handleFavicon(httpd_req_t *req){
    extern const unsigned char favicon_ico_start[] asm("_binary_favicon_ico_start");
    extern const unsigned char favicon_ico_end[]   asm("_binary_favicon_ico_end");

    const size_t favicon_ico_size = (favicon_ico_end - favicon_ico_start);

    httpd_resp_set_type(req, "image/x-icon");

    httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_size);

    return ESP_OK;
}