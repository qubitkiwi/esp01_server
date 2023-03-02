/*
 * esp8266.h
 *
 *  Created on: 2023. 2. 24.
 *      Author: asong
 */

#ifndef ESP8266_H_
#define ESP8266_H_

#include "stm32f1xx.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_usart.h"

#include "stdint.h"
#include "string.h"
#include "stdbool.h"


#define DMA_BUF_SIZE 512

typedef struct {
	char *SSID;
	char *password;
	USART_TypeDef *uart;
	DMA_TypeDef *DMA;
	bool uart_idle;
	uint16_t port;
	char DMA_buf[DMA_BUF_SIZE];
} ESP_CONFIG;

typedef enum {
	HTTP_ERROR 	= 0,
	HTTP_GET	= 1,
	HTTP_POST   = 2,
	HTTP_PUT	= 3,
	HTTP_DELETE = 4,
} HTTP_METHOD;


int esp8266_init(USART_TypeDef *uart, DMA_TypeDef *DMA, char *ssid, char* pass);
int response(char *req, int m_time);
int wait_for(char *str, int m_time);

void esp_send(char *pdata);
int server_init();

int get_IPD(char *pdata);
HTTP_METHOD get_method(char *pdata);
int get_path(char *pdata, char *path);

int Server_GET_Handle(uint8_t *path, uint32_t IPD);
int Server_Send(uint8_t *data, int IPD);
void NOT_found(int IPD);

void uart_it();


#endif /* ESP8266_H_ */
