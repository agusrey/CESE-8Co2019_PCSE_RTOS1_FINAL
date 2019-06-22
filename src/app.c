/*=====[Nombre del programa]===================================================
 * Copyright 2019 Agustín Rey <agustinrey61@gmail.com>
 * All rights reserved.
 * Licencia: Texto de la licencia o al menos el nombre y un link
 (ejemplo: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Fecha de creacion: 2019/06/12
 */

/*=====[Inclusiones de dependencias de funciones]============================*/
#include "app.h"         // <= Su propia cabecera
#include "milib.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

#include "mi_mcpwm.h"	//manejo del MCPWM
#include "task.h"
#include "queue.h"
#include "sapi.h"        // <= Biblioteca sAPI

#include "Botones.h"

#include <string.h>
#include "Control.h"
#include "Bluetooth.h"
#include "Consola.h"
#include "math.h"
#include "Disco.h"
#include "app.h"

/*================[Prototipos]=============================================*/
void Led_parpadea(void* taskParmPtr);
void hm10blePrintATCommands(int32_t uart);
bool_t hm10bleTest(int32_t uart);

/*=====[Definiciones de variables globales publicas]=========================*/

QueueHandle_t qCom_de_Consola;		//cola de comandos de consola
QueueHandle_t qCom_a_Consola;		//cola de comandos a consola
QueueHandle_t qCom_de_Bluetooth;	//cola de comandos de Bluetooth
QueueHandle_t qCom_a_Bluetooth;		//cola de comandos a Bluetooth
QueueHandle_t qTecla_Apretada;		//cola de teclas
QueueHandle_t qLog_Write;			//cola de eventos de consola a grabar en disco

Motor_t Motor;		//estructura Motor, variable global

static MCPWM_CHANNEL_CFG_Type Pwm_Channels[3];
typedef struct
{
	uint32_t pulse_width[3];
} MCPWM_CHANNEL_PULSEWIDTH;



/*=====[Definiciones de variables globales privadas]=========================*/

