/*
 * led.c
 *
 *  Created on: 2023. 3. 12.
 *      Author: asong
 */


#include "led.h"


static LED_SET LED[LED_NUM_MAX] = {{GPIOA, LL_GPIO_PIN_3},
								   {GPIOA, LL_GPIO_PIN_4},
								   {GPIOA, LL_GPIO_PIN_5}};


uint32_t led_read(uint16_t ch) {
	if (3 <= ch) {
		return 2;
	}
	return LL_GPIO_IsOutputPinSet(LED[ch].GPIO, LED[ch].PIN);
}

int led_wirte(uint16_t ch, int status) {

	if (LED_NUM_MAX <= ch)
		return 2;

	if (led_read(ch) != status) {
		LL_GPIO_TogglePin(LED[ch].GPIO, LED[ch].PIN);
	}

//	if (status == 0) {
//		LL_GPIO_ResetOutputPin(LED[ch].GPIO, LED[ch].PIN);
//	} else {
//		LL_GPIO_SetOutputPin(LED[ch].GPIO, LED[ch].PIN);
//	}

	return 1;
}
