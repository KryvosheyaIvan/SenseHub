#include "BLE_module.h"

/********************************** Semaphores *******************************************************/
xSemaphoreHandle smphr_service_search_cmplt;			// gatt service was founded

/* Semaphore service founded event */
xSemaphoreHandle gotService_FL_x[MAX_FL_NUM];

//array of DEV_FOUND_EVT semaphores
xSemaphoreHandle connect_FL_x[MAX_FL_NUM];

/* mutex /shared resource - BLE and Wi-Fi */
xSemaphoreHandle BLEmutex;

xTaskHandle fLock_com_handle;								//handle for common task

/***************************************** QUEUEs *******************************************************/

// xQueueHandle que_server_mess; //queue that holds CMD from server to foodlock
 xQueueHandle que_server_mess_v2[MAX_FL_NUM]; //queue that holds CMD from server to foodlock

 xQueueHandle que_read_char[MAX_FL_NUM]; //queue that used to transfer data from READ_CHAR_EVENT to appropriate

 xQueueHandle que_write_char[MAX_FL_NUM]; //queue that used to know that message to FoodLock was sent

 /****************************BOOLEAN variables ************************************************************/
 bool is_bda_connected[MAX_FL_NUM] = 	{ false, false, false, false, false, false, false };
 bool is_FL_opened[MAX_FL_NUM] = 		{ false, false, false, false, false, false, false };
 bool MTU_establ[MAX_FL_NUM] = 	{ false, false, false, false, false, false, false };
 bool Write_event[MAX_FL_NUM] = 		{ false, false, false, false, false, false, false };
 bool isGAP_BLE_scanning = false;			// if BLE scanning process running
 bool isDevFounded = false;					// is device was founded
 bool get_server = false;					// if BLE service was founded

 bool b_upDate = false;						//debug boolean
 bool b_debug = false;
 bool b_debug_2 = true;


 /********************************** global STRUCTURES *******************************************************/
 struct FL_x_container_v2 	server_cont_struct_v2;
 struct singleMessage 		curr_message;
 struct writeMessage 		written_message;

 /* Structure used to update BLE connection parameters */
 esp_ble_conn_update_params_t update_param_t = {
 					.latency	= 0,
 					.min_int	= 200,		// 350
 					.max_int	= 250,		// 500
 					.timeout	= 0x0C80 //  3200*10ms = 32 000ms = 32sec
 };

 /********************** SERVICES ***************************/
 //this is service that is founded in FoodLock
 esp_bt_uuid_t open_close_service = {
     .len = ESP_UUID_LEN_128,
     .uuid.uuid128 =  open_close_backw		// open_close_backw
 };

 esp_bt_uuid_t FL_time_service = {
     .len = ESP_UUID_LEN_128,
     .uuid.uuid128 =  foodlock_time_backw	// foodlock_time_backw
 };

 esp_bt_uuid_t state_service = {
     .len = ESP_UUID_LEN_128,
     .uuid.uuid128 =  state_backw			// state_backw
 };

 esp_bt_uuid_t init_service = {
     .len = ESP_UUID_LEN_128,
     .uuid.uuid128 =  init_backw				// init_backw
 };

 esp_bt_uuid_t week_service = {
     .len = ESP_UUID_LEN_128,
     .uuid.uuid128 =  week_backw				// week_backw
 };
 /*********************end services******************************/

 /******************** CHARACTERISTIC`S **************************/
 esp_bt_uuid_t open_now_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   open_now_backw 		// open_now_backw
 };

 esp_bt_uuid_t close_now_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   close_now_backw 		// close_now_backw
 };

 esp_bt_uuid_t battery_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   battery_backw 		// battery_backw
 };

 esp_bt_uuid_t currTime_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   curr_time_backw 		// curr_time_backw
 };

 esp_bt_uuid_t lock_cond_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   lock_cond_back 		// lock_condition_backw
 };

 esp_bt_uuid_t pin_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   pin_backw 			// pin_backw
 };

 esp_bt_uuid_t foodLockmode_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   foodLock_mode_backw 	// foodLock_mode_backw
 };

 esp_bt_uuid_t reserved_1_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   reserved_1_backw 	// reserved_1_backw
 };

 esp_bt_uuid_t set_time_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   set_time_backw 		// set_time_backw
 };

 esp_bt_uuid_t lock_time_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   lock_time_backw 		// lock_time_backw
 };

 esp_bt_uuid_t unlock_time_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   unlock_time_backw 	// unlock_time_backw
 };

 esp_bt_uuid_t SunLockHour_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   SunLockHour_backw 	// SunLockHour_backw
 };

 esp_bt_uuid_t SunLockMin_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   SunLockMin_backw 	// SunLockMin_backw
 };

 esp_bt_uuid_t SunUnlockHour_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   SunUnlockHour_backw 	// SunLockMin_backw
 };

 esp_bt_uuid_t SunUnlockMin_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   SunUnlockMin_backw 	// SunLockMin_backw
 };

 esp_bt_uuid_t Initialize_char = {
     .len = ESP_UUID_LEN_128,
     .uuid .uuid128 =   initialize_backw 	// SunLockMin_backw
 };

 /*********************end characteristics******************************/

 /* One gatt-based profile one app_id and one gattc_if, this array will store the gattc_if returned by ESP_GATTS_REG_EVT */
 struct gattc_profile_inst gl_profile_tab[MAX_FL_NUM] = { //MAX_FL_NUM

 	//first foodlock
     [fLOCK_ID_A] = {
         .gattc_cb = gattc_profile_x_event_handler, //gattc_profile_event_handler,
         .gattc_if = ESP_GATT_IF_NONE,       // Not get the gatt_if, so initial is ESP_GATT_IF_NONE
     },


 	//second FoodLock
     [fLOCK_ID_B] = {
         .gattc_cb =gattc_profile_x_event_handler,// gattc_profile_x_event_handler_for_B, //gattc_profile_b_event_handler, //event handler for second FoodLock_B
         .gattc_if = ESP_GATT_IF_NONE,       // Not get the gatt_if, so initial is ESP_GATT_IF_NONE
     },


 	//third FoodLock
     [fLOCK_ID_C] = {
         .gattc_cb = gattc_profile_x_event_handler, //event handler for third FoodLock_C
         .gattc_if = ESP_GATT_IF_NONE,       // Not get the gatt_if, so initial is ESP_GATT_IF_NONE
     },

 	//fourth FoodLock
     [fLOCK_ID_D] = {
         .gattc_cb = gattc_profile_x_event_handler, //event handler for fourth FoodLock_D
         .gattc_if = ESP_GATT_IF_NONE,       // Not get the gatt_if, so initial is ESP_GATT_IF_NONE
     },

 	//fifth FoodLock
     [fLOCK_ID_E] = {
         .gattc_cb = gattc_profile_x_event_handler, //event handler for fifth FoodLock_E
         .gattc_if = ESP_GATT_IF_NONE,       // Not get the gatt_if, so initial is ESP_GATT_IF_NONE
     },

 	//sixth FoodLock
     [fLOCK_ID_F] = {
         .gattc_cb = gattc_profile_x_event_handler, //event handler for sixth FoodLock_F
         .gattc_if = ESP_GATT_IF_NONE,       // Not get the gatt_if, so initial is ESP_GATT_IF_NONE
     },

 	//seventh FoodLock
     [fLOCK_ID_G] = {
         .gattc_cb = gattc_profile_x_event_handler, //event handler for seventh FoodLock_G
         .gattc_if = ESP_GATT_IF_NONE,       // Not get the gatt_if, so initial is ESP_GATT_IF_NONE
     },

 };

 /**************************other variables *****************************************************************/
uint8_t PROFILE_NUM = 0; 									// current number of registered devices
uint8_t globalIndex[MAX_FL_NUM];							// auxiliary array
char remote_device_name[20] = "Initial"; 					// String what is seeked in BLE air
static esp_gattc_char_elem_t *char_elem_result   = NULL;	// pointer to char`s structure

//CMDs to send to FoodLock
const char open_cmd [10] = "open";
const char close_cmd [10] = "close";

