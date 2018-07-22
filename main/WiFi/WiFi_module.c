#include "WiFi_module.h"

/********************* Global WiFi module variables ****************************/

/* Types of blinking indicating current mode of esp32 */
#define SLOW_BLINKING      1000
#define SPRINT_BLINKING    100
#define JUST_LIGHTING      10

/* Pin - LED */
const int DIODE_PIN = 5;

/* AP or connected to router mode */
char HUB_STATUS_1[32] = "SSID_PASS";
char HUB_STATUS_2[64] = "noSSID_noPASS";
char WIFI_SSID_2[32] = "Valera_ESP32_AP";
char WIFI_PASS_2[64] = "";

/* Variables holding ssid and password got from user*/
char ssid[32] = "";
char pass[64] = "";
char buffer[32];

/* HTML header */
const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
/* Simple HTML form */
const static char http_index_hml[] = "<html><body><form method=get><label> </label><br><label> </label><br><label>SSID</label><br><input  type='text' name='ssid' maxlength='30' size='15'><br><label>Password</label><br><input  type='password' name='password' maxlength='30' size='15'><br><br><input  type='submit' value='connect' > </form></body></html>";


bool Sosket_Status = false;
bool buttonReset = false;					//button "Reset" state holder
int blinking_delay = SLOW_BLINKING;

int attempts_Number = 0;					// counter of connecting to station attempts


EventGroupHandle_t wifi_event_group;		// Event groups concerning Wi-Fi module
const int CONNECTED_BIT = BIT0;				// Connect event indicator

/* Initial SSID and password */
#define EXAMPLE_WIFI_SSID "ValVel_ESP32_AP"
#define EXAMPLE_WIFI_PASS ""

/* Initial SSID and password in STA mode*/
#define EXAMPLE_WIFI_SSID_2 "SenseSystems"
#define EXAMPLE_WIFI_PASS_2 "sense2016"

/****************************************FUNCTIONS*******************************************************/

/* Init whole WiFi module*/
void WiFi_init(void)
{
	nvs_flash_init();
	print_read_hub_status(buffer);

    if(Sosket_Status==false)
    	{
    		initialise_wifi();
    	}
    if(Sosket_Status==true)
        {
        	initialise_wifi_2();
        }

    /* Service tasks creation */
    xTaskCreate(&http_server, "http_server",2048 , NULL, 5, NULL);
    xTaskCreate(&diode_blinking, "diode_blinking", configMINIMAL_STACK_SIZE*2, NULL, 5, NULL);
    xTaskCreate(&push_button, "push_button", configMINIMAL_STACK_SIZE*2, NULL, 5, NULL); //2048
}

/* Init WiFi in AP mode*/
void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
			.channel=0,
			.authmode=WIFI_AUTH_OPEN,
			.ssid_hidden=0,
			.max_connection=4,
			.beacon_interval=100
        },
    };
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_AP, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

/* Init WiFi in station mode*/
void initialise_wifi_2(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

    wifi_config_t wifi_config = {
            .sta = {
                .ssid = EXAMPLE_WIFI_SSID_2,
                .password = EXAMPLE_WIFI_PASS_2,
            },
        };
    strncpy((char*) wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid)); //!!!
    strncpy((char*) wifi_config.ap.password, pass, sizeof(wifi_config.ap.password));//!!!
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

/* WiFi callback function */
esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
    	blinking_delay = SLOW_BLINKING;
    	printf("3-------3\n");
    	//if(Sosket_Status == true)
    		ESP_ERROR_CHECK(esp_wifi_connect());
        break;
    case SYSTEM_EVENT_AP_START:
    	Sosket_Status = false;
    	blinking_delay = SPRINT_BLINKING ;
    	break;
    case SYSTEM_EVENT_STA_GOT_IP:
    	if(Sosket_Status == false)
    	{
    		Sosket_Status = true;
    		print_save_hub_SSID_PASS(ssid, pass);
    		print_save_hub_status( HUB_STATUS_1);
    	}
    	blinking_delay = JUST_LIGHTING ;
    	printf("\n xEventGroupSetBits \n");
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
    	blinking_delay = JUST_LIGHTING ;
    	break;
    case SYSTEM_EVENT_STA_DISCONNECTED:

    	blinking_delay = SLOW_BLINKING;
    	if(buttonReset == true)
    		{buttonReset = false;
    		break;}
    	if(Sosket_Status == true)// SSID and Password already saved
    		{
    		xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    		printf("DISCONNECTED, reason : %d\n", event->event_info.disconnected.reason);
    		ESP_ERROR_CHECK(esp_wifi_connect());
    		}
    	if(Sosket_Status == false)
    		{
    		esp_wifi_connect();
    		printf("2-------2\n");
    		if(attempts_Number<15)
    		{
    			attempts_Number ++;
    	    }
    		else
    			{
    			attempts_Number = 0;
    			wifi_reset_to_point_mode();
    			}
    		xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    		}
        break;

    default:
        break;
    }
    return ESP_OK;
}

