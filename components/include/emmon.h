/*
 * data_logger.h
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

#ifndef _EMMON_H_
#define _EMMON_H_

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/unistd.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "data_logger.h"
#include "data_transmission.h"
#include "web_interface.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

typedef struct
{
	data_logger_t data_logger;				/*!< user id */
	web_server_t web_server;				/*!< user id */
	data_transmission_t data_transmission;	/*!< user id */
} emmon_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void emmon_init( emmon_t * const me );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _EMMON_H_ */
