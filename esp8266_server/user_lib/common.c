/*
 * common.c
 *
 *  Created on: 2023. 3. 13.
 *      Author: asong
 */

#include "common.h"


/**
 * "[0,1,23,34]"의 문자열을 [0,1,23,34]의 정수형 배열로 변환한다.
 *
 */
int string_to_int_arr(int *arr, char *str, int len) {
	int idx = 0;
	while (*str != '\0' && len) {
		if (*str == '[' || *str == ']' || *str == ',') {
			str++;
			continue;
		} else {
			arr[idx++] = atoi(str);
			len--;
			while ( '0' <= *str  && *str <= '9' ) {
				str++;
			}
		}
	}
	return 1;
}