// FreeRTOS event group to signal when we are connected & ready to make a request
extern EventGroupHandle_t wifi_event_group;
extern const int CONNECTED_BIT;

/* Array where BLE addresses of already connected foodlocks are stored */
esp_bd_addr_t bda_connected[MAX_FL_NUM];



/* old app_main(). Init everything and start BLE task controlling FoodLock */
void initBLE(void)
{
	esp_err_t ret;
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT(); //default settings
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(GATTC_TAG, "%s initialize controller failed, error code = %x\n", __func__, ret);
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE); //enable controller in BLE mode
    if (ret) {
        ESP_LOGE(GATTC_TAG, "%s enable controller failed, error code = %x\n", __func__, ret);
        return;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(GATTC_TAG, "%s init bluetooth failed, error code = %x\n", __func__, ret);
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(GATTC_TAG, "%s enable bluetooth failed, error code = %x\n", __func__, ret);
        return;
    }

    //register the  callback function to the gap module
    ret = esp_ble_gap_register_callback(esp_gap_cb);
    if (ret){
        ESP_LOGE(GATTC_TAG, "%s gap register failed, error code = %x\n", __func__, ret);
        return;
    }

    //register the callback function to the gattc module
    ret = esp_ble_gattc_register_callback(esp_gattc_cb);
    if(ret){
        ESP_LOGE(GATTC_TAG, "%s gattc register failed, error code = %x\n", __func__, ret);
        return;
    }

    /* Register maximum allowable number of BT devices */
    ret = esp_ble_gattc_app_register(fLOCK_ID_A);
    if (ret){
        ESP_LOGE(GATTC_TAG, "%s gattc app register failed, error code = %x\n", __func__, ret);
    }

	ret = esp_ble_gattc_app_register(fLOCK_ID_B);
    if (ret){
        ESP_LOGE(GATTC_TAG, "%s gattc app register failed, error code = %x\n", __func__, ret);
    }

	ret = esp_ble_gattc_app_register(fLOCK_ID_C);
    if (ret){
        ESP_LOGE(GATTC_TAG, "%s gattc app register failed, error code = %x\n", __func__, ret);
    }

	ret = esp_ble_gattc_app_register(fLOCK_ID_D);
    if (ret){
        ESP_LOGE(GATTC_TAG, "%s gattc app register failed, error code = %x\n", __func__, ret);
    }

	ret = esp_ble_gattc_app_register(fLOCK_ID_E);
    if (ret){
        ESP_LOGE(GATTC_TAG, "%s gattc app register failed, error code = %x\n", __func__, ret);
    }

	ret = esp_ble_gattc_app_register(fLOCK_ID_F);
    if (ret){
        ESP_LOGE(GATTC_TAG, "%s gattc app register failed, error code = %x\n", __func__, ret);
    }

    /* Set local maximum transfer unit for esp32 */
    ret = esp_ble_gatt_set_local_mtu(500);
    if (ret){
        ESP_LOGE(GATTC_TAG, "set local  MTU failed, error code = %x", ret);
    }

    ret = restore_saved_bda(bda_connected);
    if (ret)
    {
    	ESP_LOGE(GATTC_TAG, " \n Problems write/read from NVS in BLE mode");
    }
}

/* Bluetooth GAP callback */
void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    uint8_t *adv_name = NULL; //complete name
    uint8_t adv_name_len = 0;

    uint8_t *short_name = NULL; //shortened name
    uint8_t short_name_len = 0;

    switch (event) {
    	//event - scanning parameters are set
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
        //the unit of the duration is second
        uint32_t duration = 15;  //20
        esp_ble_gap_start_scanning(duration); //start scanning for GATT servers for 30 seconds
        //after scanning ESP_GAP_SEARCH_INQ_CMPL_EVT  is triggered
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        //scan start complete event to indicate scan start successfully or failed
        if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTC_TAG, "scan start failed, error status = %x", param->scan_start_cmpl.status);
            break;
        }

        isGAP_BLE_scanning = 1;
        ESP_LOGI(GATTC_TAG, "scan start success");
        break;

    case ESP_GAP_BLE_SCAN_RESULT_EVT: {  //When one scan result ready, the event comes each time
    	//deb printf("\n When one BLE device founded \n");
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
        	//get shortened name of the device in order to know whether it is sensekitchen device
        	short_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv, ESP_BLE_AD_TYPE_NAME_SHORT, &short_name_len);
        	//deb
        	esp_log_buffer_hex(GATTC_TAG, scan_result->scan_rst.bda, 6);
        	//deb ESP_LOGI(GATTC_TAG, "searched Adv Data Len %d, Scan Response Len %d", scan_result->scan_rst.adv_data_len, scan_result->scan_rst.scan_rsp_len);
            adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv, ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);

            //deb ESP_LOGI(GATTC_TAG, "searched Device Name Len %d", adv_name_len);
            //deb esp_log_buffer_char(GATTC_TAG, adv_name, adv_name_len);
            //deb ESP_LOGI(GATTC_TAG, "\n");
            //deb ESP_LOGI(GATTC_TAG, "Shortened name  Len %d", short_name_len);
             esp_log_buffer_char(GATTC_TAG, short_name, short_name_len);


             	 	isDevFounded = true;
                	/* Check whether current device was saved in flash memory */
                	isDeviceFounded(scan_result->scan_rst.bda);

            break;
        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
        	 ESP_LOGI(GATTC_TAG, " 30 seconds?");
        	 esp_restart();
            break;
        default:
            break;
        }
        break;
    }

    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
            ESP_LOGE(GATTC_TAG, "scan stop failed, error status = %x", param->scan_stop_cmpl.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "stop scan successfully");
        break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
            ESP_LOGE(GATTC_TAG, "adv stop failed, error status = %x", param->adv_stop_cmpl.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "stop adv successfully");
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
		 esp_log_buffer_hex(GATTC_TAG, param->update_conn_params.bda, sizeof(esp_bd_addr_t));
         ESP_LOGI(GATTC_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                  param->update_conn_params.status,
                  param->update_conn_params.min_int,
                  param->update_conn_params.max_int,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
        break;
    case ESP_GAP_BLE_AUTH_CMPL_EVT:
    	ESP_LOGE(GATTC_TAG, "ESP_GAP_BLE_AUTH_CMPL_EVT");
    	break;
    default:
        break;
    }
}

/* BLE GATTC callback function */
void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{


    // If the gattc_if equal to profile A, call profile A callback handler,
    // so here call each profile's callback
   // gattc_profile_x_event_handler( event,  gattc_if, param);


}

/* Create different queues and semaphores */
void create_que_and_semaphr(void)
{
	smphr_service_search_cmplt = xSemaphoreCreateBinary(); //semaphore - service was founded
	BLEmutex = xSemaphoreCreateMutex();						//	shared resource - antenna

    /* Creating semaphores for each FoodLock (profiles) */
    for (uint8_t num = 0; num < MAX_FL_NUM; num++ )
    {
    	connect_FL_x[num] = xSemaphoreCreateBinary();
    	gotService_FL_x[num] = xSemaphoreCreateBinary();

        /* New version */
        que_server_mess_v2[num] = xQueueCreate(1,sizeof(server_cont_struct_v2));
        	if ( que_server_mess_v2[num] == NULL)
        	{
        	    ESP_LOGE(GATTC_TAG, "que_server_mess_v2[%d] can`t be created", num);
        	}

        	que_read_char[num] = xQueueCreate(1,sizeof(curr_message));	//5				// create queue used to transfer read charact. data
        	if ( que_read_char[num] == NULL)
        	{
        	    ESP_LOGE(GATTC_TAG, "que_read_char[%d] can`t be created", num);
        	}


        	que_write_char[num] = xQueueCreate(1,sizeof(written_message));				// create queue used to know
        	if ( que_write_char[num] == NULL)											// that data was sent to FoodLock
        	{
        	    ESP_LOGE(GATTC_TAG, "que_write_char[%d] can`t be created", num);
        	}

    }
}

