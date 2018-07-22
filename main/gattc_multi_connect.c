#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"


#include "BLE_module.h"
#include "WiFi_module.h"

void app_main()
{
	/*	Create different queues and semaphores
	 *  Initialize NVS
	 *  Prints saved BDA and number of registered FoodLocks
	 *  Initialize BLE
	 *
	 * start BLE task controlling FoodLock
	 */

	init_NVS();
	WiFi_init();
	create_que_and_semaphr();
	initBLE();
}

