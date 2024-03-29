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
#include "common.h"

#define DMA_BUF_SIZE 512

typedef struct {
	USART_TypeDef *uart;
	DMA_TypeDef *DMA;
	uint32_t DMA_Channel;

	char *SSID;
	char *password;
	uint16_t port;

	bool uart_idle;

	char DMA_buf[DMA_BUF_SIZE];
} ESP_CONFIG;

typedef enum {
	HTTP_ERROR 	= 0,
	HTTP_GET	= 1,
	HTTP_POST   = 2,
	HTTP_PUT	= 3,
	HTTP_DELETE = 4,
} HTTP_METHOD;


int esp8266_init(USART_TypeDef *uart, DMA_TypeDef *DMA, uint32_t Channel);
int server_init(char *ssid, char* password, uint16_t port);

void esp_send(char *pdata);
int response(char *req, int m_time);
int wait_for(char *str, int m_time);

int get_IPD(char *pdata);
HTTP_METHOD get_method(char *pdata);
char* get_path_ptr(char *pdata);
char* get_body_ptr(char *pdata);

int Server_Send(uint8_t *data, int IPD);
void NOT_found(int IPD);

int Server_GET_Handle(char *path, int IPD);
int GET_GPIO(int IPD);
int Server_GET_echo(uint8_t *path, int IPD);

int Server_PUT_Handle(char *path, char *body, int IPD);
int PUT_GPIO(char *body, int IPD);

void uart_it();


#endif /* ESP8266_H_ */