/* Check whether current device was saved in flash memory */
bool isDeviceFounded(esp_bd_addr_t* curr_bda)
{

	for ( uint8_t index = 0; index < MAX_FL_NUM; index++) //PROFILE_NUM
	{
		//compare founded devName to saved
		//and check if current wasn`t already connected
		if( !is_bda_connected[index] && (memcmp(curr_bda, bda_connected[index], 6) == 0) )
		{//if equal and not connected yet
        	ESP_LOGI(GATTC_TAG, "searched device %s\n", remote_device_name);
        	esp_log_buffer_hex(GATTC_TAG, curr_bda, 6);

        	ESP_LOGI(GATTC_TAG, "connect to the remote device %d", index);
        	//ESP_LOGI(GATTC_TAG,"\nisDeviceFounded xEventGroupWaitBits\n");
        	//даємо семафор для таска (знайдено  пристрій, будь-який)
        	//дати дозвіл на таск, що шукаэ сервіси
        	xSemaphoreGive(connect_FL_x[index]);

    		ESP_LOGI("FL task creation", "Task index %d created", index);
    		//globalIndex = index;
    		globalIndex[index] = index;
    	    //old createFoodLock_task_param( &globalIndex); //тут треба передавати не тільки індекс_проф а й uuid
    		createFoodLock_task_param( &globalIndex[index]); //тут треба передавати не тільки індекс_проф а й uuid
        	is_bda_connected[index] = true; //mark that address as connected
        	return true;
		}
	}
	return false;
}


/* Create common Food Lock task with parameter */
void createFoodLock_task_param(uint8_t* pParam)
{
	printf("\n Creating food lock common task");
	xTaskCreatePinnedToCore(FLock_task_idx_v2, "FL_tsk2", 3072, (void*)pParam, configMAX_PRIORITIES - 1, &fLock_com_handle, tskNO_AFFINITY);//4096
}

/* Common structure for each foodlocks tasks to create separate objects */
void FLock_task_idx_v2(void *pvParameters)
{
	uint8_t profileNum = *((uint8_t*)pvParameters);
	struct FL_x_container_v2 FL_x_str_v2;

	while(1)
	{

		xQueueReceive(que_server_mess_v2[profileNum], &FL_x_str_v2, portMAX_DELAY);		//forever waiting for message

		ESP_LOGE(GATTC_TAG,"is_FL_opened[%d] = %d",profileNum,is_FL_opened[profileNum]);
		ESP_LOGE(GATTC_TAG,"Waiting until FL[%d] is opened",profileNum);

		while (!is_FL_opened[profileNum])
		{
			vTaskDelay(200 / portTICK_PERIOD_MS);
			ESP_LOGE(GATTC_TAG, "*[%d]",profileNum);
		}

		xSemaphoreTake(BLEmutex, portMAX_DELAY);

		printf("\n profileNum: %d",profileNum);
		printf("\n CMD: %s",FL_x_str_v2.setOpen);
		printf("\n CMD LEN: %d", strlen(FL_x_str_v2.setOpen));

		send_CMD_to_FL_x_v2(&FL_x_str_v2,profileNum);
/*
		if (FL_x_str_v2.updateLock == 1)
		{
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			init_lock_response_struct();												// init lock_response_struct with an error code

			b_upDate = true;
			readAllDataOnFoodLock(profileNum);											// read all char`s
			b_upDate = false;

			printLockResponse();														// print data got from FoodLock
		}

		xSemaphoreGive(BLEmutex);

		printf("\n Common task_v2 %d running \n",profileNum );
		vTaskDelay(500 / portTICK_PERIOD_MS);
		*/
	}
}

/* Function sends commands to smart lock */
void send_CMD_to_FL_x_v2(struct FL_x_container_v2* pContainer, uint8_t prof_num)
{
	/*********************** INIT service **********************************/
	/*
	 * Custom Characteristic:	PIN
	 */
	if ( strcmp(pContainer->PIN, noCMD) != 0 )
	{
		removeQuotes(pContainer->PIN);
		writeCharact_in_Service_v3( prof_num, &init_service, &pin_char, pContainer->PIN, strlen(pContainer->PIN));
	}
	/*
	 * Custom Characteristic:	FoodlockMode
	 */
	if ( strcmp(pContainer->mode, noCMD) != 0 )
	{
		removeQuotes(pContainer->mode);
		writeCharact_in_Service_v3( prof_num, &init_service, &foodLockmode_char, pContainer->mode, strlen(pContainer->mode));
	}
	/*
	 * Custom Characteristic:	Initialize
	 */
	if ( strcmp(pContainer->signal, noCMD) != 0 )
	{
		removeQuotes(pContainer->signal);
		writeCharact_in_Service_v3( prof_num, &init_service, &Initialize_char, pContainer->signal, strlen(pContainer->signal));
	}

	/*********************** OPEN CLOSE service **********************************/
	/*
	 * Custom Characteristic:	OpenNow
	 * Custom Characteristic:	CloseNow
	 */
	if ( strcmp(pContainer->setOpen, noCMD) != 0 )
	{
		printf("\n pContainer->setOpen: %s", pContainer->setOpen);
		removeQuotes(pContainer->setOpen);
		printf("\n pContainer->setOpen ska: %s", pContainer->setOpen);


		if ( strcmp(pContainer->setOpen, open_cmd) == 0 ) //open
		{
			printf("\n CMD equals open_cmd ");
			//printf("\n prof_num: %d", prof_num);
			printf("\n LEN: %d", strlen(open_cmd));
			char* ska = "open";
			writeCharact_in_Service_v3( prof_num, &open_close_service, &open_now_char, ska, 4);

		}
		if ( strcmp(pContainer->setOpen, close_cmd) == 0 ) //close
		{
			printf("\n CMD equals close_cmd ");
			//LEN printf("\n prof_num: %d", prof_num);
			printf("\n LEN: %d", strlen(close_cmd));
			char* ska1 = "close";
			writeCharact_in_Service_v3( prof_num, &open_close_service, &close_now_char, ska1, 5);

		}
	}

	/*********************** FOODLOCK TIME service **********************************/
	/*
	 * Custom Characteristic:	SetTime
	 */
	//New style
	if ( (pContainer->setTimeH != errZaych) && (pContainer->setTimeM != errZaych) )
	{
		char setFL_xtime[15];
		sprintf(setFL_xtime, "%d.%d.00.5", pContainer->setTimeH, pContainer->setTimeM);
		printf(" New Vlad style time on FoodLock: %s",setFL_xtime);
		writeCharact_in_Service_v3( prof_num, &FL_time_service, &set_time_char, setFL_xtime, strlen(setFL_xtime));
	}
	/* Old style
	if ( (strcmp(pContainer->setTimeH, noCMD) != 0) && (strcmp(pContainer->setTimeM, noCMD) != 0) )
	{
		removeQuotes(pContainer->setTimeH);
		removeQuotes(pContainer->setTimeM);
		char setFL_xtime[15];
		sprintf(setFL_xtime, "%s.%s.00.1",pContainer->setTimeH,pContainer->setTimeM);

		printf(" New time on FoodLock: %s",setFL_xtime);
		writeCharact_in_Service_v3( prof_num, &FL_time_service, &set_time_char, setFL_xtime, strlen(setFL_xtime));
	}
	*/

	/*********************** WEEK service **********************************/
	/*
	 * Custom Characteristics:	SunLockHour
	 * 							SunLockMin
	 * 							SunUnlockHour
	 * 							SunUnlockMin
	 */
							/* Monday */
	if ( (*pContainer->Mon_LockTime_hour != errZaych) && (*pContainer->Mon_LockTime_min != errZaych)
			&& (*pContainer->Mon_UnlockTime_hour != errZaych) && (*pContainer->Mon_UnlockTime_min != errZaych)  )
	{
		writeCharact_in_Service_v3( prof_num, &week_service, &SunLockHour_char, (char*)pContainer->Mon_LockTime_hour , TIME_LEN);
		writeCharact_in_Service_v3( prof_num, &week_service, &SunLockMin_char, (char*)pContainer->Mon_LockTime_min , TIME_LEN);
		writeCharact_in_Service_v3( prof_num, &week_service, &SunUnlockHour_char, (char*)pContainer->Mon_UnlockTime_hour , TIME_LEN);
		writeCharact_in_Service_v3( prof_num, &week_service, &SunUnlockMin_char, (char*)pContainer->Mon_UnlockTime_min , TIME_LEN);
	}
							/* Tuesday */
	if ( (*pContainer->Tue_LockTime_hour != errZaych) && (*pContainer->Tue_LockTime_min != errZaych)
			&& (*pContainer->Tue_UnlockTime_hour != errZaych) && (*pContainer->Tue_UnlockTime_min != errZaych)  )
	{
		wrapperWeekWrite(prof_num, pContainer->Tue_LockTime_hour, pContainer->Tue_LockTime_min,
							pContainer->Tue_UnlockTime_hour, pContainer->Tue_UnlockTime_min);
	}
							/* Wednesday */
	if ( (*pContainer->Wedn_LockTime_hour != errZaych) && (*pContainer->Wedn_LockTime_min != errZaych)
			&& (*pContainer->Wedn_UnlockTime_hour != errZaych) && (*pContainer->Wedn_UnlockTime_min != errZaych)  )
	{
		wrapperWeekWrite(prof_num, pContainer->Wedn_LockTime_hour, pContainer->Wedn_LockTime_min,
							pContainer->Wedn_UnlockTime_hour, pContainer->Wedn_UnlockTime_min);
	}
							/* Thursday */
	if ( (*pContainer->Thur_LockTime_hour != errZaych) && (*pContainer->Thur_LockTime_min != errZaych)
			&& (*pContainer->Thur_UnlockTime_hour != errZaych) && (*pContainer->Thur_UnlockTime_min != errZaych)  )
	{
		wrapperWeekWrite(prof_num, pContainer->Thur_LockTime_hour, pContainer->Thur_LockTime_min,
							pContainer->Thur_UnlockTime_hour, pContainer->Thur_UnlockTime_min);
	}
							/* Friday */
	if ( (*pContainer->Fri_LockTime_hour != errZaych) && (*pContainer->Fri_LockTime_min != errZaych)
			&& (*pContainer->Fri_UnlockTime_hour != errZaych) && (*pContainer->Fri_UnlockTime_min != errZaych)  )
	{
		wrapperWeekWrite(prof_num, pContainer->Fri_LockTime_hour, pContainer->Fri_LockTime_min,
							pContainer->Fri_UnlockTime_hour, pContainer->Fri_UnlockTime_min);
	}
							/* Saturday */
	if ( (*pContainer->Sat_LockTime_hour != errZaych) && (*pContainer->Sat_LockTime_min != errZaych)
			&& (*pContainer->Sat_UnlockTime_hour != errZaych) && (*pContainer->Sat_UnlockTime_min != errZaych)  )
	{
		wrapperWeekWrite(prof_num, pContainer->Sat_LockTime_hour, pContainer->Sat_LockTime_min,
							pContainer->Sat_UnlockTime_hour, pContainer->Sat_UnlockTime_min);
	}
							/* Sunday */
	if ( (*pContainer->Sun_LockTime_hour != errZaych) && (*pContainer->Sun_LockTime_min != errZaych)
			&& (*pContainer->Sun_UnlockTime_hour != errZaych) && (*pContainer->Sun_UnlockTime_min != errZaych)  )
	{
		wrapperWeekWrite(prof_num, pContainer->Sun_LockTime_hour, pContainer->Sun_LockTime_min,
							pContainer->Sun_UnlockTime_hour, pContainer->Sun_UnlockTime_min);
	}
}

