/*
 * led.h
 *
 *  Created on: 2023. 3. 12.
 *      Author: asong
 */

#ifndef LED_H_
#define LED_H_

#include "stdint.h"
#include "stm32f1xx_ll_gpio.h"

#define LED_NUM_MAX 	3

typedef struct {
	GPIO_TypeDef* GPIO;
	uint16_t PIN;
} LED_SET;


uint32_t led_read(uint16_t ch);
int led_wirte(uint16_t ch, int status);

#endif /* LED_H_ */
