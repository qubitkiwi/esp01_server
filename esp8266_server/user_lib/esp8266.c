/*
 * esp8266.c
 *
 *  Created on: 2023. 2. 24.
 *      Author: asong
 */

#include "esp8266.h"
#include "html.h"
#include "led.h"



static ESP_CONFIG esp_config;


/**
 * USART3_IRQHandler안에 있어야 하는 함수
 * uart가 idle 상태인지 확인한다.
 */
void uart_it() {
	if(LL_USART_IsActiveFlag_IDLE(esp_config.uart)) {
		esp_config.uart_idle = true;
		LL_USART_ClearFlag_IDLE(esp_config.uart);
	}
}


int esp8266_init(USART_TypeDef *uart, DMA_TypeDef *DMA, uint32_t Channel) {

	esp_config.uart = uart;
	esp_config.DMA = DMA;
	esp_config.DMA_Channel = Channel;

	esp_config.uart_idle = false;


	// configure dma source & destination
	LL_DMA_ConfigAddresses(esp_config.DMA, esp_config.DMA_Channel, LL_USART_DMA_GetRegAddr(esp_config.uart), (uint32_t)esp_config.DMA_buf, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
	// configure data length
	LL_DMA_SetDataLength(esp_config.DMA, esp_config.DMA_Channel, DMA_BUF_SIZE);

	// enable interrupts
	LL_USART_EnableIT_IDLE(esp_config.uart);

	// enable DMA stream
	LL_USART_EnableDMAReq_RX(esp_config.uart);
	LL_DMA_EnableChannel(esp_config.DMA, esp_config.DMA_Channel);


	return 1;
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

	memset(req, 0, DMA_BUF_SIZE);

	uint32_t tickstart = HAL_GetTick();

	while (!esp_config.uart_idle) {
		//시간 초과시 0을 리턴
		if ((HAL_GetTick() - tickstart) > m_time)
			return 0;
	}

	esp_config.uart_idle = false;

	//disble DMA
	LL_DMA_DisableChannel(esp_config.DMA, esp_config.DMA_Channel);

	//total received size = total buffer size - left size for DMA
	uint32_t buff_len = DMA_BUF_SIZE - LL_DMA_GetDataLength(esp_config.DMA, esp_config.DMA_Channel);
	memcpy(req, esp_config.DMA_buf, buff_len+1);
	req[buff_len] = 0;

	//clear DMA flags
	esp_config.DMA->IFCR &= 0b1111 << ((esp_config.DMA_Channel -1) * 4);

	// configure data length
	LL_DMA_SetDataLength(esp_config.DMA, esp_config.DMA_Channel, DMA_BUF_SIZE);

	// enable DMA stream
	LL_DMA_EnableChannel(esp_config.DMA, esp_config.DMA_Channel);


	printf("rx len: %d\r\n%s\r\n==========\r\n", buff_len, req);

	return buff_len;
}

int wait_for(char *str, int m_time) {

	uint32_t tickstart = HAL_GetTick();
	uint32_t wait = m_time;

	char *str_ptr;
	char buf[512];

	while ((HAL_GetTick() - tickstart) < wait) {
		response(buf, m_time);
		str_ptr = strstr(buf, str);
		if (str_ptr != NULL)
			return 1;
	}
	return 0;
}

int server_init(char *ssid, char* password, uint16_t port) {

	esp_config.SSID = ssid;
	esp_config.password = password;
	esp_config.port = port;

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

	return 1;
}

int get_IPD(char *pdata) {
	char *IPD_ptr;
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


char* get_path_ptr(char *pdata) {

	char *path_ptr;

	path_ptr = strstr(pdata, "HTTP");

	if (path_ptr == NULL)
		return NULL;

	while (!(*(path_ptr-1) == ' ' && *path_ptr == '/')) {
		path_ptr--;
	}

	return path_ptr;
}

char* get_body_ptr(char *pdata) {

	char *body_ptr;
	body_ptr = strstr(pdata, "\r\n\r\n");

	while (body_ptr != NULL) {
		body_ptr += 4;
		if (strncmp(body_ptr, "+IPD", 4)) {
			return body_ptr;
		}
		body_ptr = strstr(body_ptr, "\r\n\r\n");
	}
	return NULL;
}

int Server_Send(uint8_t *pdata, int IPD) {

	uint8_t CMD[40];
	int header_len = strlen(OK_HEADER);
	int len = strlen(pdata);

	//Send Data
	sprintf (CMD, "AT+CIPSEND=%d,%d\r\n", IPD, header_len + len);
	esp_send(CMD);
	wait_for("OK", 100);

	esp_send(OK_HEADER);
	esp_send(pdata);
	wait_for("SEND OK", 100);

	sprintf (CMD, "AT+CIPCLOSE=%d\r\n", IPD);
	esp_send(CMD);
	wait_for("OK", 100);

	return 1;
}

void NOT_found(int IPD) {

	uint8_t CMD[40];
	int len = strlen(NOT_found_404);

	//Send Data
	sprintf (CMD, "AT+CIPSEND=%d,%d\r\n", IPD, len);
	esp_send(CMD);
	wait_for("OK", 100);

	esp_send(NOT_found_404);
	wait_for("SEND OK", 100);

	sprintf (CMD, "AT+CIPCLOSE=%d\r\n", IPD);
	esp_send(CMD);
	wait_for("OK", 100);
}

////// HTTP GET

int Server_GET_Handle(char *path, int IPD) {

	if (!strncmp(path, "/gpio", 5)) {
		GET_GPIO(IPD);
	} else if (!strncmp(path, "/", 1)) {
		Server_Send(ROOT_HTML, IPD);
	} else {
//		Server_GET_echo(path, IPD);
		NOT_found(IPD);
	}
}

int Server_GET_echo(uint8_t *path, int IPD) {
	const char *html1 = "<!doctype html><html><head><title>ESP8266</title></head><body><h1>";
	const char *html2 = "</h1></body></html>";

	uint32_t echo_len = strlen(html1) + strlen(html2) + strlen(path);

	char echo[echo_len + 1];
	strcpy(echo, html1);
	strcat(echo, &path[1]);
	strcat(echo, html2);

	Server_Send(echo, IPD);
}

int GET_GPIO(int IPD) {
	char data[15];
	uint32_t led_status[3];
	for (uint16_t i=0; i<LED_NUM_MAX; i++) {
		led_status[i] = led_read(i);
	}
	sprintf(data, "[%d,%d,%d]", led_status[0], led_status[1], led_status[2]);

	Server_Send(data, IPD);
}


////// HTTP_PUT

int Server_PUT_Handle(char *path, char *body, int IPD) {
	if (!strncmp(path, "/gpio", 5)) {
		PUT_GPIO(body, IPD);
		Server_Send("\0", IPD);
	}else {
		NOT_found(IPD);
	}
	return 0;
}


int PUT_GPIO(char *body, int IPD) {
	//문자열로 된 배열을 실제 배열로 변환
	int led_status[3];
	string_to_int_arr(led_status, body, LED_NUM_MAX);
	//받은 배열되로 led 상태를 변환
	for (int i=0; i<3; i++) {
		led_wirte(i, led_status[i]);
	}

	return 0;
}