/* Function removes quotes from string
 * "Family" --> Family
 */
void removeQuotes(char* pInString)
{
	//deb printf("\n Before : %s", pInString);
	uint8_t len = strlen(pInString);

	for (uint8_t i = 0; i < len; i++)
	{
		pInString[i] = pInString[i+1];
	}

	pInString[len-2] = '\0'; //null terminating symbol
	//deb printf("\n After : %s", pInString);
}

/* Function just writes necessary data to Week service */
void wrapperWeekWrite(uint8_t prof_num, uint8_t *lock_h, uint8_t *lock_m, uint8_t *unlock_h, uint8_t *unlock_m)
{
	writeCharact_in_Service_v3( prof_num, &week_service, &SunLockHour_char, (char*)lock_h, TIME_LEN);
	vTaskDelay(1000/portMAX_DELAY);

	writeCharact_in_Service_v3( prof_num, &week_service, &SunLockMin_char, (char*)lock_m, TIME_LEN);
	vTaskDelay(1000/portMAX_DELAY);

	writeCharact_in_Service_v3( prof_num, &week_service, &SunUnlockHour_char, (char*)unlock_h, TIME_LEN);
	vTaskDelay(1000/portMAX_DELAY);

	writeCharact_in_Service_v3( prof_num, &week_service, &SunUnlockMin_char, (char*)unlock_m, TIME_LEN);
	vTaskDelay(1000/portMAX_DELAY);
}

/* function writes value to a specific character in specific service */
void writeCharact_in_Service_v3(uint8_t fLOCK_ID_x, esp_bt_uuid_t* pService, esp_bt_uuid_t* pCharacteristic, const char* CMD, uint8_t length)
{

	//deb printf("\n writeCharact_in_Service_v3() \n");
	get_server = false;
	vTaskDelay(100 / portTICK_PERIOD_MS); //speed
	esp_ble_gattc_search_service(gl_profile_tab[fLOCK_ID_x].gattc_if, gl_profile_tab[fLOCK_ID_x].conn_id, pService);//discover services



	//Wait forever until MTU_EVT occ

	//Wait forever while service is not founded
	//deb printf("\n Forever waiting till service founded \n");
	vTaskDelay(100 / portTICK_PERIOD_MS);
	xSemaphoreTake(smphr_service_search_cmplt, portMAX_DELAY); //forever



    if (get_server)
    {
        uint16_t count = 2; //number of char`s we want to find
        esp_gatt_status_t status = esp_ble_gattc_get_attr_count( gl_profile_tab[fLOCK_ID_x].gattc_if,
        														 gl_profile_tab[fLOCK_ID_x].conn_id,
                                                                 ESP_GATT_DB_CHARACTERISTIC,
                                                                 gl_profile_tab[fLOCK_ID_x].service_start_handle,
                                                                 gl_profile_tab[fLOCK_ID_x].service_end_handle,
                                                                 INVALID_HANDLE,
                                                                 &count);

        vTaskDelay(100 / portTICK_PERIOD_MS); //speed

        //debug printf("\n Count = %d", count);
        if (status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_attr_count error");
        }

        if (count > 0){
        //debug	printf(" \n There is at least 1 characteristic \n");
            char_elem_result = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
            if (!char_elem_result){
                ESP_LOGE(GATTC_TAG, "gattc no mem");
            }else{
                status = esp_ble_gattc_get_char_by_uuid( gl_profile_tab[fLOCK_ID_x].gattc_if,
                										 gl_profile_tab[fLOCK_ID_x].conn_id,
                                                         gl_profile_tab[fLOCK_ID_x].service_start_handle,
                                                         gl_profile_tab[fLOCK_ID_x].service_end_handle,
														 *pCharacteristic,
                                                         char_elem_result,
                                                         &count);



				//deb esp_log_buffer_hex("Char address", char_elem_result->uuid.uuid.uuid128, 16); //print out all addresses (including false bda`s)

				//speed ESP_LOGI(GATTC_TAG,"%d",char_elem_result->char_handle);
				ESP_LOGI(GATTC_TAG,"\n Connection ID: %d", gl_profile_tab[fLOCK_ID_x].conn_id);
				//speed ESP_LOGI(GATTC_TAG,"\n gattc_if: %d", gl_profile_tab[fLOCK_ID_x].gattc_if);
				//speed ESP_LOGI(GATTC_TAG,"\n fLOCK_ID_x: %d", fLOCK_ID_x);

                vTaskDelay(100 / portTICK_PERIOD_MS);
                //speed printf("\n before esp_ble_gattc_write_char");
                /* Write characteristics value */
                esp_err_t ret_write = -2;

                ret_write = esp_ble_gattc_write_char(  gl_profile_tab[fLOCK_ID_x].gattc_if,
                						   gl_profile_tab[fLOCK_ID_x].conn_id,
										   	char_elem_result->char_handle,
                                            length,								//+
											CMD,								//+
                                            ESP_GATT_WRITE_TYPE_RSP,			//+ ESP_GATT_WRITE_TYPE_NO_RSP
											ESP_GATT_AUTH_REQ_NONE);			//+ ESP_GATT_AUTH_REQ_NONE

                //speed printf("\n after esp_ble_gattc_write_char");
                printf("\n return code: %d", ret_write);
                vTaskDelay(200 / portTICK_PERIOD_MS); //speed

                ESP_LOGE(GATTC_TAG, "Forever waiting until written");

                uint8_t not_wr_c = 0;
                while ( !Write_event[fLOCK_ID_x] )
                {
                	not_wr_c++;
                	ESP_LOGE(GATTC_TAG, "not written yet");
                	vTaskDelay(400 / portTICK_PERIOD_MS);  //200
                }
                ESP_LOGI(GATTC_TAG, "WRITTEN[%d]",fLOCK_ID_x);
                Write_event[fLOCK_ID_x] = false;

                if (status != ESP_GATT_OK){
                    ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_char_by_uuid error");
                }
            }
            // free char_elem_result
            free(char_elem_result);
        }else{
            ESP_LOGE(GATTC_TAG, "no char found");
        }
    }//end of if(get_server)
}

