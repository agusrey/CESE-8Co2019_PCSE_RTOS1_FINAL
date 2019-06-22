/*=====[Consola.c]===================================================
 * Copyright 2019 Agustín Rey <agustinrey61@gmail.com>
 * All rights reserved.
 * Licencia: Texto de la licencia o al menos el nombre y un link
 (ejemplo: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Fecha de creacion: 2019/06/18
 */

/*=====[Inclusiones de dependencias de funciones]============================*/
#include "Consola.h"         // <= Su propia cabecera

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

#include "mi_mcpwm.h"	//manejo del MCPWM
#include "task.h"
#include "queue.h"      //Api de colas
#include "sapi.h"        // <= Biblioteca sAPI

#include "Botones.h"
#include "app.h"
#include "Control.h"

#include <string.h>
#include "milib.h"
/*================[Prototipos]=============================================*/

/*=====[Macros de definición de constantes privadas]=========================*/

/*=====[Definiciones de variables globales externas]=========================*/
extern Motor_t Motor;
extern QueueHandle_t qCom_de_Consola;
extern QueueHandle_t qCom_a_Consola;
/*=====[Definiciones de variables globales publicas]=========================*/

/*
 uint8_t Com_de_Consola[STRLENGTHMAX];
 uint8_t Com_a_Consola[STRLENGTHMAX];

 QueueHandle_t qCom_de_Consola;
 QueueHandle_t qCom_a_Consola;
 */
#define UART_PC        UART_USB
#define UART_BLUETOOTH UART_232

/*=====[Definiciones de variables globales privadas]=========================*/
/*========================================================================================================================
 * Esta tarea espera por la llegada de un comando de consola que no debe superar los strlengthmax-2 caracteres, sino se descarta.
 * Se queda esperando por el string y si no hay byte para leer hace un vtaskdelay de 10 ms y reintenta
 * Cuando se completa el string lo evalua y si es uno de los esperados, envía el string a la cola para que la tarea control lo procese
 * Entonces vuelve a buscar la llegada de otro comando.
 *
 */

void consola(void* taskParmPtr)
{

	Comando_t Comando_a_Enviar;
	static uint8_t comando[20];
	Txt2Cons_t ComandoRecibido;
	static uint8_t info[20];
	uint8_t i = 0;
	uint8_t data;
	uint8_t numstr[4];

	bool completo = FALSE;

	while (1)
	{
		while (!completo)
		{
			if (uartReadByte( UART_PC, &data))
			{
				comando[i++] = data;
				if (i >= (STRLENGTHMAX - 2))
					i = 0;
				if (data == '\n')
				{	//ACA PULSARON ENTER, TENGO EL STRING, SALIR DEL WHILE
					comando[i] = '\0';
					i = 0;
					completo = TRUE;
				}
			}
			else//si encuentra que recibio un dato para enviar a consola lo manda a la UART_PC
			{
				if (qCom_a_Consola != NULL)
				{
					if (uxQueueMessagesWaiting(qCom_a_Consola) > 0)
					{
						xQueueReceive(qCom_a_Consola, &ComandoRecibido.str[0],
								0);
						uartWriteString( UART_PC, &ComandoRecibido.str[0]);
						uartWriteString( UART_PC, "\n\r");
						vTaskDelay(2 / portTICK_RATE_MS);
					}
				}
			}
		}
		completo = FALSE;

		//si llega acá es que se recibió un string, los analizamos y luego se envía a la cola

		if (searchforsubstr(&comando[0], "START"))
		{
			strcpy(Comando_a_Enviar.str, "START");
			Comando_a_Enviar.tipo = START;
		}
		else
		{
			if (searchforsubstr(&comando[0], "STOP"))
			{
				strcpy(Comando_a_Enviar.str, "STOP");
				Comando_a_Enviar.tipo = STOP;
			}
			else
			{
				if (searchforsubstr(&comando[0], "FREC="))
				{
					if (comando[5] != '\r')
					{
						numstr[0] = comando[5];
						if (comando[6]!='\r')
						{
							numstr[1] = comando[5];
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
		if ( xQueueSend(qCom_de_Consola, (void * ) &Comando_a_Enviar,
				(TickType_t ) 1) != pdPASS)
		{
			uartWriteString( UART_PC,
					"Error enviar cola de comandos de consola\r\n");
		}
	}
}