/* Function saves current SSID and password to flash storage*/
esp_err_t print_save_hub_SSID_PASS(char *SSID, char *PASS)
{
			nvs_handle wifi_handle;
		    esp_err_t err;

			// Open NVS
		    //deb printf("\n Opening NVS...\n");
		    err = nvs_open("storage", NVS_READWRITE, &wifi_handle);
		    if (err != ESP_OK) return err;


			// Read SSID written on NVS
			size_t required_size_SSID = 0;  // value will default to 0, if not set yet in NVS
			size_t required_size_PASS = 0;

			err = nvs_get_str(wifi_handle, "SSID", NULL, &required_size_SSID); // проверяем записано ли туда чтото
			err = nvs_get_str(wifi_handle, "PASS", NULL, &required_size_PASS);
			//deb printf("Obtain required memory space to store complex data structure (hubStatus) being read from NVS...\n");

			if (required_size_SSID  == 0) {  //ssid ще не записаний, записуэмо

				char ssid_2[32] = "---";
				err = nvs_set_str(wifi_handle, "SSID", ssid_2); //
				printf("\n Writing ssid_2 to  NVS...\n");
				if (err != ESP_OK) return err;
				err = nvs_commit(wifi_handle);
				//deb printf("\n Committing to  NVS...\n");
				if (err != ESP_OK) return err;
			}

			if (required_size_PASS  == 0) {  //ssid ще не записаний, записуэмо

				char pass_2[64] = "---";
				err = nvs_set_str(wifi_handle, "PASS", pass_2); //
				printf("\n Writing pass_2 to  NVS...\n");
				if (err != ESP_OK) return err;
				err = nvs_commit(wifi_handle);
			    printf("\n Committing to  NVS...\n");
				if (err != ESP_OK) return err;
						}

			if (required_size_SSID  > 0) {

			    err = nvs_set_str(wifi_handle, "SSID", SSID); //
			    //deb printf("\n Reading SSID from  NVS...\n");
				err = nvs_commit(wifi_handle);
				//deb printf("\n Committing to  NVS...\n");
				if (err != ESP_OK) return err;
			}

			if (required_size_PASS  > 0) {

				err = nvs_set_str(wifi_handle, "PASS", PASS); //
				//deb printf("\n Reading PASS from  NVS...\n");
				err = nvs_commit(wifi_handle);
				//deb printf("\n Committing to  NVS...\n");
				if (err != ESP_OK) return err;
			}

			 nvs_close(wifi_handle);
			 return ESP_OK;
}

/* Reset WiFi configuration to point mode*/
void wifi_reset_to_point_mode(void)
{
	 wifi_config_t wifi_config = {
	        .ap = {
	            .ssid = EXAMPLE_WIFI_SSID,
	            .password = EXAMPLE_WIFI_PASS,
				.channel=0,
				.authmode=WIFI_AUTH_OPEN,
				.ssid_hidden=0,
				.max_connection=4,
				.beacon_interval=100
	        },
	    };
	    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
	    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_AP, &wifi_config) );
	    ESP_ERROR_CHECK( esp_wifi_start() );
}

/* Save hub status*/
esp_err_t print_save_hub_status(char *hubStatus)
{
		nvs_handle wifi_handle;
	    esp_err_t err;

		// Open NVS
	    //deb printf("\n Opening NVS...\n");
	    err = nvs_open("storage", NVS_READWRITE, &wifi_handle);
	    if (err != ESP_OK) return err;

		// Read SSID written on NVS
		size_t required_size = 0;  // value will default to 0, if not set yet in NVS
		// obtain required memory space to store blob being read from NVS
		err = nvs_get_str(wifi_handle, "hubStatus", NULL, &required_size); // check if there was written something
		//deb printf("Obtain required memory space to store complex data structure (hubStatus) being read from NVS...\n");

		if (required_size == 0) {  //ssid ще не записаний, записуэмо
			printf("\n No  hubStatus been saved yet!");
			printf("\n Saving  hubStatus...");
			char HUB_STATUS_2[32] = "noSSID_noPASS";

			err = nvs_set_str(wifi_handle, "hubStatus", HUB_STATUS_2); //

			//deb printf("\n Writing hubStatus to  NVS...\n");

			if (err != ESP_OK) return err;
			// Commit. Mandatory!
			err = nvs_commit(wifi_handle);

			//deb printf("\n Committing to  NVS...\n");

			if (err != ESP_OK) return err;

		}

			else { //if value was written previously then display it on the screen

				            err = nvs_set_str(wifi_handle, "hubStatus", hubStatus); //
							printf("\n Writing hubStatus to  NVS...\n");
							if (err != ESP_OK) return err;
							// Commit. Mandatory!
							err = nvs_commit(wifi_handle);
							//deb printf("\n Committing to  NVS...\n");
							if (err != ESP_OK) return err;
				 }

		 nvs_close(wifi_handle);
		 return ESP_OK;
}