/* Common GAP GATTC event handler for every device (gattc_if) */
void gattc_profile_x_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;

    uint8_t profile_index = 0;
    profile_index =  idx_Of_GATTC_if(gattc_if);
    //deb printf("\n profile_index = %d \n", profile_index);//deb

    switch (event) {
    case ESP_GATTC_ACL_EVT :{
  	  ESP_LOGI(LOCK_x_TAG, "ESP_GATTC_ACL_EVT");
    }
    case ESP_GATTC_CONGEST_EVT:{
  	  ESP_LOGE(LOCK_x_TAG, "ESP_GATTC_CONGEST_EVT");
  	  break;
    }
    case ESP_GATTC_EXEC_EVT:{
  	  ESP_LOGI(LOCK_x_TAG, "ESP_GATTC_EXEC_EVT");
    }
    case ESP_GATTC_CANCEL_OPEN_EVT:
    {
  	  ESP_LOGE(LOCK_x_TAG, "ESP_GATTC_CANCEL_OPEN_EVT");
    }
    case ESP_GATTC_REG_EVT:
        ESP_LOGI(LOCK_x_TAG, "REG_EVT");
        printf("\n GATT client is registered , gattc_if = %d \n", gattc_if);//deb
        break;
    case ESP_GATTC_CONNECT_EVT:{
        break;
    }
    case ESP_GATTC_OPEN_EVT:{ //When GATT virtual connection is set up, the event comes
  	  ESP_LOGI(LOCK_x_TAG, "OPEN_EVT");
  	  esp_log_buffer_hex(LOCK_x_TAG, param->open.remote_bda, sizeof(esp_bd_addr_t));
        if (param->open.status != ESP_GATT_OK){
            ESP_LOGE(LOCK_x_TAG, "open failed, status %d", p_data->open.status);
           // esp_ble_gattc_open(gl_profile_tab[index].gattc_if, bda_connected[index], true); //tries to open a connection to the remote device
            vTaskDelay(400 / portTICK_PERIOD_MS); //500!
            ESP_LOGI(LOCK_x_TAG, "Reopening...");
            ESP_LOGI(LOCK_x_TAG, "gattc_if: %d", gattc_if);
            ESP_LOGI(LOCK_x_TAG, "BDA:");
            esp_log_buffer_hex(LOCK_x_TAG, bda_connected[profile_index], sizeof(esp_bd_addr_t));
            //new vers esp-idf esp_ble_gattc_open(gattc_if, bda_connected[profile_index], true); //tries to REopen a connection to the remote device
            break;
        }

        //p_data->open.conn_id +

        ESP_LOGI(LOCK_x_TAG, "open FL_%d success", profile_index);
        gl_profile_tab[profile_index].conn_id = p_data->open.conn_id;

        printf("\n GATT client is registered , gattc_if = %d \n", gattc_if);//deb

        memcpy(gl_profile_tab[profile_index].remote_bda, p_data->open.remote_bda, sizeof(esp_bd_addr_t)); //copy bda to gl_profile_tab[index]
        ESP_LOGI(LOCK_x_TAG, "REMOTE BDA:");
        esp_log_buffer_hex(LOCK_x_TAG, gl_profile_tab[profile_index].remote_bda, sizeof(esp_bd_addr_t));

        ESP_LOGI(LOCK_x_TAG, "Sending MTU request...");
        vTaskDelay(200 / portTICK_PERIOD_MS);//deb 2000 !

        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gl_profile_tab[profile_index].gattc_if, p_data->open.conn_id);
        vTaskDelay(800 / portTICK_PERIOD_MS);//deb 800!

        int32_t error_code = (int32_t)mtu_ret;
       //deb printf(" \n esp_ble_gattc_send_mtu_req error code = %d \n",error_code);
        if (mtu_ret){
            ESP_LOGE(LOCK_x_TAG, "config MTU error, error code = %x", mtu_ret);
        }
        break;
    }
    case ESP_GATTC_CFG_MTU_EVT:  //When configuration of MTU completes, the event comes
    	//deb printf("\n Start searching service in FL_A... \n");/////////////////////vanya
        if (param->cfg_mtu.status != ESP_GATT_OK){
            ESP_LOGE(LOCK_x_TAG,"config mtu failed, error status = %x", param->cfg_mtu.status);
            MTU_establ[profile_index] = false;
            break;//(vanya)
        }
        ESP_LOGI(LOCK_x_TAG, "ESP_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
        esp_log_buffer_hex(LOCK_x_TAG, gl_profile_tab[profile_index].remote_bda, sizeof(esp_bd_addr_t));
        //ESP_LOGI(LOCK_x_TAG,"Remote bda[%d]: ",profile_index);
        //esp_log_buffer_hex(GATTC_TAG, p_data->search_res.srvc_id.uuid.uuid.uuid128, 16);
        //give semaphore
        //xSemaphoreGive(semaphore_MTU_FL_x[profile_index]);//give semaphore to task
        ESP_LOGI(LOCK_x_TAG,"MTU_establ[%d]",profile_index);
        MTU_establ[profile_index] = true;

        ESP_LOGE(LOCK_x_TAG,"Food Lock[%d] opened",profile_index);
        is_FL_opened[profile_index] = true;
        esp_ble_gap_set_prefer_conn_params(bda_connected[profile_index],
                                           update_param_t.min_int, update_param_t.max_int,
											 update_param_t.latency, update_param_t.timeout);
        memcpy(update_param_t.bda, bda_connected[profile_index], sizeof(esp_bd_addr_t));
        esp_ble_gap_update_conn_params(&update_param_t);

        break;
    case ESP_GATTC_SEARCH_RES_EVT: {  //When GATT service discovery result is got, the event comes
    	// printf("GATT service discovery result is got, the event comes \n");
        //deb ESP_LOGI(GATTC_TAG, "ESP_GATTC_SEARCH_RES_EVT");

  	  //p_data->search_res.conn_id +

        esp_gatt_srvc_id_t *srvc_id =(esp_gatt_srvc_id_t *)&p_data->search_res.srvc_id;


        if (srvc_id->id.uuid.len == ESP_UUID_LEN_128 ) {
      	  //&&  p_data->search_res.srvc_id.uuid.uuid.uuid128 == gl_curr_serv_uuid128) {
            //deb ESP_LOGI(GATTC_TAG, "service found");
      	  printf("\n");
      	  //deb esp_log_buffer_hex("search_res_stable", p_data->search_res.srvc_id.uuid.uuid.uuid128, 16);

            gl_profile_tab[profile_index].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[profile_index].service_end_handle = p_data->search_res.end_handle;
            get_server = true;
            xSemaphoreGive(gotService_FL_x[profile_index]);
        }

        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:                               //When GATT service discovery is completed, the event comes
    	//deb printf ("\n GATT service discovery is completed \n");

  	  // p_data->search_cmpl.conn_id +
        if (p_data->search_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(LOCK_x_TAG, "search service failed, error status = %x", p_data->search_cmpl.status);
            break;
        }
        //deb ESP_LOGI(GATTC_TAG, "ESP_GATTC_SEARCH_CMPL_EVT");
        //if (param->search_cmpl.)
        xSemaphoreGive(smphr_service_search_cmplt);//give semaphore to task
         break;

    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
    	printf(" \n Registered for notification!");
        break;
    }
    case ESP_GATTC_NOTIFY_EVT:
    	printf(" \n ESP_GATTC_NOTIFY_EVT!");
        break;
    case ESP_GATTC_WRITE_DESCR_EVT:
    	printf("\n GATT characteristic descriptor write completes \n");
        break;
    case ESP_GATTC_SRVC_CHG_EVT: {
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(LOCK_x_TAG, "ESP_GATTC_SRVC_CHG_EVT, bd_addr:");
        esp_log_buffer_hex(LOCK_x_TAG, bda, sizeof(esp_bd_addr_t));
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:
    	//deb printf(" \n When GATT characteristic write operation complete \n");
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(LOCK_x_TAG, "write char failed, error status = %x", p_data->write.status);
            break;
        }

        ESP_LOGI(LOCK_x_TAG, "write char FL[%d] success error status = %x", profile_index, p_data->write.status );
        //deb ESP_LOGI(LOCK_x_TAG, "write char FL[%d] success offset = %x", profile_index, p_data->write.offset );

        if ( b_upDate)
        {
  		// Copy message, length and charact. handle to "written_message" structure
  		written_message.ch_handle = p_data->write.handle;
  		xQueueSend(que_write_char[profile_index], &written_message,5000 / portTICK_PERIOD_MS ); ////Send all fields to FLock_task_idx
        }

        if ( b_debug)
        {
  		// Copy message, length and charact. handle to "written_message" structure
  		written_message.ch_handle = p_data->write.handle;
  		xQueueSend(que_write_char[profile_index], &written_message,5000 / portTICK_PERIOD_MS ); ////Send all fields to FLock_task_idx
        }

        if ( b_debug_2)
        {
      	  // it works!!
      	  printf("\n Give semaphore");
      	  Write_event[profile_index] =  true;
        }

        break;
    case ESP_GATTC_DISCONNECT_EVT:
  	  ESP_LOGE(LOCK_x_TAG, "ESP_GATTC_DISCONNECT_EVT");
        if (memcmp(p_data->disconnect.remote_bda, gl_profile_tab[profile_index].remote_bda, 6) == 0){
             ESP_LOGI(LOCK_x_TAG, "device FL_[%d] disconnected",profile_index);

             ESP_LOGI(LOCK_x_TAG, "Reopening...");
             ESP_LOGI(LOCK_x_TAG, "gattc_if: %d", gattc_if);
             ESP_LOGI(LOCK_x_TAG, "BDA:");
             vTaskDelay(400 / portTICK_PERIOD_MS); //500!
             esp_log_buffer_hex(LOCK_x_TAG, bda_connected[profile_index], sizeof(esp_bd_addr_t));

             ESP_LOGE(LOCK_x_TAG,"Food Lock[%d] closed", profile_index);
             is_FL_opened[profile_index] = false;

             //new vers esp-idf esp_ble_gattc_open(gattc_if, bda_connected[profile_index], true); //tries to REopen a connection to the remote device
         }



        //another way to restart app
        xSemaphoreGive(BLEmutex); //??
        is_bda_connected[profile_index] = false; //mark that address as connected
        MTU_establ[profile_index] = false;
        break;
    case ESP_GATTC_READ_CHAR_EVT:
    	if ( p_data->read.status != ESP_GATT_OK)
    	{
    		printf("\n Read error \n");
    		break;
    	}
    	else
    	{
    		printf("\n Read char x event!");
    		//deb esp_log_buffer_char(LOCK_x_TAG, p_data->read.value, p_data->read.value_len);

    		// Copy message, length and charact. handle to "curr_message" structure
    		memcpy(curr_message.message, p_data->read.value, p_data->read.value_len);
    		curr_message.ch_handle = p_data->read.handle;
    		curr_message.length = p_data->read.value_len;

    		//deb printf("\n Sending data to Fl_%d... from handle %d \n", profile_index, curr_message.ch_handle);
    		xQueueSend(que_read_char[profile_index], &curr_message,5000 / portTICK_PERIOD_MS ); ////Send all fields to FLock_task_idx
    		/* equals
    		uint8_t prof_idx = idx_Of_conn_id( p_data->read.conn_id);
    		printf("\n Comparison! \n");
    		printf("\n Index from gattc_if: %d \n",profile_index);
    		printf("\n Index from conn_id: %d \n",prof_idx);
    		*/
    	}
    	break;
    case ESP_GATTC_READ_DESCR_EVT:
  		ESP_LOGI(GATTC_TAG,"ESP_GATTC_READ_DESCR_EVT ska");
      	break;
    case ESP_GATTC_QUEUE_FULL_EVT:
		ESP_LOGE("error","ska");
		break;
    case ESP_GATTC_CLOSE_EVT:
		ESP_LOGE("error","ESP_GATTC_CLOSE_EVT ska");
    	break;
    default:
        break;
    }
}


