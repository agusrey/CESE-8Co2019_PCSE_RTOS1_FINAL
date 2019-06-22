/*=====[Disco.c]===================================================
 * Copyright 2019 Agustín Rey <agustinrey61@gmail.com>
 * All rights reserved.
 * Licencia: Texto de la licencia o al menos el nombre y un link
 (ejemplo: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Fecha de creacion: 2019/06/21
 */

/*=====[Inclusion de su propia cabecera]=====================================*/

/*=====[Inclusiones de dependencias de funciones privadas]===================*/
// sAPI header
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "sd_spi.h"   // <= own header (optional)
#include "sapi.h"     // <= sAPI header
#include "ff.h"       // <= Biblioteca FAT FS
#include "fssdc.h"      // API de bajo nivel para unidad "SDC:" en FAT FS
#include "board.h"
#include "Disco.h"

/*=====[Macros de definicion de constantes privadas]=========================*/
#define FILENAME "SDC:/Log.txt"
/*=====[Macros estilo funcion privadas]======================================*/

/*=====[Definiciones de tipos de datos privados]=============================*/
static FATFS fs;           // <-- FatFs work area needed for each volume
static FIL fp;             // <-- File object needed for each open file
static char buf[1000];

/*=====[Definiciones de Variables globales publicas externas]================*/

/*=====[Definiciones de Variables globales publicas]=========================*/
extern QueueHandle_t qLog_Write;

/*=====[Definiciones de Variables globales privadas]=========================*/

/*=====[Prototipos de funciones privadas]====================================*/
static void SdWrite(void* taskParmPtr);
static void DiskTp(void* taskParmPtr);

/*=====[Implementaciones de funciones publicas]==============================*/

void DiskLog(void* taskParmPtr)
{
	UINT nbytes;
	UINT i;
	Txt2Disk_t comLog;

	// SPI configuration
	spiConfig(SPI0);

	// Initialize SD card driver
	FSSDC_InitSPI();
	// Give a work area to the default drive
	if (f_mount(&fs, "SDC:", 0) != FR_OK)
	{
		// If this fails, it means that the function could
		// not register a file system object.
		// Check whether the SD card is correctly connected
	}

	while (1)
	{
		if (qLog_Write != NULL)
		{
			if (uxQueueMessagesWaiting(qLog_Write) > 0)
			{
				xQueueReceive(qLog_Write, &comLog, 0);
				for (i = 0; comLog.str[i] != '\0'; i++);
				comLog.str[i++]='\r';
				comLog.str[i++]='\n';

				if (f_open(&fp, FILENAME, FA_OPEN_APPEND | FA_WRITE) == FR_OK)
				{
					f_write(&fp, &comLog.str, i, &nbytes);

					f_close(&fp);

					if (nbytes == i)
					{
						// Turn ON LEDG if the write operation was successful
						gpioWrite(LEDG, ON);
						gpioWrite(LEDR, OFF);
					}
				}
				else
				{
					// Turn ON LEDR if the write operation was fail
					gpioWrite(LEDG, OFF);
					gpioWrite(LEDR, ON);
				}
				vTaskDelay(2 / portTICK_RATE_MS);
			}
		}
	}
}
/*=====[Implementaciones de funciones de interrupcion publicas]==============*/

/*=====[Implementaciones de funciones privadas]==============================*/

