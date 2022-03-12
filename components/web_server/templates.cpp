#include "web_server.h"


esp_err_t handleLogin(httpd_req_t *req){

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Type", "text/html"); // Define o tipo de conteúdo do arquivo
    httpd_resp_set_hdr(req, "Set-Cookie", "ESPSESSIONID=1");

    size_t size = 0;
    
    read_file_size("components/header1.html", &size);

    char *buffer = (char *)malloc(sizeof(char) * size+1);

    esp_err_t ret = read_file("components/header1.html", buffer, size);

    if(ret != ESP_OK){
        httpd_resp_send_500(req);
    }

    // httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);

    httpd_resp_send_chunk(req, buffer, size);

    free(buffer); // Libera a memória alocada para o buffer

    httpd_resp_send_chunk(req, "Login", 5);

    read_file_size("components/header2.html", &size);

    buffer = (char *)malloc(sizeof(char) * size+1);

    ret = read_file("components/header2.html", buffer, size);

    if(ret != ESP_OK){
        httpd_resp_send_500(req);
    }

    httpd_resp_send_chunk(req, buffer, size);

    free(buffer); // Libera a memória alocada para o buffer

    read_file_size("pages/login.html", &size);

    buffer = (char *)malloc(sizeof(char) * size+1);

    ret = read_file("pages/login.html", buffer, size);

    if(ret != ESP_OK){
        httpd_resp_send_500(req);
    }

    httpd_resp_send_chunk(req, buffer, size);

    free(buffer); // Libera a memória alocada para o buffer

    read_file_size("components/footer.html", &size);

    buffer = (char *)malloc(sizeof(char) * size+1);

    ret = read_file("components/footer.html", buffer, size);

    if(ret != ESP_OK){
        httpd_resp_send_500(req);
    }

    httpd_resp_send_chunk(req, buffer, size);

    free(buffer); // Libera a memória alocada para o buffer

    httpd_resp_send_chunk(req, NULL, 0);

    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(WebServerTAG, "Request headers lost");
    }

    return ESP_OK;
}


esp_err_t handleRoot(httpd_req_t *req){

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Type", "text/html"); // Define o tipo de conteúdo do arquivo

    
    

    return ESP_OK;
}

esp_err_t handleInfo(httpd_req_t *req){


    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Type", "text/html"); // Define o tipo de conteúdo do arquivo);

    size_t size = 0;
    
    read_file_size("components/header1.html", &size);

    char *buffer = (char *)malloc(sizeof(char) * size+1);

    esp_err_t ret = read_file("components/header1.html", buffer, size);

    if(ret != ESP_OK){
        httpd_resp_send_500(req);
    }

    // httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);

    httpd_resp_send_chunk(req, buffer, size);

    free(buffer); // Libera a memória alocada para o buffer

    httpd_resp_send_chunk(req, "Info", 4);

    read_file_size("components/header2.html", &size);

    buffer = (char *)malloc(sizeof(char) * size+1);

    ret = read_file("components/header2.html", buffer, size);
    
    if(ret != ESP_OK){
        httpd_resp_send_500(req);
    }

    httpd_resp_send_chunk(req, buffer, size);

    free(buffer); // Libera a memória alocada para o buffer

    read_file_size("pages/info.html", &size);

    buffer = (char *)malloc(sizeof(char) * size+1);

    ret = read_file("pages/info.html", buffer, size);

    if(ret != ESP_OK){
        httpd_resp_send_500(req);
    }

    httpd_resp_send_chunk(req, buffer, size);

    free(buffer); // Libera a memória alocada para o buffer

    read_file_size("pages/infoscript.html", &size);

    buffer = (char *)malloc(sizeof(char) * size+1);

    ret = read_file("pages/infoscript.html", buffer, size);

    if(ret != ESP_OK){
        httpd_resp_send_500(req);
    }

    httpd_resp_send_chunk(req, buffer, size);

    free(buffer); // Libera a memória alocada para o buffer    

    read_file_size("components/footer.html", &size);

    buffer = (char *)malloc(sizeof(char) * size+1);

    ret = read_file("components/footer.html", buffer, size);

    if(ret != ESP_OK){
        httpd_resp_send_500(req);
    }

    httpd_resp_send_chunk(req, buffer, size);

    free(buffer); // Libera a memória alocada para o buffer

    httpd_resp_send_chunk(req, NULL, 0);

    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(WebServerTAG, "Request headers lost");
    }

    httpd_resp_send_chunk(req, buffer, size);

    free(buffer); // Libera a memória alocada para o buffer

    return ESP_OK;
}

esp_err_t handleChangeSecrets(httpd_req_t *req){

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Type", "text/html"); // Define o tipo de conteúdo do arquivo
    httpd_resp_set_hdr(req, "Set-Cookie", "ESPSESSIONID=1");

    size_t size = 0;
    
    read_file_size("components/header1.html", &size);

    char *buffer = (char *)malloc(sizeof(char) * size+1);

    esp_err_t ret = read_file("components/header1.html", buffer, size);

    if(ret != ESP_OK){
        httpd_resp_send_500(req);
    }

    // httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);

    httpd_resp_send_chunk(req, buffer, size);

    free(buffer); // Libera a memória alocada para o buffer

    httpd_resp_send_chunk(req, "Alterar Credenciais", 20);

    read_file_size("components/header2.html", &size);

    buffer = (char *)malloc(sizeof(char) * size+1);

    ret = read_file("components/header2.html", buffer, size);

    if(ret != ESP_OK){
        httpd_resp_send_500(req);
    }

    httpd_resp_send_chunk(req, buffer, size);

    free(buffer); // Libera a memória alocada para o buffer

    read_file_size("pages/changesecrets.html", &size);

    buffer = (char *)malloc(sizeof(char) * size+1);

    ret = read_file("pages/changesecrets.html", buffer, size);

    if(ret != ESP_OK){
        httpd_resp_send_500(req);
    }

    httpd_resp_send_chunk(req, buffer, size);

    free(buffer); // Libera a memória alocada para o buffer

    read_file_size("components/footer.html", &size);

    buffer = (char *)malloc(sizeof(char) * size+1);

    ret = read_file("components/footer.html", buffer, size);

    if(ret != ESP_OK){
        httpd_resp_send_500(req);
    }

    httpd_resp_send_chunk(req, buffer, size);

    free(buffer); // Libera a memória alocada para o buffer

    httpd_resp_send_chunk(req, NULL, 0);

    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(WebServerTAG, "Request headers lost");
    }

    return ESP_OK;
}

esp_err_t handleGetUpdate(httpd_req_t *req){

    return ESP_OK;
}