/* Function returns index of profile, where internal gattc_if equals to that, what that call callback function */
uint8_t idx_Of_GATTC_if(esp_gatt_if_t gattc_if_curr)
{
	  for(uint8_t prof_idx = 0; prof_idx < MAX_FL_NUM; prof_idx++)  //PROFILE_NUM
	  {
		  //compare data
		  if ( gl_profile_tab[prof_idx].gattc_if == gattc_if_curr)
			  {
  		  	  	  return prof_idx;
			  }
	  }

	  ESP_LOGE(GATTC_TAG, "error idx_Of_GATTC_if(). gattc current = %d \n", (uint8_t)gattc_if_curr);
	  return 0;
}

/* Function forming string of POST request for curl*/
void formStringRequest_v5(char* request, esp_bd_addr_t* pBLE_addr)
{
	uint8_t MAC_addr[6];
	uint8_t UUID_temp[6];
	esp_efuse_mac_get_default(MAC_addr);
	memcpy(UUID_temp, pBLE_addr, sizeof(esp_bd_addr_t)); //copy current UUID into temporary array

	/* form body of POST request */
	sprintf (request, "hubID=%02X%02X%02X%02X%02X%02X&lockID=%02X%02X%02X%02X%02X%02X&state=0",
				MAC_addr[0],MAC_addr[1],MAC_addr[2],MAC_addr[3],MAC_addr[4],MAC_addr[5],
				UUID_temp[0],UUID_temp[1],UUID_temp[2],UUID_temp[3],UUID_temp[4],UUID_temp[5]);

	//debugging, printing
	printf("\n Request: %s",request);
}

