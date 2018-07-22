#ifndef FLASH_MODULE_H
#define FLASH_MODULE_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"
//#include "controller.h"

#include "esp_bt.h"						//configures the BT controller and VHCI from the host side.
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"                 //initializes and enables the Bluedroid stack.
#include "esp_gatt_common_api.h"


#include "esp_log.h"
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

/* Init NVS */
void init_NVS(void);

/* Function must print saved BLE addresses if they were written on flash or in other case write 10 false bda addresses into flash */
esp_err_t restore_saved_bda(esp_bd_addr_t* pBLE_addr);


#endif
