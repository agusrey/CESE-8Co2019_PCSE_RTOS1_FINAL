/*=====[Botones.c]===================================================
 * Copyright 2019 Agustín Rey <agustinrey61@gmail.com>
 * All rights reserved.
 * Licencia: Texto de la licencia o al menos el nombre y un link
 (ejemplo: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Fecha de creacion: 2019/06/12
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
#include "semphr.h"

#include "Botones.h"
/*=====[Macros de definicion de constantes privadas]=========================*/

/*=====[Macros estilo funcion privadas]======================================*/

/*=====[Definiciones de tipos de datos privados]=============================*/

/*=====[Definiciones de Variables globales publicas externas]================*/
#define TIEMPO_ANTIRREBOTE 40

/*=====[Definiciones de Variables globales publicas]=========================*/
extern QueueHandle_t qTecla_Apretada;

/*=====[Definiciones de Variables globales privadas]=========================*/

const gpioMap_t teclas[4] =
{ TEC1, TEC2, TEC3, TEC4 };
arrebSt_t BotSt[CANT_TECLAS];

/*=====[Prototipos de funciones privadas]====================================*/
static void fsmArrebInicia(arrebSt_t* Boton, gpioMap_t tecla);
static bool fsmArreb(arrebSt_t* Boton); //antirrebote botones, retorna TRUE si el botón es pulsado

/*=====[Implementaciones de funciones publicas]==============================*/

void tecla(void* taskParmPtr)
{
	uint32_t tecIndice;
	uint8_t tec_presionada;

/*	while(1){
		vTaskDelay(5 / portTICK_PERIOD_MS);
	}
*/

	for (tecIndice = 0; tecIndice < CANT_TECLAS; tecIndice++)
		fsmArrebInicia(&BotSt[tecIndice], teclas[tecIndice]);



	while (1)
	{
		for (tecIndice = 0; tecIndice < CANT_TECLAS; tecIndice++)
		{
			if (fsmArreb(&BotSt[tecIndice]))
			{

				tec_presionada = teclas[tecIndice];
				if ( xQueueSend(qTecla_Apretada, &tec_presionada,
						(TickType_t ) 1) != pdPASS)
				{
					uartWriteString(3, "Error cola de teclas\r\n");
				}

			}
			vTaskDelay(5 / portTICK_PERIOD_MS);
		}
	}
}
/*=====[Implementaciones de funciones de interrupcion publicas]==============*/

/*=====[Implementaciones de funciones privadas]==============================*/

static void fsmArrebInicia(arrebSt_t* Boton, gpioMap_t tecla)
{
	Boton->boton = tecla;
	Boton->status = UP;
}

static bool fsmArreb(arrebSt_t* Boton)
{
	bool retorno = FALSE;
	if (Boton == NULL)
		return (retorno);

	switch (Boton->status)
	{
	default:
	case UP:
		if (!gpioRead(Boton->boton)) //detecta que se pulsó, va a falling
		{
			delayConfig(&Boton->demSt, TIEMPO_ANTIRREBOTE);
			delayRead(&Boton->demSt);	//al leerlo se le da arranque
			Boton->status = FALLING;
		}
		break;
	case FALLING:
		if (delayRead(&Boton->demSt)) //pasó el tiempo, verificar que la tecla sigue apretada
		{
			if (!gpioRead(Boton->boton))
			{
				retorno = TRUE;
				Boton->status = DOWN;
			}
			else
				Boton->status = UP;
		}
		break;
	case DOWN:
		if (gpioRead(Boton->boton)) //detecta que se soltó, va a estado SOLTANDO
		{
			delayConfig(&Boton->demSt, TIEMPO_ANTIRREBOTE);
			delayRead(&Boton->demSt);	//al leerlo se le da arranque
			Boton->status = RISING;
		}
		break;
	case RISING:
		if (delayRead(&Boton->demSt)) //pasó el tiempo, verificar que la tecla sigue soltada
		{
			if (gpioRead(Boton->boton))
			{
				Boton->status = UP;
			}
			else
				Boton->status = DOWN;
		}
		break;
	}
	return (retorno);
}