/* Functions initiates every field of server_cont_struct_v2  to "noCMD" string */
void initStructContainer_v2(void)
{
	 strcpy(server_cont_struct_v2.PIN,		noCMD);
	 strcpy(server_cont_struct_v2.lockName,	noCMD);
	 strcpy(server_cont_struct_v2.mode,		noCMD);
	 strcpy(server_cont_struct_v2.setOpen,	noCMD);
	 strcpy(server_cont_struct_v2.signal,	noCMD);
	 server_cont_struct_v2.setTimeH			= errZaych;
	 server_cont_struct_v2.setTimeM			= errZaych;
	 server_cont_struct_v2.updateLock		= errZaych;

	 //Initiate Time gaps with 99 (errZaych)
	 for ( uint8_t index = 0; index < TIME_LEN; index++)
	 {
		 server_cont_struct_v2.Mon_LockTime_hour[index] 	= errZaych;
		 server_cont_struct_v2.Mon_LockTime_min[index] 		= errZaych;
		 server_cont_struct_v2.Mon_UnlockTime_hour[index] 	= errZaych;
		 server_cont_struct_v2.Mon_UnlockTime_min[index] 	= errZaych;

		 server_cont_struct_v2.Tue_LockTime_hour[index] 	= errZaych;
		 server_cont_struct_v2.Tue_LockTime_min[index] 		= errZaych;
		 server_cont_struct_v2.Tue_UnlockTime_hour[index] 	= errZaych;
		 server_cont_struct_v2.Tue_UnlockTime_min[index] 	= errZaych;

		 server_cont_struct_v2.Wedn_LockTime_hour[index] 	= errZaych;
		 server_cont_struct_v2.Wedn_LockTime_min[index] 	= errZaych;
		 server_cont_struct_v2.Wedn_UnlockTime_hour[index] 	= errZaych;
		 server_cont_struct_v2.Wedn_UnlockTime_min[index] 	= errZaych;

		 server_cont_struct_v2.Thur_LockTime_hour[index] 	= errZaych;
		 server_cont_struct_v2.Thur_LockTime_min[index] 	= errZaych;
		 server_cont_struct_v2.Thur_UnlockTime_hour[index] 	= errZaych;
		 server_cont_struct_v2.Thur_UnlockTime_min[index] 	= errZaych;

		 server_cont_struct_v2.Fri_LockTime_hour[index] 	= errZaych;
		 server_cont_struct_v2.Fri_LockTime_min[index] 		= errZaych;
		 server_cont_struct_v2.Fri_UnlockTime_hour[index] 	= errZaych;
		 server_cont_struct_v2.Fri_UnlockTime_min[index] 	= errZaych;

		 server_cont_struct_v2.Sat_LockTime_hour[index] 	= errZaych;
		 server_cont_struct_v2.Sat_LockTime_min[index] 		= errZaych;
		 server_cont_struct_v2.Sat_UnlockTime_hour[index] 	= errZaych;
		 server_cont_struct_v2.Sat_UnlockTime_min[index] 	= errZaych;

		 server_cont_struct_v2.Sun_LockTime_hour[index] 	= errZaych;
		 server_cont_struct_v2.Sun_LockTime_min[index] 		= errZaych;
		 server_cont_struct_v2.Sun_UnlockTime_hour[index] 	= errZaych;
		 server_cont_struct_v2.Sun_UnlockTime_min[index] 	= errZaych;
	 }
}

