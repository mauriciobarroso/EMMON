/*
 * data_transmission.h
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

#ifndef _DATATRANSMISSION_H_
#define _DATATRANSMISSION_H_

/*==================[inclusions]=============================================*/

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "queue.h"

#include "sx127x.h"
#include "spiffs.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

#define QUEUE_LENGTH	4
#define GATEWAY_ADDR	0x0
#define HOST_ADDR		0x12345678
#define BROADCAST_ADDR	0xFFFFFFFF

/*==================[typedef]================================================*/

typedef struct
{
	QueueHandle_t queue;		/*!< packet receive queue */
	spiffs_t settings;			/*!< data from settings.txt */
} data_transmission_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

bool data_transmission_init( data_transmission_t * const me );

void send_to_gateway( void * arg );	/* cambiar nombre! */
/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _DATATRANSMISSION_H_ */