/* Function prints out WiFi hub status*/
esp_err_t print_read_hub_status(char *hubStatus)
{
		nvs_handle wifi_handle;
	    esp_err_t err;

		// Open NVS
	    //deb printf("\n Opening NVS...\n");
	    err = nvs_open("storage", NVS_READWRITE, &wifi_handle);
	    if (err != ESP_OK) return err;

		// Read SSID written on NVS
		size_t required_size = 0;  // value will default to 0, if not set yet in NVS
		// obtain required memory space to store blob being read from NVS
		err = nvs_get_str(wifi_handle, "hubStatus", NULL, &required_size);
		//deb printf("Obtain required memory space to store complex data structure (hubStatus) being read from NVS...\n");

		if (required_size == 0) {  //ssid wasnt yet written, do it
			printf("\n No  hubStatus been saved yet!");
			printf("\n Saving  hubStatus...");


			char HUB_STATUS_2[32] = "noSSID_noPASS";

			err = nvs_set_str(wifi_handle, "hubStatus", HUB_STATUS_2); //

			printf("\n Writing hubStatus to  NVS...\n");

			if (err != ESP_OK) return err;
			// Commit. Mandatory!
			err = nvs_commit(wifi_handle);

			//deb printf("\n Committing to  NVS...\n");

			if (err != ESP_OK) return err;

		}
		else { //if value was written before
							err = nvs_get_str(wifi_handle, "hubStatus", hubStatus, &required_size);
							//deb printf(" Getting existing hubStatus...\n");
							if (err != ESP_OK) return err;
							printf(" \n Current hubStatus is : %s \n", hubStatus);

			 }
	nvs_close(wifi_handle);
	return ESP_OK;
}

/* Serve function that handles incoming requests */
void http_server_netconn_serve(struct netconn *conn)
{
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  esp_err_t err;
  uint8_t pp = 0;
  /* Read the data from the port, blocking if nothing yet there.
   We assume the request (the part we care about) is in one netbuf */
  err = netconn_recv(conn, &inbuf);

  if (err == ERR_OK) {
    netbuf_data(inbuf, (void**)&buf, &buflen);

    /*--------------MMMMMMMMMMMMMMMMMMMMMMM---------------*/
    printf(buf);

    char ssid_mask[]="ssid=";
    char pass_mask[]="password=";
    int pointer = -1;

    for(uint8_t i=0;i<40;i++)
    {
    	uint8_t man = 0;
    	for(uint8_t j=0;j<5;j++)
    		{
    		if(buf[i+j]==ssid_mask[j])
    			man++;
    		}
    	if(man==5)
    	{	pointer = i;
    	break;}

    	else
    		pointer = -1;
    }

    uint8_t i = 0;
    if (pointer>=0)
    {

    	do
    	{
    	if(buf[pointer+i+5]=='&')
    			break;

    	ssid[i] = buf[pointer+i+5];
    	i++;
    	} while (buf[pointer+i+5]!='&');

    	if (i>5) pp = 1;
    }
    ssid[i]='\0';

    pointer = -1;

    for(uint8_t i=0;i<40;i++)
        {
        	int man = 0;
        	for(uint8_t j=0;j<9;j++)
        		{
        		if(buf[i+j]==pass_mask[j])
        			man++;
        		}
        	if(man==9)
        	{	pointer = i;
        	break;}

        	else
        		pointer = -1;

        }
        i = 0;
        if (pointer>=0)
        {
        	do
        	{
        		if(buf[pointer+i+9]==' ')
        		break;

        		pass[i] = buf[pointer+i+9];
        	i++;
        	} while (buf[pointer+i+9]!=' ');
        }
     pass[i]='\0';

          printf(ssid);
          printf("\n");
          printf(pass);
          printf("\n");

    printf(WIFI_SSID_2);
    printf("\n");
    printf(WIFI_PASS_2);
    printf("\n");

    /*--------------MMMMMMMMMMMMMMMMMMMMMMM---------------*/

    /* Is this an HTTP GET command? (only check the first 5 chars, since
    there are other formats for GET, and we're keeping it very simple )*/
    if (buflen>=5 &&
        buf[0]=='G' &&
        buf[1]=='E' &&
        buf[2]=='T' &&
        buf[3]==' ' &&
        buf[4]=='/' ) {
          printf("%c\n", buf[5]);
      /* Send the HTML header
             * subtract 1 from the size, since we dont send the \0 in the string
             * NETCONN_NOCOPY: our data is const static, so no need to copy it
       */
       gpio_pad_select_gpio(DIODE_PIN);
       /* Set the GPIO as a push/pull output */
       gpio_set_direction(DIODE_PIN, GPIO_MODE_OUTPUT);
       if(buf[5]=='h'){
         gpio_set_level(DIODE_PIN,1);

       }
       if(buf[5]=='l'){
         gpio_set_level(DIODE_PIN,0);

       }
      netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY);

      /* Send our HTML page */
      netconn_write(conn, http_index_hml, sizeof(http_index_hml)-1, NETCONN_NOCOPY);
    }

  }
  /* Close the connection (server closes in HTTP) */
  netconn_close(conn);

  /* Delete the buffer (netconn_recv gives us ownership,
   so we have to make sure to deallocate the buffer) */
  netbuf_delete(inbuf);

  if (pp==1)
      {
	  attempts_Number = 0;
	  wifi_reset_to_client_mode(ssid, pass);
	 // wifi_reset_to_PointClient_mode(ssid, pass);
      }
}