/* Function that makes requests */
void http2_task_v2(void *args)
{
	uint8_t bda_false[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //

	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);//for synhro
	ESP_LOGI(GATTC_TAG,"Waiting for start scanning");
	while ( !isGAP_BLE_scanning )
	{
		printf(".");
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
	isGAP_BLE_scanning = false;
	while ( !isDevFounded )
	{
		printf(".");
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
	isDevFounded = false;
	vTaskDelay(6000 / portTICK_PERIOD_MS); 												//Time for scanning! important
	//new
	//refactoring
	initStructContainer_v2();					//DEinit struct server_cont_struct --> all fields = noCMD
	//initStructContainer_v2(&server_cont_struct_v2);					//DEinit struct server_cont_struct --> all fields = noCMD

	esp_ble_gap_stop_scanning();

	vTaskDelay(400 / portTICK_PERIOD_MS);
	printf("\n CONNECTING!!");
	open_ble_conn();
	UBaseType_t http_2_stack;
	char* page_getCMD = "/get-command";

	char req_str_ex[MAX_FL_NUM][MAX_REQ_LEN]; 											//2D array for storing POST requests for different lock UUIDs

	//Getting POST requests for different lock UUIDs in order to do this only at once

		for(uint8_t idx = 0; idx < MAX_FL_NUM; idx++)  // MAX_FL_NUM
		{
			formStringRequest_v5((char*)(req_str_ex[idx]), bda_connected[idx]);
		}

    while (1) {

    	xSemaphoreTake(BLEmutex, portMAX_DELAY);
    	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);//for synhro

    	for( uint8_t FL_NUM = 0; FL_NUM < MAX_FL_NUM; FL_NUM++) //
    	{

    		if ( 0 == memcmp(bda_connected[FL_NUM],bda_false,sizeof(esp_bd_addr_t)) )
    		{
    			//deb ESP_LOGI(GATTC_TAG,"Under index[%d] is false address", FL_NUM);
    			continue;										//new iteration. FLock is not connected
    		}

    		// MTU_establ

    		char* pBuff;
    		printf("\n makeHTTPrequest");
    		pBuff = makeHTTPrequest_v3((char*)(req_str_ex[FL_NUM]), page_getCMD);

    		 if (pBuff != NULL)																//HTTP request success
    		{
    			char* str_json =  highlightJSONfromSTR(pBuff, strlen(pBuff));
    			free(pBuff);								//free resources


    			if (str_json != NULL)
    				{
    					if( strlen(str_json) > TIME_LEN )
						{
							jsonParse_fill_struct_v3(str_json, FL_NUM);	//Parse and send data
						}
    				}

    			if (str_json != NULL)
    			{
    				free(str_json);
    			}

    			//refactoring
    			initStructContainer_v2();					//DEinit struct server_cont_struct --> all fields = noCMD
    		}
    		printf("\n\n\n");
    	}

    	http_2_stack = uxTaskGetStackHighWaterMark(NULL);
    	printf("\n Stack on http2_task_v2 task: %d",(int)http_2_stack);

    	xSemaphoreGive(BLEmutex);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/*	Function checks if registered SenseKitchen devices were in advertising mode (while scanning) and if true tries to establish BT connection 	*/
void open_ble_conn(void)
{
	for ( uint8_t index = 0; index < MAX_FL_NUM; index++)
	{
		//deb ESP_LOGI(GATTC_TAG, "\n MAX_FL_NUM: %d \n",index);
		if (xSemaphoreTake(connect_FL_x[index],0) == pdPASS)
		{

			{
				ESP_LOGI(GATTC_TAG, "Try to establish connection with index: %d \n",index);
				//deb ESP_LOGI(GATTC_TAG, "\n gattc_if: %d \n",index);

				//new vers esp-idf esp_ble_gattc_open(gl_profile_tab[index].gattc_if, bda_connected[index], true); //tries to open a connection to the remote device


				do
				{
					vTaskDelay(200 / portTICK_PERIOD_MS);
					ESP_LOGI(GATTC_TAG, ".");
				}
				while (!MTU_establ[index]);
			}
		}
	}
}

/* Function Parse the answer and fill the struct to send*/
int jsonParse_fill_struct_v3(char* get_str_f, uint8_t prof_idx)
{
    cJSON *json = cJSON_Parse(get_str_f);

    if ( json == NULL )
    {
   	 printf(" \n json_v2 = NULL \n");
   	 return -1;
    }

    if (!json)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf("Error before: %s\n", error_ptr);
        }
      return -1; //not success
    }

    /* Error checking */
    if ( cJSON_GetObjectItem(json,"error") != NULL )
    {
    	//error message received
    	ESP_LOGI(GATTC_TAG, "error message received_v2");
    	 cJSON_Delete(json);//deallocating json
    	 return -1; //not success
    }

        // Get all objects from json
        cJSON *jsPIN			= cJSON_GetObjectItem(json,"PIN");
        cJSON *jssetTimeM 		= cJSON_GetObjectItem(json,"setTimeM");
        cJSON *jssetTimeH 		= cJSON_GetObjectItem(json,"setTimeH");
        cJSON *jsmode 			= cJSON_GetObjectItem(json,"mode");
        cJSON *jslockName 		= cJSON_GetObjectItem(json,"lockName");
        cJSON *jssignal		= cJSON_GetObjectItem(json,"signal");
        cJSON *jssetOpen 		= cJSON_GetObjectItem(json,"setOpen");
        cJSON *jsupdateLock	= cJSON_GetObjectItem(json,"updateLock");



        //check whether objects exist
        if ( jsPIN != NULL) 			{ char *s_PIN 			= cJSON_Print(jsPIN); 			strcpy(server_cont_struct_v2.PIN, 		s_PIN);				}
        if ( jsmode != NULL) 			{ char *s_mode 			= cJSON_Print(jsmode);			strcpy(server_cont_struct_v2.mode, 		s_mode);			}
        if ( jslockName != NULL) 		{ char *s_lockName 		= cJSON_Print(jslockName);		strcpy(server_cont_struct_v2.lockName, 	s_lockName);		}
        if ( jssignal != NULL) 		{ char *s_signal 		= cJSON_Print(jssignal);		strcpy(server_cont_struct_v2.signal, 	s_signal);			}
        if ( jssetOpen != NULL) 		{ char *s_setOpen 		= cJSON_Print(jssetOpen);		strcpy(server_cont_struct_v2.setOpen,  	s_setOpen);			}

        //new  style of Time setting
        if ( jssetTimeH != NULL)
        	 {
       	 	 if ( cJSON_IsNumber(jssetTimeH) ) server_cont_struct_v2.setTimeH = jssetTimeH->valueint;
        	 }
        if ( jssetTimeM != NULL)
        	 {
       	 	 if ( cJSON_IsNumber(jssetTimeM) ) server_cont_struct_v2.setTimeM = jssetTimeM->valueint;
        	 }

        //Check flag "updateLock"
        if ( jsupdateLock != NULL)
        {
       	 if ( cJSON_IsNumber(jsupdateLock) ) server_cont_struct_v2.updateLock = jsupdateLock->valueint;
        }

        /* Server response:
         *
         *    openCloseTime: {
         *    			Monday		:[],
         *    			Tuesday		:[],
         *    			Wednesday	:[],
         *    			Thursday	:[],
         *    			Friday		:[],
         *    			Saturday	:[],
         *    			Sunday		:[]
         *    		}
         *
         *    		"Monday" : [
         *    		{
         *    			"lock_h"	 : 8,
         *    			"lock_m"	 : 30,
         *    			"unlock_h"	 : 11,
         *    			"unlock_m"	 : 30
         *    		}, ...
         *    		]
         */

        /* cJSON. Working with arrays. Getting objects if they exist */
        cJSON* jsAllDays = cJSON_GetObjectItemCaseSensitive(json, "openCloseTime");

        // Main error checking
        if ( jsAllDays != NULL)
        {
       	 cJSON *jsMonday		= cJSON_GetObjectItemCaseSensitive(jsAllDays,"Monday");
       	 cJSON *jsTuesday		= cJSON_GetObjectItemCaseSensitive(jsAllDays,"Tuesday");
       	 cJSON *jsWednesday		= cJSON_GetObjectItemCaseSensitive(jsAllDays,"Wednesday");
        	 cJSON *jsThursday		= cJSON_GetObjectItemCaseSensitive(jsAllDays,"Thursday");
        	 cJSON *jsFriday		= cJSON_GetObjectItemCaseSensitive(jsAllDays,"Friday");
        	 cJSON *jsSaturday		= cJSON_GetObjectItemCaseSensitive(jsAllDays,"Saturday");
        	 cJSON *jsSunday		= cJSON_GetObjectItemCaseSensitive(jsAllDays,"Sunday");

        	 //Monday error checking
        	 if (jsMonday != NULL)
        	 {
        		 getSchedule(jsMonday, MONDAY, server_cont_struct_v2.Mon_LockTime_hour, server_cont_struct_v2.Mon_LockTime_min,
       			 	 	 server_cont_struct_v2.Mon_UnlockTime_hour, server_cont_struct_v2.Mon_UnlockTime_min);
        	 }
        	 //Tuesday error checking
        	 if (jsTuesday != NULL)
        	 {
        		 getSchedule(jsTuesday, TUESDAY, server_cont_struct_v2.Tue_LockTime_hour, server_cont_struct_v2.Tue_LockTime_min,
        				 	 server_cont_struct_v2.Tue_UnlockTime_hour, server_cont_struct_v2.Tue_UnlockTime_min);
        	 }
        	 //Wednesday error checking
        	 if (jsWednesday != NULL)
        	 {
        		 getSchedule(jsWednesday, THURSDAY, server_cont_struct_v2.Thur_LockTime_hour, server_cont_struct_v2.Thur_LockTime_min,
        				 	 server_cont_struct_v2.Thur_UnlockTime_hour, server_cont_struct_v2.Thur_UnlockTime_min);
        	 }
        	 //Thursday error checking
        	 if (jsThursday != NULL)
        	 {
        		 getSchedule(jsThursday, THURSDAY, server_cont_struct_v2.Thur_LockTime_hour, server_cont_struct_v2.Thur_LockTime_min,
        				 	 server_cont_struct_v2.Thur_UnlockTime_hour, server_cont_struct_v2.Thur_UnlockTime_min);
        	 }
        	 //Friday error checking
        	 if (jsFriday != NULL)
        	 {
        		 getSchedule(jsFriday, FRIDAY, server_cont_struct_v2.Fri_LockTime_hour, server_cont_struct_v2.Fri_LockTime_min,
        				 	 server_cont_struct_v2.Fri_UnlockTime_hour, server_cont_struct_v2.Fri_UnlockTime_min);
        	 }
        	 //Saturday error checking
        	 if (jsSaturday != NULL)
        	 {
        		 getSchedule(jsSaturday, SATURDAY, server_cont_struct_v2.Sat_LockTime_hour, server_cont_struct_v2.Sat_LockTime_min,
        				 	 server_cont_struct_v2.Sat_UnlockTime_hour, server_cont_struct_v2.Sat_UnlockTime_min);
        	 }
        	 //Friday error checking
        	 if (jsSunday != NULL)
        	 {
        		 getSchedule(jsSunday, SUNDAY, server_cont_struct_v2.Sun_LockTime_hour, server_cont_struct_v2.Sun_LockTime_min,
        				 	 server_cont_struct_v2.Sun_UnlockTime_hour, server_cont_struct_v2.Sun_UnlockTime_min);
        	 }
        }// end af jsAllDays

        //Send all fields to FLock_task_idx
        xQueueSend(que_server_mess_v2[prof_idx], &server_cont_struct_v2,5000 / portTICK_PERIOD_MS ); ////Send all fields to FLock_task_idx

    cJSON_Delete(json);//deallocating json
	 return 1;
}

/* Function gets json object of day schedule and fills structure to send to foodlock */
uint8_t getSchedule(cJSON *jsDay, uint8_t DAY_CODE, uint8_t *lock_h, uint8_t *lock_m, uint8_t *unlock_h, uint8_t *unlock_m)
{
    if (jsDay != NULL)
    {
   	 const cJSON* el_jsDay;			// every separate element of the array
   	 uint8_t dayIdx = 1;			// Wednesday counter

   	 	// First element must be specially coded
   	 	lock_h[0] 	= DAY_CODE;
   	 	lock_m[0] 	= DAY_CODE;
   	 	unlock_h[0] = DAY_CODE;
   	 	unlock_m[0] = DAY_CODE;

   	 cJSON_ArrayForEach(el_jsDay,jsDay)	//looping through elements
   	 {
   		 	//Getting separate elements of the array Wednesday
   	        cJSON *js_lock_h 	= cJSON_GetObjectItemCaseSensitive(el_jsDay, "lock_h");
   	        cJSON *js_lock_m 	= cJSON_GetObjectItemCaseSensitive(el_jsDay, "lock_m");
   	        cJSON *js_unlock_h	= cJSON_GetObjectItemCaseSensitive(el_jsDay, "unlock_h");
   	        cJSON *js_unlock_m	= cJSON_GetObjectItemCaseSensitive(el_jsDay, "unlock_m");
   	        //Filling structure to send
   	        if (cJSON_IsNumber(js_lock_h) && cJSON_IsNumber(js_lock_m) && cJSON_IsNumber(js_unlock_h) && cJSON_IsNumber(js_unlock_m) && (dayIdx <TIME_LEN)   )
   	        {
   	        	lock_h[dayIdx]		= js_lock_h->valueint;
   	        	lock_m[dayIdx]		= js_lock_m->valueint;
   	        	unlock_h[dayIdx]	= js_unlock_h->valueint;
   	        	unlock_m[dayIdx]	= js_unlock_m->valueint;
   	        	dayIdx++;
   	        }
   	 }
    }
    else return cJSON_NULL;

   return 1;
}

