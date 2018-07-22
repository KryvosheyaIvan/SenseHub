#include "Flash_module.h"


/* Init NVS */
void init_NVS(void)
{
    // Initialize NVS.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    printf("\n NVS initialised");
}

/*
 * Function must prints saved BLE addresses if they were written on flash or
 * in other case write 10 false bda addresses into flash
 *
 * */
esp_err_t restore_saved_bda(esp_bd_addr_t* pBLE_addr)
{
    nvs_handle ble_handle;
    esp_err_t err;

    // Open NVS
    //deb printf("\n Restoring Flash BLE data NVS...\n");
    err = nvs_open("storage", NVS_READWRITE, &ble_handle);
    if (err != ESP_OK) return err;

    // Read BLE addresses
    size_t required_size = 0;  // value will default to 0, if not set yet in NVS
    // obtain required memory space to store blob being read from NVS
    err = nvs_get_blob(ble_handle, "bda_saved_nvs", NULL, &required_size);
    //deb printf("Obtain required memory space to store complex data structure (bda) being read from NVS...\n");
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;


    /************* Read Number of registered SenseKitchenDevices*************************/
    //deb printf("Reading number of registered devices from NVS ... ");
    uint8_t FL_num_reg = 0; // value will default to 0, if not set yet in NVS
    err = nvs_get_u8(ble_handle, "FL_num_nvs", &FL_num_reg);
    switch (err) {
        case ESP_OK:
            //deb printf("Done\n");
            printf("Number of FoodLocks = %d\n", FL_num_reg);
            //PROFILE_NUM = FL_num_reg; //write number of foodlocks to global variable
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            printf("Initializing Number of FoodLocks...\n");
            err = nvs_set_u8(ble_handle, "FL_num_nvs", FL_num_reg);
            printf("Committing FL_num_nvs in NVS ... ");
            err = nvs_commit(ble_handle);
            printf((err != ESP_OK) ? "Failed!\n" : "Done writing FL_num_nvs \n");
            break;
        default :
            break;
    }
    /****************************************************************************************/


    if (required_size == 0) {
        printf("\n No BLE addresses hasnt been saved yet!");
		printf("\n Saving 10 random addresses...");

		size_t bda_struct_size = sizeof(esp_bd_addr_t)*10; //size of 10 BLE addresses
		esp_bd_addr_t BLE_addr_false[10];

		uint8_t bda_false[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //false address

		//Copy 10 addressed to NVS flash
		for(uint8_t i = 0; i < 10; i++)
		{
			memcpy(BLE_addr_false[i], bda_false, sizeof(esp_bd_addr_t));
		}

		err = nvs_set_blob(ble_handle, "bda_saved_nvs", BLE_addr_false, bda_struct_size); //required_size is Nupgraded (+1)essentially

		printf("\n Writing to  NVS...\n");

		if (err != ESP_OK) return err;
		// Commit. Mandatory!
		err = nvs_commit(ble_handle);

		printf("\n Committing to  NVS...\n");

		if (err != ESP_OK) return err;

    } else { //There are saved addresses
        esp_bd_addr_t* bda_saved = malloc(required_size);
        err = nvs_get_blob(ble_handle, "bda_saved_nvs", bda_saved, &required_size);

        //deb printf(" Getting existing bda...\n");
        printf(" Registered FoodLock`s addresses: \n");
        if (err != ESP_OK) return err;

        for (int i = 0; i < required_size / sizeof(esp_bd_addr_t); i++) { //number of addresses
			printf("%d ", i);
			esp_log_buffer_hex("BLE_NVS", bda_saved[i], sizeof(esp_bd_addr_t)); //print out all addresses (including false bda`s)
			memcpy(pBLE_addr[i], bda_saved[i], sizeof(esp_bd_addr_t)); //write BDA of foodlocks to global array bda_connected

			printf("\n");
        }
        free(bda_saved); //free allocated memory by means of malloc()
    }

    // Close NVS
    nvs_close(ble_handle);
    return ESP_OK;
}