//========= FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void)
{

	// ---------- CONFIGURACIONES ------------------------------

	// Inicializar y configurar la plataforma
	boardConfig();
	Motor.marcha=FALSE;
	Motor.frecuencia=5;

	gpioInit(GPIO8, GPIO_OUTPUT); //prepara GPIO8 como salida. Esta salida se togglea con la interrupción del MCPWM
	MCPWM_Init(LPC_MCPWM);						//inicializa el MCPWM
	MCPWM_Pin_Init();					//configura pines de salida del MCPWM
	MCPWM_ACMode(LPC_MCPWM, ENABLE);			//configura el MCPWM en modo AC
	MCPWM_InitChannels(LPC_MCPWM, Pwm_Channels);//inicializa los 3 canales del MCPWM

	//======== Configura la Interrupción del MCPWM
	MCPWM_IntConfig(LPC_MCPWM, MCPWM_INTFLAG_LIM0, ENABLE); //configura interrupción cuando se completa el período en el canal 0
	NVIC_EnableIRQ(MCPWM_IRQn);				//habilita la inerrupción en el NVIC
	NVIC_SetPriority(MCPWM_IRQn, 2); 						//prioridad 2

	MCPWM_Start(LPC_MCPWM, 1, 1, 1); //arranca el MCPWM los 3 canales a la vez

														//se inicializan las distintas colas
	qCom_de_Consola = xQueueCreate(5, sizeof(Comando_t));
	if (qCom_de_Consola == NULL)
		uartWriteString(UART_PC,
				"Error en creación de cola de comandos de consola.\r\n");
	qCom_a_Consola = xQueueCreate(5, sizeof(Txt2Cons_t));
	if (qCom_a_Consola == NULL)
		uartWriteString(UART_PC,
				"Error en creación de cola de comandos hacia consola.\r\n");

	qCom_de_Bluetooth = xQueueCreate(5, sizeof(Comando_t));
	if (qCom_de_Bluetooth == NULL)
		uartWriteString(UART_PC,
				"Error en creación de cola de comandos de Bluetooth.\r\n");

	qCom_a_Bluetooth = xQueueCreate(5, sizeof(Txt2Blue_t));
	if (qCom_a_Bluetooth == NULL)
		uartWriteString(UART_PC,
				"Error en creación de cola de comandos hacia Bluetooth.\r\n");

	qTecla_Apretada = xQueueCreate(5, sizeof(uint8_t));
	if (qTecla_Apretada == NULL)
		uartWriteString(UART_PC, "Error en creación de cola de teclas.\r\n");

	qLog_Write = xQueueCreate(5, sizeof(Txt2Disk_t));
	if (qLog_Write == NULL)
		uartWriteString(UART_PC, "Error en creación de cola deL LOG.\r\n");


	// Inicializar UART_USB para conectar a la PC
	uartConfig(UART_PC, 115200);
	uartWriteString(UART_PC, "UART_PC configurada.\r\n");

	// Inicializar UART_232 para conectar al modulo bluetooth
	uartConfig(UART_BLUETOOTH, 9600);
	uartWriteString(UART_PC,
			"UART_BLUETOOTH para modulo Bluetooth configurada.\r\n");

	uint8_t data = 0;

	uartWriteString(UART_PC,
			"Testeto si el modulo esta conectado enviando: AT\r\n");
	while (1)
	{
		if (hm10bleTest(UART_BLUETOOTH))
		{
			uartWriteString(UART_PC, "Modulo conectado correctamente.\r\n");
			break;
		}
		else
		{
			uartWriteString(UART_PC, "No funciona, reintentando.\r\n");
		}
		delay(2000);
	}

	//si llegó acá es que está conectado

	//========= CREACIÓN DE LAS TAREAS =================================
	xTaskCreate(Led_parpadea,                  // Funcion de la tarea a ejecutar
			(const char *) "Led_parpadea", // Nombre de la tarea como String amigable para el usuario
			configMINIMAL_STACK_SIZE * 2, // Cantidad de stack de la tarea
			0,                          // Parametros de tarea
			tskIDLE_PRIORITY + 3,         // Prioridad de la tarea(baja)
			0                         // Puntero a la tarea creada en el sistema
			);

	xTaskCreate(Bluetooth,                  // Funcion de la tarea a ejecutar
			(const char *) "Bluetooth", // Nombre de la tarea como String amigable para el usuario
			configMINIMAL_STACK_SIZE * 2, // Cantidad de stack de la tarea
			0,                          // Parametros de tarea
			tskIDLE_PRIORITY + 2,         // Prioridad de la tarea(baja)
			0                         // Puntero a la tarea creada en el sistema
			);

	xTaskCreate(tecla,                  // Funcion de la tarea a ejecutar
			(const char *) "Teclado", // Nombre de la tarea como String amigable para el usuario
			configMINIMAL_STACK_SIZE * 2, // Cantidad de stack de la tarea
			0,                          // Parametros de tarea
			tskIDLE_PRIORITY + 2,         // Prioridad de la tarea(baja)
			0                         // Puntero a la tarea creada en el sistema
			);

	xTaskCreate(consola,                  // Funcion de la tarea a ejecutar
			(const char *) "Consola", // Nombre de la tarea como String amigable para el usuario
			configMINIMAL_STACK_SIZE * 2, // Cantidad de stack de la tarea
			0,                          // Parametros de tarea
			tskIDLE_PRIORITY + 2,         // Prioridad de la tarea(baja)
			0                         // Puntero a la tarea creada en el sistema
			);

	xTaskCreate(Control,                  // Funcion de la tarea a ejecutar
			(const char *) "Control", // Nombre de la tarea como String amigable para el usuario
			configMINIMAL_STACK_SIZE * 2, // Cantidad de stack de la tarea
			0,                          // Parametros de tarea
			tskIDLE_PRIORITY + 2,         // Prioridad de la tarea(baja)
			0                         // Puntero a la tarea creada en el sistema
			);

	xTaskCreate(DiskLog,                  // Funcion de la tarea a ejecutar
			(const char *) "DiskLog", // Nombre de la tarea como String amigable para el usuario
			configMINIMAL_STACK_SIZE * 2, // Cantidad de stack de la tarea
			0,                          // Parametros de tarea
			tskIDLE_PRIORITY + 2,         // Prioridad de la tarea(baja)
			0                         // Puntero a la tarea creada en el sistema
			);


	// Iniciar scheduler
	vTaskStartScheduler();

	// ---------- REPETIR POR SIEMPRE --------------------------
	while ( TRUE)
	{

	}

	// NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
	// directamenteno sobre un microcontroladore y no es llamado por ningun
	// Sistema Operativo, como en el caso de un programa para PC.
	return 0;
}

