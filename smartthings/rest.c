#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"

#include "smartthings/rest.h"


#define USER_AGENT "@smartthings/cli/1.8.1 win32-x64 node-v18.5.0"
#define HTTP_BUF_SIZE 2048
#define HTTP_TIMEOUT 5000
const char *TAG = "ST-REST";


esp_err_t request(const char *accessToken, esp_http_client_method_t method, const char *url, const char *body, cJSON **result) {
    ESP_LOGD("ST-REST HTTP request", "URL: %s, METHOD: %d", url, method);

    ESP_RETURN_ON_FALSE(
        strlen(accessToken) == 36, ESP_ERR_INVALID_ARG,
        TAG, "Invalid access token length %d.", strlen(accessToken)
    );

    esp_http_client_config_t cfg = {
        .url = url,
        .method = method,
        .user_agent = USER_AGENT,
        .buffer_size = HTTP_BUF_SIZE,
        .timeout_ms = HTTP_TIMEOUT,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_err_t ret = ESP_OK;
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    ESP_GOTO_ON_ERROR(
        esp_http_client_set_header(client, "Content-Type", "application/json;charset=utf-8"), CLEANUP,
        TAG, "Error: set-header Content-Type."
    );
    ESP_GOTO_ON_ERROR(
        esp_http_client_set_header(client, "Accept", "application/vnd.smartthings+json;v=20170916"), CLEANUP,
        TAG, "Error: set-header Accept."
    );
    ESP_GOTO_ON_ERROR(
        esp_http_client_set_header(client, "User-Agent", "@smartthings/cli/1.8.1 win32-x64 node-v18.5.0"), CLEANUP,
        TAG, "Error: set-header User-Agent."
    );
    ESP_GOTO_ON_ERROR(
        esp_http_client_set_header(client, "Accept-Language", "ko-KR"), CLEANUP,
        TAG, "Error: set-header Accept-Language."
    );
    char authValue[44];
    sprintf(authValue, "Bearer %s", accessToken);
    ESP_GOTO_ON_ERROR(
        esp_http_client_set_header(client, "Authorization", authValue), CLEANUP,
        TAG, "Error: set-header Authorization."
    );

    int body_size = 0;
    if (body != NULL) {
        if (method == HTTP_METHOD_POST || method == HTTP_METHOD_PUT) {
            body_size = strlen(body);
        } else {
            ESP_LOGW("ST-REST HTTP request", "Method(%d) is not POST or PUT", method);
        }
    }

    ESP_LOGD(TAG, "*****Free Mem: %u*****", heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT));
    ESP_GOTO_ON_ERROR(
        ret = esp_http_client_open(client, body_size), CLEANUP,
        "ST-REST HTTP request", "Error perform http request (Error: %s).", esp_err_to_name(ret)
    );
    int data_written = 0;
    if (body_size > 0) {
        data_written = esp_http_client_write(client, body, body_size);
    }
    ESP_GOTO_ON_FALSE(
        data_written > -1, ESP_FAIL, CLEANUP,
        "ST-REST HTTP request", "POST/PUT Failed to send data."
    );
    ESP_GOTO_ON_FALSE(
        data_written == body_size, ESP_FAIL, CLEANUP,
        "ST-REST HTTP request", "POST/PUT Data partially written."
    );
    if (body_size > 0) {
        ESP_LOGI("ST-REST HTTP request", "POST/PUT Data successfully written.");
    }
    int64_t response_length = esp_http_client_fetch_headers(client);
    int status_code = esp_http_client_get_status_code(client);
    ESP_GOTO_ON_FALSE(
        status_code == 200, ESP_FAIL, CLEANUP,
        "ST-REST HTTP request", "Request failed. (Status code: %d)", status_code
    );

    char *buf = malloc(sizeof(char) * (response_length + 1));
    ESP_GOTO_ON_FALSE(
        buf != NULL, ESP_ERR_NO_MEM, CLEANUP,
        "ST-REST HTTP request", "No Sufficient memory for body buffer."
    );
    int response_read = esp_http_client_read_response(client, buf, response_length + 1);
    ESP_LOGD("ST-REST HTTP request", "Response length: %lld | Read: %d", response_length, response_read);
    if (result == NULL) {
        ESP_LOGD("ST-REST HTTP request", "result is NULL, skipping parsing in json...");
        if (response_read > 0)
            ESP_LOGD("ST-REST HTTP request", "Body: %s", buf);  
    } else {
        ESP_LOGD("ST-REST HTTP request", "Start Parsing JSON response...");
        *result = cJSON_Parse(buf);
        ESP_LOGD("ST-REST HTTP request", "End Parsing.");
    }
    free(buf);

CLEANUP:
    if (ret != ESP_OK)
        ESP_LOGE(
            "ST-REST Cleanup", "Error dected. (Err: %s, Free Mem: %u)",
            esp_err_to_name(ret), heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT)
        );
    esp_http_client_cleanup(client);
    return ret;
}
