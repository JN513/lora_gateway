#include "web_server.h"


esp_err_t handleNotFound(httpd_req_t *req){
    return ESP_OK;
}

esp_err_t init_web_server(){

    httpd_uri_t home = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = handleRoot,
        .user_ctx = NULL
    };

    httpd_uri_t login = {
        .uri = "/login",
        .method = HTTP_GET,
        .handler = handleLogin,
        .user_ctx = NULL
    };

    httpd_uri_t reboot = {
        .uri = "/reboot",
        .method = HTTP_GET,
        .handler = handleReboot,
        .user_ctx = NULL
    };
    httpd_uri_t quit = {
        .uri = "/quit",
        .method = HTTP_GET,
        .handler = handleQuit,
        .user_ctx = NULL
    };

    httpd_uri_t apmode = {
        .uri = "/initap",
        .method = HTTP_GET,
        .handler = handleInitAPmode,
        .user_ctx = NULL
    };

    httpd_uri_t stamode = {
        .uri = "/initsta",
        .method = HTTP_GET,
        .handler = handleInitSTAmode,
        .user_ctx = NULL
    };

    httpd_uri_t info = {
        .uri = "/info",
        .method = HTTP_GET,
        .handler = handleInfo,
        .user_ctx = NULL
    };

    httpd_uri_t reconfig = {
        .uri = "/reconfig",
        .method = HTTP_GET,
        .handler = handleReconfig,
        .user_ctx = NULL
    };

    httpd_uri_t css = {
        .uri = "/css",
        .method = HTTP_GET,
        .handler = handleCSS,
        .user_ctx = NULL
    };

    httpd_uri_t js = {
        .uri = "/main.js",
        .method = HTTP_GET,
        .handler = handleJS,
        .user_ctx = NULL
    };

    httpd_uri_t favicon = {
        .uri = "/favicon.ico",
        .method = HTTP_GET,
        .handler = handleFavicon,
        .user_ctx = NULL
    };

    httpd_uri_t change = {
        .uri = "/change",
        .method = HTTP_GET,
        .handler = handleChangeSecrets,
        .user_ctx = NULL
    };

    httpd_uri_t systemstats = {
        .uri = "/api/info",
        .method = HTTP_GET,
        .handler = handleAPIInfo,
        .user_ctx = NULL
    };

    httpd_uri_t systemwifi = {
        .uri = "/api/wifi",
        .method = HTTP_GET,
        .handler = handleAPIWifi,
        .user_ctx = NULL
    };

    httpd_uri_t apilogin = {
        .uri = "/api/login",
        .method = HTTP_POST,
        .handler = handleAPILogin,
        .user_ctx = NULL
    };

    httpd_uri_t apiconfigs = {
        .uri = "/api/configs",
        .method = HTTP_GET,
        .handler = handleAPIConfigs,
        .user_ctx = NULL
    };

    httpd_uri_t apiconfig = {
        .uri = "/api/config",
        .method = HTTP_POST,
        .handler = handleAPIConfig,
        .user_ctx = NULL
    };

    httpd_uri_t apicheckupdate = {
        .uri = "/api/update",
        .method = HTTP_GET,
        .handler = handleAPICheckUpdate,
        .user_ctx = NULL
    };

    httpd_uri_t apiuser = {
        .uri = "/api/user",
        .method = HTTP_GET,
        .handler = handleAPIUser,
        .user_ctx = NULL
    };

    httpd_uri_t apichangesecrets = {
        .uri = "/api/change/secrets",
        .method = HTTP_POST,
        .handler = handleAPIChangeSecrets,
        .user_ctx = NULL
    };

    httpd_uri_t apiisesp32 = {
        .uri = "/api/is/esp32",
        .method = HTTP_GET,
        .handler = handleAPIIsESP32,
        .user_ctx = NULL
    };

    /*
    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();
    config.httpd.lru_purge_enable = true;
    config.httpd.max_uri_handlers = 15;
    config.httpd.uri_match_fn = httpd_uri_match_wildcard;
    */

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.max_uri_handlers = 20;
    config.uri_match_fn = httpd_uri_match_wildcard;

    /*
    extern const unsigned char cacert_pem_start[] asm("_binary_cacert_pem_start");
    extern const unsigned char cacert_pem_end[]   asm("_binary_cacert_pem_end");
    config.cacert_pem = cacert_pem_start;
    config.cacert_len = cacert_pem_end - cacert_pem_start;

    extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
    extern const unsigned char prvtkey_pem_end[]   asm("_binary_prvtkey_pem_end");
    config.prvtkey_pem = prvtkey_pem_start;
    config.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;
    

    ESP_LOGI(WebServerTAG, "Server iniciado na porta: '%d'", config.port_secure);
    if (httpd_ssl_start(&server, &config) == ESP_OK) {
    */
    ESP_LOGI(WebServerTAG, "Server iniciado na porta: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(WebServerTAG, "Registrando rotas");
        httpd_register_uri_handler(server, &login);
        httpd_register_uri_handler(server, &home);
        httpd_register_uri_handler(server, &reboot);
        httpd_register_uri_handler(server, &quit);
        httpd_register_uri_handler(server, &apmode);
        httpd_register_uri_handler(server, &stamode);
        httpd_register_uri_handler(server, &info);
        httpd_register_uri_handler(server, &reconfig);
        httpd_register_uri_handler(server, &css);
        httpd_register_uri_handler(server, &js);
        httpd_register_uri_handler(server, &favicon);
        httpd_register_uri_handler(server, &change);
        httpd_register_uri_handler(server, &systemstats);
        httpd_register_uri_handler(server, &systemwifi);
        httpd_register_uri_handler(server, &apilogin);
        httpd_register_uri_handler(server, &apiconfigs);
        httpd_register_uri_handler(server, &apiconfig);
        httpd_register_uri_handler(server, &apicheckupdate);
        httpd_register_uri_handler(server, &apiuser);
        httpd_register_uri_handler(server, &apichangesecrets);
        httpd_register_uri_handler(server, &apiisesp32);

        ESP_LOGI(WebServerTAG, "Rotas registradas");

        return ESP_OK;
    }

    ESP_LOGI(WebServerTAG, "Erro ao iniciar o WebServer");

    return ESP_FAIL;
}
