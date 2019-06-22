/*=====[Control.c]===================================================
 * Copyright 2019 Agustín Rey <agustinrey61@gmail.com>
 * All rights reserved.
 * Licencia: Texto de la licencia o al menos el nombre y un link
 (ejemplo: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Fecha de creacion: 2019/06/18
 */

/*=====[Inclusion de su propia cabecera]=====================================*/

/*=====[Inclusiones de dependencias de funciones privadas]===================*/
// sAPI header
#include "sapi.h"
#include "board.h"
#include "sapi_peripheral_map.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"      //Api de colas
#include "Botones.h"
#include "Control.h"
#include "string.h"
#include "Bluetooth.h"
#include "Disco.h"
#include "app.h"
/*=====[Macros de definicion de constantes privadas]=========================*/

//#define CANT_TECLAS 4
/*=====[Macros estilo funcion privadas]======================================*/

/*=====[Definiciones de tipos de datos privados]=============================*/

// Tipo de datos que renombra un tipo basico

/*=====[Definiciones de Variables globales publicas externas]================*/
extern QueueHandle_t qTecla_Apretada;
extern QueueHandle_t qCom_a_Consola;
extern QueueHandle_t qCom_de_Consola;
extern QueueHandle_t qCom_a_Bluetooth;
extern QueueHandle_t qCom_de_Bluetooth;
extern QueueHandle_t qLog_Write;
extern arrebSt_t BotSt[CANT_TECLAS];
extern const gpioMap_t teclas[4];
/*=====[Definiciones de Variables globales publicas]=========================*/

//int32_t varGlobalPublica = 0;
/*=====[Definiciones de Variables globales privadas]=========================*/

Motor_t Motor =
{ FALSE, 10 };

/*=====[Prototipos de funciones privadas]====================================*/
char* itoa_dec(int value, char* result);
/*=====[Implementaciones de funciones publicas]==============================*/


