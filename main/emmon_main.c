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

#include "data_logger.h"
#include "data_transmission.h"
#include "web_interface.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

typedef struct
{
	data_logger_t data_logger;
	web_interface_t web_interface;
	data_transmission_t data_transmission;
} app_t;

/*==================[data declaration]================================================*/

app_t data;

/*==================[function declaration ]================================================*/

/*========================[main]=============================================*/

void app_main ()
{

	if( data_transmission_init ( &data.data_transmission ) )
	{
		data.data_logger.queue = data.data_transmission.queue;
		data_logger_init ( &data.data_logger );
		web_interface_init( &data.data_logger );
	}
}

/*==================[function definition ]================================================*/
