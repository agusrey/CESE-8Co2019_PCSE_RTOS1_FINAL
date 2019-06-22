/*=====[Botones.h]===================================================
 * Copyright 2019 Agustín Rey <agustinrey61@gmail.com>
 * All rights reserved.
 * Licencia: Texto de la licencia o al menos el nombre y un link
         (ejemplo: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Fecha de creacion: 2019/06/12
 */

/*=====[Evitar inclusion multiple comienzo]==================================*/

#ifndef _BOTONES_H_
#define _BOTONES_H_

/*=====[Inclusiones de dependencias de funciones publicas]===================*/

/*=====[C++ comienzo]========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Macros de definicion de constantes publicas]=========================*/

#define CANT_TECLAS 4

/*=====[Macros estilo funcion publicas]======================================*/


// Tipo de datos que renombra un tipo basico

// Tipo de datos de puntero a funcion

// Tipo de datos enumerado

typedef enum {
   UP,
   FALLING,
   DOWN,
   RISING
}BotStatus_t;

/*=====[Definiciones de tipos de datos publicos]=============================*/
typedef struct {
	uint8_t numbot;
	bool press;
} BtnSt_t;

typedef struct {
	gpioMap_t boton;
	BotStatus_t status;
	delay_t demSt;
} arrebSt_t;





// Tipo de datos estructua, union o campo de bits

/*=====[Prototipos de funciones publicas]====================================*/
void tecla(void* taskParmPtr);

/*=====[Prototipos de funciones publicas de interrupcion]====================*/



/*=====[C++ fin]=============================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Evitar inclusion multiple fin]=======================================*/

#endif /* _MODULE_NAME_H_ */