void Control(void* taskParmPtr)
{
	uint8_t bp;
	uint8_t info[20];
	Txt2Cons_t txt2Con;
	Txt2Blue_t txt2Blue;
	Txt2Disk_t txt2Disk;

	uint8_t aux[10];
	Comando_t Com_de_Consola;
	Comando_t Com_de_Bluetooth;


	while (1)
	{

		/*
		 * Aqui verifica si hay tecla apretada y actúa en consecuencia. Informa a consola y bluetooth y hace el Log
		 */

		if (qTecla_Apretada != NULL)
		{
			if (uxQueueMessagesWaiting(qTecla_Apretada) > 0)
			{
				xQueueReceive(qTecla_Apretada, &bp, 0);
				switch (bp)
				{
				case TEC1:
					if (Motor.marcha != TRUE)
					{
						Motor.marcha = TRUE;
						strcpy(txt2Con.str, "Start. Frec =  ");
						itoa_dec(Motor.frecuencia, &txt2Con.str[12]);
					}
					break;
				case TEC2:
					Motor.marcha = FALSE;
					strcpy(txt2Con.str, "Stop");
					break;
				case TEC3:
					if (Motor.frecuencia < 50)
						Motor.frecuencia++;
					else
						Motor.frecuencia = 50;
					strcpy(txt2Con.str, "Run. Frec =  ");
					itoa_dec(Motor.frecuencia, &txt2Con.str[12]);
					break;
				case TEC4:
					if (Motor.frecuencia > 0)
						Motor.frecuencia--;
					else
						Motor.frecuencia = 0;
					strcpy(txt2Con.str, "Run. Frec =  ");
					itoa_dec(Motor.frecuencia, &txt2Con.str[12]);
					break;
				}

				if ( xQueueSend(qCom_a_Consola, &txt2Con,(TickType_t ) 1) != pdPASS)
				{
					uartWriteString( UART_PC, "Error envio cola de mensajes consola\r\n");
				}
				if ( xQueueSend(qCom_a_Bluetooth, (Txt2Blue_t *) &txt2Con,
						(TickType_t ) 1) != pdPASS)
				{
					uartWriteString( UART_PC, "Error envio cola de mensajes Bluetooth\r\n");
				}

				strcpy(txt2Disk.str,txt2Blue.str);
				txt2Disk.print=FALSE;
				if ( xQueueSend(qLog_Write, &txt2Disk,
						(TickType_t ) 1) != pdPASS)
				{
					uartWriteString( UART_PC, "Error envio cola de mensajes Disco\r\n");
				}
			}
		}

		/*
		 * Aqui verifica si hay algún comando de consola y actúa en consecuencia. Informa a bluetooth y hace el log.
		 */
		if (qCom_de_Consola != NULL)
		{
			if (uxQueueMessagesWaiting(qCom_de_Consola) > 0)
			{
				xQueueReceive(qCom_de_Consola, &Com_de_Consola, 0);
				switch (Com_de_Consola.tipo)
				{
				case START:
					Motor.marcha = TRUE;
					strcpy(txt2Blue.str, "START, FREC=");
					itoa_dec(Motor.frecuencia, aux);
					strcat(txt2Blue.str, aux);
					if ( xQueueSend(qCom_a_Bluetooth, &txt2Blue,
							(TickType_t ) 1) != pdPASS)
					{
						uartWriteString( UART_PC, "Error envio cola de mensajes Bluetooth\r\n");
					}
					strcpy(txt2Disk.str,txt2Blue.str);
					txt2Disk.print=FALSE;
					if ( xQueueSend(qLog_Write, &txt2Disk,
							(TickType_t ) 1) != pdPASS)
					{
						uartWriteString( UART_PC, "Error envio cola de mensajes Disco\r\n");
					}

					break;
				case STOP:
					Motor.marcha = FALSE;
					strcpy(txt2Blue.str, "STOP");
					if ( xQueueSend(qCom_a_Bluetooth, &txt2Blue,
							(TickType_t ) 1) != pdPASS)
					{
						uartWriteString( UART_PC, "Error envio cola de mensajes Bluetoot\r\n");
					}
					strcpy(txt2Disk.str,txt2Blue.str);
					txt2Disk.print=FALSE;
					if ( xQueueSend(qLog_Write, &txt2Disk,
							(TickType_t ) 1) != pdPASS)
					{
						uartWriteString( UART_PC, "Error envio cola de mensajes Disco\r\n");
					}

					break;
				case UPDATE_FREC:
					Motor.frecuencia = Com_de_Consola.frecuencia;
					if (Motor.marcha == TRUE)
						strcpy(txt2Blue.str, "RUN, FREC=");
					else
						strcpy(txt2Blue.str, "STOP, FREC=");
					itoa_dec(Motor.frecuencia, aux);
					strcat(txt2Blue.str, aux);
					if ( xQueueSend(qCom_a_Bluetooth, &txt2Blue,
							(TickType_t ) 1) != pdPASS)
					{
						uartWriteString( UART_PC, "Error envio cola de mensajes Bluetooth\r\n");
					}
					strcpy(txt2Disk.str,txt2Blue.str);
					txt2Disk.print=FALSE;
					if ( xQueueSend(qLog_Write, &txt2Disk,
							(TickType_t ) 1) != pdPASS)
					{
						uartWriteString( UART_PC, "Error envio cola de mensajes Disco\r\n");
					}

					break;
				}
			}

			/*
			 * Aqui verifica si hay algún comando de Bluetooth y actúa en consecuencia. Informa a Consola y hace el log
			 */
			if (qCom_de_Bluetooth != NULL)
			{
				if (uxQueueMessagesWaiting(qCom_de_Bluetooth) > 0)
				{
					xQueueReceive(qCom_de_Bluetooth, &Com_de_Bluetooth, 0);
					switch (Com_de_Bluetooth.tipo)
					{
					case START:
						Motor.marcha = TRUE;
						strcpy(txt2Con.str, "START, FREC=");
						itoa_dec(Motor.frecuencia, aux);
						strcat(txt2Con.str, aux);
						if ( xQueueSend(qCom_a_Consola, &txt2Con,
								(TickType_t ) 1) != pdPASS)
						{
							uartWriteString( UART_PC, "Error envio cola de mensajes a Consola\r\n");
						}
						strcpy(txt2Disk.str,txt2Con.str);
						txt2Disk.print=FALSE;
						if ( xQueueSend(qLog_Write, &txt2Disk,
								(TickType_t ) 1) != pdPASS)
						{
							uartWriteString( UART_PC, "Error envio cola de mensajes Disco\r\n");
						}


						break;
					case STOP:
						Motor.marcha = FALSE;
						strcpy(txt2Con.str, "STOP");
						if ( xQueueSend(qCom_a_Consola, &txt2Con,
								(TickType_t ) 1) != pdPASS)
						{
							uartWriteString( UART_PC, "Error envio cola de mensajes Consola\r\n");
						}

						strcpy(txt2Disk.str,txt2Con.str);
						txt2Disk.print=FALSE;
						if ( xQueueSend(qLog_Write, &txt2Disk,
								(TickType_t ) 1) != pdPASS)
						{
							uartWriteString( UART_PC, "Error envio cola de mensajes Disco\r\n");
						}

						break;
					case UPDATE_FREC:
						Motor.frecuencia = Com_de_Bluetooth.frecuencia;
						if (Motor.marcha == TRUE)
							strcpy(txt2Con.str, "RUN, FREC=");
						else
							strcpy(txt2Con.str, "STOP, FREC=");
						itoa_dec(Motor.frecuencia, aux);
						aux[2]='\r';
						aux[3]='\n';
						aux[4]='\0';
						strcat(txt2Con.str, aux);
						if ( xQueueSend(qCom_a_Consola, &txt2Con,
								(TickType_t ) 1) != pdPASS)
						{
							uartWriteString( UART_PC, "Error envio cola de mensajes Consola\r\n");
						}
						strcpy(txt2Disk.str,txt2Con.str);
						txt2Disk.print=FALSE;
						if ( xQueueSend(qLog_Write, &txt2Disk,
								(TickType_t ) 1) != pdPASS)
						{
							uartWriteString( UART_PC, "Error envio cola de mensajes Disco\r\n");
						}

						break;
					}
				}

			}

		}

		//-----aca se cierra el while(1)-----------------------
	}
}
/*=====[Implementaciones de funciones de interrupcion publicas]==============*/

/*=====[Implementaciones de funciones privadas]==============================*/