/* http server service task*/
void http_server(void *pvParameters)
{
  struct netconn *conn, *newconn;
  esp_err_t err;
  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, NULL, 80);
  netconn_listen(conn);
  while(1)
{
  do {
     err = netconn_accept(conn, &newconn);
     if (err == ERR_OK) {
    	 if(Sosket_Status==false)
    	     	{
    		 http_server_netconn_serve(newconn);

    	     	}
    	 if(Sosket_Status==true)
    	     	     	{
    		// http_server_netconn_serve_2(newconn);
    	     	     	}
       netconn_delete(newconn);
     }
   } while(err == ERR_OK);
   netconn_close(conn);
   netconn_delete(conn);
}
}

/* Reset WiFi to client mode in case of unsuccessful connections*/
void wifi_reset_to_client_mode(char *ssid, char *pass)
{

	 //Sosket_Status=true;
	 wifi_config_t wifi_config = {
	            .sta = {
	                .ssid = EXAMPLE_WIFI_SSID_2,
	                .password = EXAMPLE_WIFI_PASS_2,
	            },
	        };

	    strncpy((char*) wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid)); //!!!
	    strncpy((char*) wifi_config.ap.password, pass, sizeof(wifi_config.ap.password));//!!!
	    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
	    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
	    ESP_ERROR_CHECK( esp_wifi_start() );

}

/* Task controlling indication LED */
void diode_blinking(void *pvParameters)
{
	gpio_pad_select_gpio(17);
	gpio_set_direction(17, GPIO_MODE_OUTPUT);
	gpio_set_level(17,1);

	while(1)
	{
		gpio_set_level(17,1);
		vTaskDelay(blinking_delay / portTICK_PERIOD_MS);
		gpio_set_level(17,0);
		vTaskDelay(blinking_delay / portTICK_PERIOD_MS);

	}
}

/* Task checking control button status */
void push_button(void *pvParameters)
{
	gpio_pad_select_gpio(2);
	gpio_pullup_dis(2);
	gpio_set_direction(2, GPIO_MODE_INPUT );
	uint8_t pin_condition = 0;

	while(1)
	{
		pin_condition = gpio_get_level(2);
		if(pin_condition)
		{
			for(uint8_t i=0; i<10; i++)
				{
					pin_condition = gpio_get_level(2);
					vTaskDelay(100 / portTICK_PERIOD_MS);//vanya
				}
			pin_condition = gpio_get_level(2);

			if(pin_condition)
			{
				attempts_Number = 0;
				buttonReset = true;
				print_save_hub_status( HUB_STATUS_2);
				wifi_reset_to_point_mode();
			}
		}
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}
