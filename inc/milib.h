/*=====[milib.h]===================================================
 * Copyright 2019 Agustín Rey <agustinrey61@gmail.com>
 * All rights reserved.
 * Licencia: Texto de la licencia o al menos el nombre y un link
         (ejemplo: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Fecha de creacion: 2019/06/12
 */

/*=====[Evitar inclusion multiple comienzo]==================================*/

#ifndef _MILIB_H_
#define _MILIB_H_

/*=====[Inclusiones de dependencias de funciones publicas]===================*/

/*=====[C++ comienzo]========================================================*/

#ifdef __cplusplus
extern "C" {
#endif


#include "sapi_datatypes.h"
/*=====[Macros de definicion de constantes publicas]=========================*/

/*=====[Macros estilo funcion publicas]======================================*/

// Tipo de datos que renombra un tipo basico

// Tipo de datos de puntero a funcion

// Tipo de datos enumerado

/*=====[Definiciones de tipos de datos publicos]=============================*/


// Tipo de datos estructua, union o campo de bits

/*=====[Prototipos de funciones publicas]====================================*/

char* itoa_dec(int value, char* result);	//entero a string
bool searchforsubstr(char* str, char* substr);	//busca substring dentro de un strin y retorna TRUE si lo encuentra
int my_atoi(char *p);	//convierte string ascii a entero

/*=====[Prototipos de funciones publicas de interrupcion]====================*/



/*=====[C++ fin]=============================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Evitar inclusion multiple fin]=======================================*/

#endif /* _MODULE_NAME_H_ */
