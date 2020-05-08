/*
 * emmon_main.c
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

#include "dataLogger.h"
#include "webServer.h"
#include "dataTransmission.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[data declaration]================================================*/

data_logger_t data_logger;

/*==================[function declaration ]================================================*/

/*========================[main]=============================================*/

void app_main()
{
	//web_interface_init();
	data_logger_init( &data_logger );
	data_transmission_init( &data_logger.transmission );

    /* el scheduler es iniciado en las tareas de inicialización del esp8266 */
}

/*==================[function definition ]================================================*/
