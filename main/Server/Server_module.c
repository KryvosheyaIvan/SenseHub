#include "Server_module.h"


/* Function reads servers response */
char* process_http_v2(int sockfd, char *host, char *page, char *poststr)
{
	char sendline[MAXLINE + 1];
	char* recvline_v2 = (char*)malloc((MAXLINE+1)*sizeof(char));

	ssize_t n;
	snprintf(sendline, MAXSUB,
		 "POST %s HTTP/1.0\r\n"
		 "Host: %s\r\n"
		 "Content-type: application/x-www-form-urlencoded\r\n"
		 "Content-length: %d\r\n\r\n"
		 "%s", page, host, strlen(poststr), poststr);

	write(sockfd, sendline, strlen(sendline));
	while ((n = read(sockfd, recvline_v2, MAXLINE)) > 0) {
		recvline_v2[n] = '\0';
		//deb printf("%s", recvline_v2);
	}
	return recvline_v2;
}

/* Function takes whole HTTP request and returns a pointer to only its JSON part */
char* highlightJSONfromSTR(char* pStr, short int len)
{
	short int i  = 0;				//just auxiliary variables
	short int index  = 0;

	short int startJSON = -1;		//index, starting from which, json message begins
	short int endJSON = -1;			//index, where json message ends
	short int numLen;				//number of elements of final array

	char* strJSON;

	//deb printf("\n Input str: %s", pStr);

	for( i = 0; i < len; i++)				//find start of the json string
	{
		if( pStr[i] == '{' )
			{
				//deb printf("\n startJSON founded \n");
				startJSON = i;
				break;
			}
		//deb putchar(pStr[i]);
	}


	for( i = (len-1); i > 0; i--)			//find last index of the json string
	{
		if( pStr[i] == '}' )
		{
			endJSON = i;
			//deb printf("\n endJSON founded \n");
			break;
		}
		//deb putchar(pStr[i]);
	}

	if ( (startJSON == -1) || (endJSON == -1) )			//check if there is json string
	{
		printf("\n There are no JSON string \n");
		return NULL;
	}

	numLen = endJSON-startJSON+5;				//number of elements calculation
	//create new string
	strJSON = (char*) malloc( (numLen)*sizeof(char));

	memset(strJSON, 0, numLen);							// init string with NULL-terminating symb

	for( i = startJSON; i <= endJSON; i++)				// Init new json string
	{
		strJSON[index++] = pStr[i];
	}
	printf("\n JSON string:\n%s",strJSON);
	//free(strJSON);
	return strJSON;
}

/* Function makes request to server using sockets*/
char* makeHTTPrequest_v3(char* poststr, char* page)
{
	int sockfd;
	struct sockaddr_in servaddr;

	char **pptr;
	//********** You can change. Put any values here *******
	char *hname = "boiling-island-68868.herokuapp.com";
	//char *page = "/get-command";
	//char poststr[42];
	char* pBuff = NULL;

	//sprintf(poststr,"hubID=%s&lockID=%s",pCharHub,pCharLock);
	printf("\n Request: %s",poststr);
	//old char *poststr = "hubID=30AEA4199554&lockID=F0F8F26F7748";
	//*******************************************************

	char str[50];
	struct hostent *hptr;

	if ((hptr = gethostbyname(hname)) == NULL) {
		printf(" gethostbyname error for host");
		return pBuff;
	}
	//deb printf("hostname: %s\n", hptr->h_name);
	if (hptr->h_addrtype == AF_INET
	    && (pptr = hptr->h_addr_list) != NULL) {
		printf("address: %s\n",inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
	} else {
		printf("Error call inet_ntop");
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(80);
	inet_pton(AF_INET, str, &servaddr.sin_addr);

	connect(sockfd, (SA *) & servaddr, sizeof(servaddr));
	pBuff = process_http_v2(sockfd, hname, page, poststr);

	printf("\n Response: %s",pBuff);

	close(sockfd);

	return pBuff;
}

/////////////////////////////////////////////////////////////TEST REGISTER FUNCTION ////////////////////////////////////////////





/* Function that makes requests */
void Register_task_v1(void *args)
{
	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);//for synhro

	UBaseType_t reg_stack;

	// register user
	char* request_user = "userID=sense_nick";
	char* page_user = "/user/register";

	//register hub
	char* request_hub = "userID=sense_nick&hubID=30AEA4199554";
	char* page_hub = "/hub/register";

	//register lock
	char* request_lock = "hubID=30AEA4199554&lockID=B091226945A2";
	char* page_lock = "/lock/register";

	//register lock_2
	char* request_lock_2 = "hubID=30AEA4199554&lockID=F0F8F26F7748";

	char* pBuff;

    while (1) {

    	xSemaphoreTake(BLEmutex, portMAX_DELAY);
    	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);//for synhro



    	printf("\n make User register request");
    	pBuff = makeHTTPrequest_v3(request_user,page_user);

    	if (pBuff != NULL)																//HTTP request success
    	{
    		// printf("\n My personal response: ");
    		// printf("\n%s", pBuff);

    		// char* str_json =  highlightJSONfromSTR(pBuff, strlen(pBuff));
    		free(pBuff);
    	}

    	printf("\n\n\n");
    	vTaskDelay(2000 / portTICK_PERIOD_MS);

    	printf("\n make Hub register request");
    	pBuff = makeHTTPrequest_v3(request_hub,page_hub);
    	if (pBuff != NULL)																//HTTP request success
    	{
    		// printf("\n My personal response: ");
    		// printf("\n%s", pBuff);

    		// char* str_json =  highlightJSONfromSTR(pBuff, strlen(pBuff));
    		free(pBuff);
    	}
    	printf("\n\n\n");

    	printf("\n\n\n");
    	vTaskDelay(2000 / portTICK_PERIOD_MS);

    	printf("\n make Lock register request");
    	pBuff = makeHTTPrequest_v3(request_lock,page_lock);
    	if (pBuff != NULL)																//HTTP request success
    	{
    		// printf("\n My personal response: ");
    		// printf("\n%s", pBuff);

    		// char* str_json =  highlightJSONfromSTR(pBuff, strlen(pBuff));
    		free(pBuff);
    	}
    	printf("\n\n\n");

    	printf("\n make Lock_2 register request");
    	pBuff = makeHTTPrequest_v3(request_lock_2,page_lock);
    	if (pBuff != NULL)																//HTTP request success
    	{
    		// printf("\n My personal response: ");
    		// printf("\n%s", pBuff);

    		// char* str_json =  highlightJSONfromSTR(pBuff, strlen(pBuff));
    		free(pBuff);
    	}
    	printf("\n\n\n");


    	reg_stack = uxTaskGetStackHighWaterMark(NULL);
    	printf("\n Stack on Register task: %d",(int)reg_stack);

    	xSemaphoreGive(BLEmutex);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

}

