#ifndef BLE_MODULE_H_
#define BLE_MODULE_H_

#include <stdbool.h>
#include <stdio.h>

#include "esp_bt.h"						//configures the BT controller and VHCI from the host side.
#include "esp_bt_main.h"                 //initializes and enables the Bluedroid stack.
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_gatt_common_api.h"

#include "esp_log.h"
//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_coexist.h"

#include <cJSON.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "Flash_module.h"
#include "Server_module.h"

#include "BLE_defs.h"
#include "sdkconfig.h"


/* old app_main(). Init everything and start BLE task controlling FoodLock */
void initBLE(void);

/* Bluetooth GAP callback */
void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

/* BLE GATTC callback function */
void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

/* Create different queues and semaphores */
void create_que_and_semaphr(void);

/* Check whether current device was saved in flash memory */
bool isDeviceFounded(esp_bd_addr_t* curr_bda);

/* Create common Food Lock task with parameter */
void createFoodLock_task_param(uint8_t* pParam);

/* Common structure for each foodlocks tasks to create separate objects */
void FLock_task_idx_v2(void *pvParameters);

/* Function sends commands to smart lock */
void send_CMD_to_FL_x_v2(struct FL_x_container_v2* pContainer, uint8_t prof_num);

/* Function removes quotes from string */
void removeQuotes(char* pInString);

/* Function just writes necessary data to Week service */
void wrapperWeekWrite(uint8_t prof_num, uint8_t *lock_h, uint8_t *lock_m, uint8_t *unlock_h, uint8_t *unlock_m);

/* function writes value to a specific character in specific service */
void writeCharact_in_Service_v3(uint8_t fLOCK_ID_x, esp_bt_uuid_t* pService, esp_bt_uuid_t* pCharacteristic, const char* CMD, uint8_t length);

/* Common GAP GATTC event handler for every device (gattc_if) */
void gattc_profile_x_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

/* Function returns index of profile, where internal gattc_if equals to that, what that call callback function
 * gattc_profile_x_event_handler */
uint8_t idx_Of_GATTC_if(esp_gatt_if_t gattc_if_curr);

/* Function forming string of POST request for curl*/
void formStringRequest_v5(char* request, esp_bd_addr_t* pBLE_addr);

/* Functions initiates every field of server_cont_struct_v2  to "noCMD" string */
void initStructContainer_v2(void);

/* Function-task that makes requests */
void http2_task_v2(void *args);

/*	Function checks if registered SenseKitchen devices were in advertising mode (while scanning) and if true tries to establish BT connection 	*/
void open_ble_conn(void);

/* Function Parse the answer and fill the struct to send*/
int jsonParse_fill_struct_v3(char* get_str_f, uint8_t prof_idx);

/* Function gets json object of day schedule and fills structure to send to foodlock */
uint8_t getSchedule(cJSON *jsDay, uint8_t DAY_CODE, uint8_t *lock_h, uint8_t *lock_m, uint8_t *unlock_h, uint8_t *unlock_m);

#endif /* BLE_MODULE_H_ */



