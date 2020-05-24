/*
 * web_interface.c
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

#ifndef _SPIFFS_H_
#define _SPIFFS_H_

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

esp_err_t spiffs_init( void );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _SPIFFS_H_ */
