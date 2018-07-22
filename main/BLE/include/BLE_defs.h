#ifndef BLE_DEFS_H_
#define BLE_DEFS_H_

/*	TAGs	*/
#define GATTC_TAG 	"GATTC_DEMO"
#define NVS_BLE_TAG "NVS_BLE"
#define LOCK_x_TAG 	"F_LOCK_x"

/* FoodLock profiles */
#define fLOCK_ID_A 0
#define fLOCK_ID_B 1
#define fLOCK_ID_C 2
#define fLOCK_ID_D 3
#define fLOCK_ID_E 4
#define fLOCK_ID_F 5
#define fLOCK_ID_G 6

#define INVALID_HANDLE   0

/*	Daytime codes	*/
#define MONDAY		 1
#define TUESDAY		 2
#define WEDNESDAY	 3
#define THURSDAY	 4
#define FRIDAY		 5
#define SATURDAY	 6
#define SUNDAY		 7

/* Maximum string length definitions */
#define NULLplace 1													//NULL placeholder to print strings

#define MAX_FL_MESS_LEN 	8										// max length of message to write to FoodLock characteristic
#define LOCK_COND_LEN		6										// max LockCondition length in 	FL_x_read_str
#define MAX_BATT_LEN		5										// max Battery length in FL_x_read_str
#define MAX_REQ_LEN	80												// max len of my request
#define TIME_LEN		5
#define MAX_SERVER_LEN 37											//len = 36 + NULL


#define MAX_FL_NUM    7  //max allowable number of connected device

/* Error codes for string and for int variables */
#define noCMD "noCMD"		//if in FL_x_container no new data
#define errZaych 99			//zaychenko error code


/* Array where BLE addresses of already connected foodlocks are stored */
esp_bd_addr_t bda_connected[MAX_FL_NUM];

// Struct that contains all necessary CMDs that will be sent to FoodLock characteristics
struct FL_x_container_v2 {
	char setOpen[MAX_FL_MESS_LEN];			// 	close or open
	char PIN[MAX_FL_MESS_LEN];				//  1111 or xxxx...
	char mode[MAX_FL_MESS_LEN];				//  Fitnes, Family or Biohak
	char lockName[MAX_FL_MESS_LEN];			//  name you want
	char signal[MAX_FL_MESS_LEN];			//  ?
	uint8_t updateLock;						// if updateLock=1 --> then program reads all FL chars

	uint8_t setTimeH;			//	0..24
	uint8_t setTimeM;			//	0..60

	uint8_t Mon_LockTime_hour[TIME_LEN];			//  Monday Lock Time
	uint8_t Mon_LockTime_min[TIME_LEN];				//
	uint8_t Mon_UnlockTime_hour[TIME_LEN];			//  Unlock Time
	uint8_t Mon_UnlockTime_min[TIME_LEN];			//

	uint8_t Tue_LockTime_hour[TIME_LEN];			//  Tuesday Lock Time
	uint8_t Tue_LockTime_min[TIME_LEN];				//
	uint8_t Tue_UnlockTime_hour[TIME_LEN];			//
	uint8_t Tue_UnlockTime_min[TIME_LEN];			//

	uint8_t Wedn_LockTime_hour[TIME_LEN];			//  Wednesday Lock Time
	uint8_t Wedn_LockTime_min[TIME_LEN];			//
	uint8_t Wedn_UnlockTime_hour[TIME_LEN];			//  Unlock Time
	uint8_t Wedn_UnlockTime_min[TIME_LEN];			//

	uint8_t Thur_LockTime_hour[TIME_LEN];			//  Thursday Lock Time
	uint8_t Thur_LockTime_min[TIME_LEN];			//
	uint8_t Thur_UnlockTime_hour[TIME_LEN];			//  Unlock Time
	uint8_t Thur_UnlockTime_min[TIME_LEN];			//

