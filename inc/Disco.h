/*============================================================================
 * Licencia:
 * Autor:
 * Fecha:
 *===========================================================================*/

#ifndef _DISCO_H_
#define _DISCO_H_

/*==================[inclusiones]============================================*/

/*==================[c++]====================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/
#define STRLENGTHMAX 20

/*==================[tipos de datos declarados por el usuario]===============*/
typedef struct{
	uint8_t str[STRLENGTHMAX];
	bool print;
}Txt2Disk_t;

/*==================[declaraciones de datos externos]========================*/

/*==================[declaraciones de funciones externas]====================*/
/*==================[Prototipos]====================*/
void DiskLog(void* taskParmPtr);

/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_H_ */
