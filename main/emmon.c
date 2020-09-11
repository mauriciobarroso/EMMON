/*
 * emmon.c
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/unistd.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "data_logger.h"
#include "web_server.h"
#include "data_communication.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[data declaration]================================================*/

/* tag for debug */
//static const char * TAG = "emmon";

/* data for modules */
data_logger_t data_logger;				/*!< user id */
web_server_t web_server;				/*!< user id */
data_transmission_t data_transmission;	/*!< user id */

/*==================[function declaration ]================================================*/

/*========================[main]=============================================*/

void app_main ()
{
	spiffs_init();

	spiffs_get_settings( &data_logger.settings );
	spiffs_get_settings( &web_server.settings );
	spiffs_get_settings( &data_transmission.settings );

	web_server_init( &web_server );
	data_logger_init( &data_logger );
	data_transmission_init( &data_transmission );

	data_logger.queue = data_transmission.queue;
}

/*==================[function definition ]================================================*/
