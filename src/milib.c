/*=====[milib.c]===================================================
 * Copyright 2019 Agustín Rey <agustinrey61@gmail.com>
 * All rights reserved.
 * Licencia: Texto de la licencia o al menos el nombre y un link
 (ejemplo: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Fecha de creacion: 2019/06/20
 */

#include "lpc_types.h"


//----------- Rutina que convierte de entero a ascii base decimal---------------------------
char* itoa_dec(int value, char* result) {
	char* ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	if (result != NULL) {
		do {
			tmp_value = value;
			value /= 10;
			*ptr++ =
					"zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35
							+ (tmp_value - value * 10)];
		} while (value);

// Apply negative sign
		if (tmp_value < 0)
			*ptr++ = '-';
		*ptr-- = '\0';
		while (ptr1 < ptr) {
			tmp_char = *ptr;
			*ptr-- = *ptr1;
			*ptr1++ = tmp_char;
		}
		return result;
	}
}

#define MAXLENGTH 20
//rutna que busca un substrin dentro de un string y retorna TRUE si lo encuentra
bool searchforsubstr(char* str,  char* substr) {
	uint32_t i, j, l1 = 0, l2 = 0;
	volatile bool flag = FALSE;

	if (str == NULL || substr == NULL)//verifica que los punteros no sean nulos
		return (FALSE);

	while (str[l1] != '\0') {			//determina el largo del string
		l1++;
		if (l1 > MAXLENGTH)//verifica que la longitud del string no supere el maximo
			return (FALSE);
	}

	while (substr[l2] != '\0') {	//determina el largo del substring
		l2++;
		if (l2 > l1)	//verifica que la longitud del substring no supere la del string
			return (FALSE);
	}
	if (l2 > l1)		//el substring no puede ser mayor que el string
		return (FALSE);

	for (i = 0; i <= l1 - l2; i++) {
		for (j = i; j < i + l2; j++) {
			flag = TRUE;
			if (str[j] != substr[j - i]) {
				flag = FALSE;
				break;
			}
		}
		if (flag == TRUE)
			break;
	}
	return (flag);
}


//rutina que convierte un ascii en un entero

int my_atoi(char *p) {
    int k = 0;
    while (*p) {
        k = (k << 3) + (k << 1) + (*p) - '0';
        p++;
     }
     return k;
}