	uint8_t Fri_LockTime_hour[TIME_LEN];			//  Friday Lock Time
	uint8_t Fri_LockTime_min[TIME_LEN];				//
	uint8_t Fri_UnlockTime_hour[TIME_LEN];			//  Unlock Time
	uint8_t Fri_UnlockTime_min[TIME_LEN];			//

	uint8_t Sat_LockTime_hour[TIME_LEN];			//  Saturday Lock Time
	uint8_t Sat_LockTime_min[TIME_LEN];				//
	uint8_t Sat_UnlockTime_hour[TIME_LEN];			//  Unlock Time
	uint8_t Sat_UnlockTime_min[TIME_LEN];			//

	uint8_t Sun_LockTime_hour[TIME_LEN];			//  Sunday Lock Time
	uint8_t Sun_LockTime_min[TIME_LEN];				//
	uint8_t Sun_UnlockTime_hour[TIME_LEN];			//  Unlock Time
	uint8_t Sun_UnlockTime_min[TIME_LEN];			//
};

/* struct of separate FAP BLE profile */
struct gattc_profile_inst {
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_start_handle;
    uint16_t service_end_handle;
    uint16_t char_handle;
    esp_bd_addr_t remote_bda;
};



/* Struct that used for transferring characteristics data from event_x_handler to  necessary FL_x task*/
struct singleMessage {
	uint16_t ch_handle;
	uint8_t message[12];
	uint16_t length;
};

/* Struct that used to know that data was sent to FoodLock*/
struct writeMessage {
	uint16_t ch_handle;
};


/* Custom Service FoodLock uuid`s */
#define open_close_backw 	{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0xa3, 0x78, 0x3f, 0x0a,} , //+
#define foodlock_time_backw { 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0x59, 0xd8, 0x3f, 0x0a,} , //+
#define state_backw 		{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0x78, 0x65, 0x3f, 0x0a,} , //+
#define init_backw 			{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0x29, 0xc1, 0x3f, 0x0a,} , //+
#define week_backw 			{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0x00, 0x93, 0x3f, 0x0a,} , //

/* custom characteristics uuid`s */
#define open_now_backw 	{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0xe8, 0xa0, 0x3f, 0x0a,} ,
#define close_now_backw { 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0x4e, 0x2e, 0x3f, 0x0a,} ,
#define battery_backw 	{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0xf7, 0xda, 0x3f, 0x0a,} ,
#define reserved_1_backw 	{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0x2e, 0x88, 0x3f, 0x0a,} ,//reed switch state

#define initialize_backw 	{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0x6a, 0xdd, 0x3f, 0x0a,} ,
#define curr_time_backw 	{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0x7d, 0xa2, 0x3f, 0x0a,} ,  //+
#define lock_cond_back 		{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0xee, 0x70, 0x3f, 0x0a,} ,//+

#define pin_backw 				{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0x97, 0x9a, 0x3f, 0x0a,} ,//hz
#define foodLock_mode_backw 	{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0xf0, 0x10, 0x3f, 0x0a,} ,//hz
#define set_time_backw 			{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0xba, 0x93, 0x3f, 0x0a,} ,//+
#define lock_time_backw 		{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0xb5, 0x8c, 0x3f, 0x0a,} ,//hz
#define unlock_time_backw 		{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0xfa, 0xa3, 0x3f, 0x0a,} ,//hz

#define SunLockHour_backw 		{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0x81, 0x63, 0x3f, 0x0a,} ,//hz
#define SunLockMin_backw 		{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0xf0, 0xc9, 0x3f, 0x0a,} ,//hz
#define SunUnlockHour_backw		{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0x4f, 0x3d, 0x3f, 0x0a,} ,//hz
#define SunUnlockMin_backw		{ 0x59, 0x44, 0xB2, 0x12, 0x8d, 0xe3, 0xe1, 0xa1, 0xfc, 0x42, 0xbd, 0x1d,  0x40, 0x1a, 0x3f, 0x0a,} ,//hz



#endif
