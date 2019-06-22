/*=====[Control.h]===================================================
 * Copyright 2019 Agustín Rey <agustinrey61@gmail.com>
 * All rights reserved.
 * Licencia: Texto de la licencia o al menos el nombre y un link
         (ejemplo: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Fecha de creacion: 2019/06/12
 */

/*=====[Evitar inclusion multiple comienzo]==================================*/

#ifndef _CONTROL_H_
#define _CONTROL_H_

/*=====[Inclusiones de dependencias de funciones publicas]===================*/

/*=====[C++ comienzo]========================================================*/

#ifdef __cplusplus
extern "C" {
#endif


#include "Consola.h"
/*=====[Macros de definicion de constantes publicas]=========================*/

/*=====[Macros estilo funcion publicas]======================================*/
#define STRCOMMAX 10
/*=====[Definiciones de tipos de datos publicos]=============================*/

// Tipo de datos que renombra un tipo basico

// Tipo de datos de puntero a funcion

// Tipo de datos enumerado
typedef enum {
   UPDATE_FREC,
   START,
   STOP
}TipoCom_t;

// Tipo de datos estructua, union o campo de bits
typedef struct {
	bool marcha;
	uint8_t frecuencia;
} Motor_t;

typedef struct{
	uint8_t str[STRCOMMAX];
	uint8_t frecuencia;
	TipoCom_t tipo;
}Comando_t;


/*=====[Prototipos de funciones publicas]====================================*/
void Control(void* taskParmPtr);

/*=====[Prototipos de funciones publicas de interrupcion]====================*/



/*=====[C++ fin]=============================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Evitar inclusion multiple fin]=======================================*/

#endif /* _CONTROL_H_ */
