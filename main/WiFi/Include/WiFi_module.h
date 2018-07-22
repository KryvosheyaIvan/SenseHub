#ifndef WIFI_MODULE_H_
#define WIFI_MODULE_H_

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"


#include "sdkconfig.h"


/* Init whole WiFi module*/
void WiFi_init(void);

/* Init WiFi in AP mode*/
void initialise_wifi(void);

/* Init WiFi in station mode*/
void initialise_wifi_2(void);

/* WiFi callback function */
esp_err_t event_handler(void *ctx, system_event_t *event);

/* Function saves current SSID and password to flash storage*/
esp_err_t print_save_hub_SSID_PASS(char *SSID, char *PASS);

/* Reset WiFi configuration to point mode*/
void wifi_reset_to_point_mode(void);

/* Save hub status*/
esp_err_t print_save_hub_status(char *hubStatus);

/* Function prints out WiFi hub status*/
esp_err_t print_read_hub_status(char *hubStatus);

/* Serve function that handles incoming requests */
void http_server_netconn_serve(struct netconn *conn);

/* Reset WiFi to client mode in case of unsuccessful connections*/
void wifi_reset_to_client_mode(char *ssid, char *pass);

/* http server service task*/
void http_server(void *pvParameters);

/* Task controlling indication LED */
void diode_blinking(void *pvParameters);

/* Task checking control button status */
void push_button(void *pvParameters);

#endif
