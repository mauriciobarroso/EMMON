/*
 * spiffs.h
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

typedef struct
{
	int frequency;			/*!< data transmission frequency */
	float pulses_to_kwh;	/*!< constant to convert pulses to kwh */
	char wifi_data[32];		/*!< wifi ssid and password */
	int id;					/*!< user id */
} spiffs_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

esp_err_t spiffs_init( void );

void spiffs_get_settings( spiffs_t * const me );

void spiffs_set_settings( spiffs_t * const me );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _SPIFFS_H_ */
