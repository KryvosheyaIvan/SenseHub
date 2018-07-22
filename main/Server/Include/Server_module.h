#ifndef _SERVER_MODULE_H_
#define _SERVER_MODULE_H_

#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"


#define SA      struct sockaddr
#define MAXLINE 4096
#define MAXSUB  200
#define LISTENQ         1024
extern int h_errno;

// FreeRTOS event group to signal when we are connected & ready to make a request
extern EventGroupHandle_t wifi_event_group;
extern const int CONNECTED_BIT;
/* mutex /shared resource - BLE and Wi-Fi */
extern xSemaphoreHandle BLEmutex;


/* Function reads servers response */
char* process_http_v2(int sockfd, char *host, char *page, char *poststr);

/* Function makes request to server using sockets*/
char* makeHTTPrequest_v3(char* poststr, char* page);

/* Function takes whole HTTP request and returns a pointer to only its JSON part */
char* highlightJSONfromSTR(char* pStr, short int len);

/* Function that makes requests */
void Register_task_v1(void *args);

#endif
