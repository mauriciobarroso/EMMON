/*
 * control.h
 *
 * Created on: Sep 11, 2020
 * Author: Mauricio Barroso Benavides
 */

#ifndef _CONTROL_H_
#define _CONTROL_H_

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "data_logger.h"
#include "web_server.h"
#include "data_communication.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/* debug mode */
#define DEBUG_MESSAGES

/* definicion de pines de interrupcion */
#define GPIO_PULSES	4	/*!< pulses interrupt pin */
#define GPIO_ALARM	5	/*!< alarm interrupt pin */

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void control_get_csv( data_logger_t * const me );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _CONTROL_H_ */
