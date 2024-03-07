#ifndef __ST_COMMON_REST_CLIENT_H_INCLUDED__
#define __ST_COMMON_REST_CLIENT_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_http_client.h"
#include "cJSON.h"


#define DEVICE_STATUS_URL(deviceId) "https://api.smartthings.com/devices/"deviceId"/components"
#define DEVICE_MAIN_COMPONENT_STATUS_URL(deviceId) "https://api.smartthings.com/devices/"deviceId"/components/main/status"
#define DEVICE_PREFERENCES_URL(deviceId) "https://api.smartthings.com/devices/"deviceId"/preferences"
#define VIRTUALDEVICE_EVENT_URL(deviceId) "https://api.smartthings.com/virtualdevices/"deviceId"/events"

#define get(token, url, result) request(token, HTTP_METHOD_GET, url, NULL, result)
#define post(token, url, body, result) request(token, HTTP_METHOD_POST, url, body, result)
#define put(token, url, body, result) request(token, HTTP_METHOD_PUT, url, body, result)
#define delete(token, url, result) request(token, HTTP_METHOD_DELETE, url, NULL, result)

esp_err_t request(const char *accessToken, esp_http_client_method_t method, const char *url, const char *body, cJSON **result);


#ifdef __cplusplus
}
#endif

#endif