/*
 * esp8266.c
 *
 *  Created on: 2023. 2. 24.
 *      Author: asong
 */

#include "esp8266.h"


char *HEADER = "HTTP/1.1 200 OK\r\n\r\n";
char *HELLO_HTML = "<!doctype html><html><head><title>ESP8266</title></head><body><h1>esp hello world</h1></body></html>";

char *NOT_found_404 = "HTTP/1.1 404 Not Found\r\n";


static ESP_CONFIG esp_config;


/**
 * USART3_IRQHandler안에 있어야 하는 함수
 * uart가 idle 상태이지 확인한다.
 */
void uart_it() {
	if(LL_USART_IsActiveFlag_IDLE(esp_config.uart)) {
		esp_config.uart_idle = true;
		LL_USART_ClearFlag_IDLE(esp_config.uart);
	}
}



int esp8266_init(USART_TypeDef *uart, DMA_TypeDef *DMA, char *ssid, char* pass) {
	esp_config.uart = uart;
	esp_config.SSID = ssid;
	esp_config.DMA = DMA;
	esp_config.password = pass;
	esp_config.uart_idle = false;
	esp_config.port = 80;

	// configure dma source & destination
	LL_DMA_ConfigAddresses(esp_config.DMA, LL_DMA_CHANNEL_3, LL_USART_DMA_GetRegAddr(esp_config.uart), (uint32_t)esp_config.DMA_buf, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
	// configure data length
	LL_DMA_SetDataLength(esp_config.DMA, LL_DMA_CHANNEL_3, DMA_BUF_SIZE);

	// enable interrupts
	LL_USART_EnableIT_IDLE(esp_config.uart);

	// enable DMA stream
	LL_USART_EnableDMAReq_RX(esp_config.uart);
	LL_DMA_EnableChannel(esp_config.DMA, LL_DMA_CHANNEL_3);


	return 0;
}

void esp_send(char *pdata) {

	printf("tx: %s\r\n==========\r\n", pdata);

	while (*pdata) {
		while(!LL_USART_IsActiveFlag_TXE(esp_config.uart));
		LL_USART_TransmitData8(esp_config.uart, *pdata);
		pdata++;
	}
}

int response(char *req, int m_time) {
	uint32_t tickstart = SysTick->CTRL;

	while (!esp_config.uart_idle) {
		//시간 초과시 0을 리턴
		if ((SysTick->CTRL - tickstart) > m_time)
			return 0;
	}

	esp_config.uart_idle = false;

	//disble DMA
	LL_DMA_DisableChannel(esp_config.DMA, LL_DMA_CHANNEL_3);

	//total received size = total buffer size - left size for DMA
	uint32_t buff_len = DMA_BUF_SIZE - LL_DMA_GetDataLength(esp_config.DMA, LL_DMA_CHANNEL_3);
	memcpy(req, esp_config.DMA_buf, buff_len+1);
	req[buff_len] = 0;

	//clear DMA flags
	esp_config.DMA->IFCR &= 0b1111 << ((LL_DMA_CHANNEL_3 -1) * 4);

	// configure data length
	LL_DMA_SetDataLength(esp_config.DMA, LL_DMA_CHANNEL_3, DMA_BUF_SIZE);

	// enable DMA stream
	LL_DMA_EnableChannel(esp_config.DMA, LL_DMA_CHANNEL_3);


	printf("rx len: %d\r\n%s\r\n==========\r\n", buff_len, req);

	return buff_len;
}

int wait_for(char *str, int m_time) {

	uint32_t tickstart = HAL_GetTick();
	uint32_t wait = m_time;

	char *str_ptr;
	char buf[512];

	while ((HAL_GetTick() - tickstart) < wait) {
		memset(buf, 0, DMA_BUF_SIZE);
		response(buf, m_time);
		str_ptr = strstr(buf, str);
		if (str_ptr != NULL)
			return 1;
	}
	return 0;
}

int server_init() {

	char CMD[40];

	SERVER_RESET:
	LL_mDelay(1000);

	//AT reset
	printf("1-----------\r\n");
	esp_send("AT+RST\r\n");
	if(!wait_for("OK", 1000))
		goto SERVER_RESET;

	//Wi-Fi mode : Station mode
	printf("2-----------\r\n");
	esp_send("AT+CWMODE=1\r\n");
	if(!wait_for("OK", 1000))
		goto SERVER_RESET;

	//Connect to an AP
	printf("3-----------\r\n");
	sprintf(CMD, "AT+CWJAP=\"%s\",\"%s\"\r\n", esp_config.SSID, esp_config.password);
	esp_send(CMD);
	if(!wait_for("ready", 1000))
		goto SERVER_RESET;

	// get IP info
	printf("4-----------\r\n");
	sprintf(CMD, "AT+CIFSR\r\n");
	esp_send(CMD);
	if(!wait_for("OK", 1000))
		goto SERVER_RESET;

	//Query the connection type. : Enable multiple connections
	printf("5-----------\r\n");
	esp_send("AT+CIPMUX=1\r\n");
	if(!wait_for("OK", 1000))
		goto SERVER_RESET;

	//create a server, port
	printf("6-----------\r\n");
	sprintf(CMD, "AT+CIPSERVER=1,%d\r\n", esp_config.port);
	esp_send(CMD);
	if(!wait_for("OK", 1000))
		goto SERVER_RESET;


	return 0;
}

int get_IPD(char *pdata) {
	uint8_t *IPD_ptr;
	IPD_ptr = strstr(pdata, "+IPD");

	if (IPD_ptr == NULL)
		return -1;

	return atoi(IPD_ptr+5);
}

HTTP_METHOD get_method(char *pdata) {
	char *ptr;
	ptr = strstr(pdata, ":");
	ptr++;
	if (!strncmp(ptr, "GET", 3)) return HTTP_GET;
	else if (!strncmp(ptr, "POST", 4)) return HTTP_POST;
	else if (!strncmp(ptr, "PUT", 3)) return HTTP_PUT;
	else if (!strncmp(ptr, "DELETE", 6)) return HTTP_DELETE;
	else return HTTP_ERROR;
}


int get_path(char *pdata, char *path) {
	uint8_t *path_ptr;
	int i;
	path_ptr = strstr(pdata, "/");

	if (path_ptr == NULL)
		return -1;

	for (i=0; *(path_ptr+i) != ' '; i++) {
		path[i] = *(path_ptr+i);
	}
	path[i] = 0;

	return 1;
}

int Server_Handle(uint8_t *path, uint32_t IPD) {

	if (!strcmp(path, "/")) {
		Server_Send(HELLO_HTML, IPD);
	}
}

int Server_Send(uint8_t *pdata, int IPD) {

	uint8_t CMD[40];
	int header_len = strlen(HEADER);
	int len = strlen(pdata);

	//Send Data
	sprintf (CMD, "AT+CIPSEND=%d,%d\r\n", IPD, header_len + len);
	esp_send(CMD);
	wait_for("OK", 100);

	esp_send(HEADER);
	esp_send(pdata);
	wait_for("SEND OK", 100);

	sprintf (CMD, "AT+CIPCLOSE=%d\r\n", IPD);
	esp_send(CMD);
	wait_for("OK", 100);

	return 1;
}

void NOT_found(int IPD) {
	Server_Send(NOT_found_404, IPD);
}


