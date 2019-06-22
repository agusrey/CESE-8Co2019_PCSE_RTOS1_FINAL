/*=====[Bluetooth.c]===================================================
 * Copyright 2019 Agustín Rey <agustinrey61@gmail.com>
 * All rights reserved.
 * Licencia: Texto de la licencia o al menos el nombre y un link
 (ejemplo: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Fecha de creacion: 2019/06/20
 *
 */

/*=====[Inclusiones de dependencias de funciones]============================*/


#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"


#include "task.h"
#include "queue.h"      //Api de colas
#include "sapi.h"        // <= Biblioteca sAPI
#include "Control.h"
#include <string.h>
#include "milib.h"
#include "Consola.h"         // <= Su propia cabecera
/*================[Prototipos]=============================================*/

/*=====[Macros de definición de constantes privadas]=========================*/

/*=====[Definiciones de variables globales externas]=========================*/

extern Motor_t Motor;
extern QueueHandle_t qCom_a_Bluetooth;

/*=====[Definiciones de variables globales publicas]=========================*/

QueueHandle_t qCom_de_Bluetooth;

#define UART_BLUETOOTH UART_232
#define UART_PC        UART_USB

/*=====[Definiciones de variables globales privadas]=========================*/
/*========================================================================================================================
 * Esta tarea espera por la llegada de un comandos para enviar a bluetooth
 * Además si recibe algún comando del Bluetooth se lo envía a al rutina de control
 *
 */

void Bluetooth(void* taskParmPtr)
{
	Comando_t Comando_a_Enviar;
	uint8_t data;
	uint8_t Bcomando[STRLENGTHMAX];
	uint8_t info[20];
	uint8_t i = 0;
	uint8_t numstr[4];
	bool completo = FALSE;
	while (1)
	{
		while (!completo)
		{
			if (uartReadByte( UART_BLUETOOTH, &data))
			{
				Bcomando[i++] = data;
				if (i >= (STRLENGTHMAX - 2))
					i = 0;
				if (data == '\0')
				{	//ACA se termino el string
					Bcomando[i-1] = '\r';
					Bcomando[i] = '\n';
					Bcomando[i+1] = '\0';
					i = 0;
					completo = TRUE;
				}
			}
			else//si encuentra que recibio un dato para enviar a Bluetooth lo manda a la UART_BLUETOOTH
			{
				if (qCom_a_Bluetooth != NULL)
				{
					if (uxQueueMessagesWaiting(qCom_a_Bluetooth) > 0)
					{
						xQueueReceive(qCom_a_Bluetooth, &info, 0);
						uartWriteString( UART_BLUETOOTH, info);
					}
				}
			}
		}
		completo = FALSE;

		//si llega acá es que se recibió un string, los analizamos y luego se envía a la cola

		if (searchforsubstr(&Bcomando[0], "START"))
		{
			strcpy(Comando_a_Enviar.str, "START");
			Comando_a_Enviar.tipo = START;
		}
		else
		{
			if (searchforsubstr(&Bcomando[0], "STOP"))
			{
				strcpy(Comando_a_Enviar.str, "STOP");
				Comando_a_Enviar.tipo = STOP;
			}
			else
			{
				if (searchforsubstr(&Bcomando[0], "FREC="))
				{
					if (Bcomando[5] != '\r')
					{
						numstr[0] = Bcomando[5];
						if (Bcomando[6]!='\r')
						{
							numstr[1] = Bcomando[5];
							numstr[2] = '\0';
						}
						else
							numstr[1] = '\0';

						Comando_a_Enviar.frecuencia = my_atoi(numstr);
						strcpy(Comando_a_Enviar.str, "FREC=");
						Comando_a_Enviar.tipo = UPDATE_FREC;
					}
				}
				else
				{
					continue;
				}
			}
		}
		if ( xQueueSend(qCom_de_Bluetooth, (void * ) &Comando_a_Enviar,
				(TickType_t ) 1) != pdPASS)
		{
			uartWriteString( UART_PC,
					"Error enviar cola de comandos de Bluetooth\r\n");
		}
	}
	vTaskDelay(2 / portTICK_PERIOD_MS);
}