//===== Tarea que parpadea un led, solo para mostrar que está con vida
void Led_parpadea(void* taskParmPtr)
{
	while (TRUE)
	{
		gpioToggle(LED3);
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

bool_t hm10bleTest(int32_t uart)
{
	uartWriteString(uart, "AT\r\n");
	return waitForReceiveStringOrTimeoutBlocking(uart, "OK\r\n",
			strlen("OK\r\n"), 1000);
}

//--------HANDLER DE AL INTERRUPCIÓN DEL MCPWM
#define KANGLE 858993
#define NOPULSE 20500
#define MAXPW 20000
#define MINPW 100

void MCPWM_IRQHandler(void)
{

	MCPWM_CHANNEL_CFG_Type ChannelConfig;
	static MCPWM_CHANNEL_PULSEWIDTH canal;

	static uint8_t cnt = 100;
	bool marcha = FALSE;
	static uint32_t angle;
	uint32_t pulse_width;
	Trig_Components Trig;
	uint32_t auxAngle;
	uint32_t auxSin;

	if (MCPWM_GetIntStatus(LPC_MCPWM, MCPWM_INTFLAG_LIM0))
	{
		MCPWM_IntClear(LPC_MCPWM, MCPWM_INTFLAG_LIM0);
		gpioToggle(GPIO8); 			//se togglea un pin que sirve de disparo del osciloscopio
		if ((cnt--) == 0)
		{
			cnt = 100;
			gpioToggle(LED1); 		//se togglea un led como testigo de que está interrumpiendo
		}

		if (Motor.marcha == TRUE)
		{
			marcha = TRUE;
			angle = angle + Motor.frecuencia * KANGLE;	//calcula nuevo ángulo
		}
		else
		{
			marcha = FALSE;
			angle = 0;
		}

		if (marcha == FALSE){
			pulse_width = NOPULSE;
			ChannelConfig.channelPulsewidthValue = pulse_width;
			ChannelConfig.channelPeriodValue = 20400;
			MCPWM_WriteToShadow(LPC_MCPWM, 0, &ChannelConfig);
			MCPWM_WriteToShadow(LPC_MCPWM, 1, &ChannelConfig);
			MCPWM_WriteToShadow(LPC_MCPWM, 2, &ChannelConfig);
		}
		else
		{
			auxAngle = angle >> 16;							//angulo en formato q1.15
			Trig = MCM_Trig_Functions((int16_t)(auxAngle));	//se obtiene el seno de la tabla
			auxSin = 32768 + Trig.hSin;						//se normaliza
			pulse_width = MINPW + (auxSin * MAXPW) >> 16;	//calcual pulse width

			ChannelConfig.channelPulsewidthValue = pulse_width;	//escritira en los registros
			ChannelConfig.channelPeriodValue = 20400;
			MCPWM_WriteToShadow(LPC_MCPWM, 0, &ChannelConfig);

			auxAngle = (angle+0x55555555) >> 16;			//se desplaza 120 grados y recalcula canal 1
			Trig = MCM_Trig_Functions((int16_t)(auxAngle));
			auxSin = 32768 + Trig.hSin;
			pulse_width = MINPW + (auxSin * MAXPW) >> 16;

			ChannelConfig.channelPulsewidthValue = pulse_width;
			ChannelConfig.channelPeriodValue = 20400;
			MCPWM_WriteToShadow(LPC_MCPWM, 1, &ChannelConfig);

			auxAngle = (angle+0xaaaaaaaa) >> 16;			//se desplaza 240 grados y recalcula canal 2
			Trig = MCM_Trig_Functions((int16_t)(auxAngle));
			auxSin = 32768 + Trig.hSin;
			pulse_width = MINPW + (auxSin * MAXPW) >> 16;

			ChannelConfig.channelPulsewidthValue = pulse_width;
			ChannelConfig.channelPeriodValue = 20400;
			MCPWM_WriteToShadow(LPC_MCPWM, 2, &ChannelConfig);

		}

		NVIC_ClearPendingIRQ(MCPWM_IRQn);
	}
}

