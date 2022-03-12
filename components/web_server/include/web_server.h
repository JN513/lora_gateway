#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <esp_log.h>
#include <esp_system.h>
#include "esp_netif.h"
#include "esp_eth.h" 
#include "cJSON.h"

#include <esp_https_server.h>

#include "storage_manager.h"
#include "time_manager.h"
#include "communication.h"

#include <mbedtls/sha512.h>

#define SCRATCH_BUFSIZE (10240)

static const char *WebServerTAG = "web_server";

static char token[129];

static time_t token_time;

static httpd_handle_t server = NULL;

// API
void generate_token();
bool check_token(char *token_received);
esp_err_t handleAPILogin(httpd_req_t *req);
esp_err_t handleAPIInfo(httpd_req_t *req);
esp_err_t handleAPIWifi(httpd_req_t *req);
esp_err_t handleAPIConfigs(httpd_req_t *req);
esp_err_t handleAPIConfig(httpd_req_t *req);
esp_err_t handleAPICheckUpdate(httpd_req_t *req);
esp_err_t handleAPIUser(httpd_req_t *req);
esp_err_t handleAPIChangeSecrets(httpd_req_t *req);
esp_err_t handleAPIIsESP32(httpd_req_t *req);

// System
esp_err_t handleReboot(httpd_req_t *req);
esp_err_t handleQuit(httpd_req_t *req);
esp_err_t handleInitAPmode(httpd_req_t *req);
esp_err_t handleInitSTAmode(httpd_req_t *req);
esp_err_t handleReconfig(httpd_req_t *req);
esp_err_t handleCSS(httpd_req_t *req);
esp_err_t handleJS(httpd_req_t *req);
esp_err_t handleFavicon(httpd_req_t *req);

// Template
esp_err_t handleLogin(httpd_req_t *req);
esp_err_t handleRoot(httpd_req_t *req);
esp_err_t handleInfo(httpd_req_t *req);
esp_err_t handleChangeSecrets(httpd_req_t *req);
esp_err_t handleGetUpdate(httpd_req_t *req);

// Main
esp_err_t init_web_server();

#ifdef __cplusplus
}
#endif

#endif // WEB_SERVER_H
