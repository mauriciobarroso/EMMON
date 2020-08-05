/*
 * web_interface.h
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

#ifndef _WEBS_INTERFACE_H_
#define _WEBS_INTERFACE_H_

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "esp_err.h"
#include "esp_log.h"

#include "esp_vfs.h"
#include "esp_http_server.h"

#include "wifi.h"

#include "data_logger.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

//#define USE_GZIP

/*==================[typedef]================================================*/

typedef struct
{
	uint16_t port;
	uint16_t max_uri_handlers;

} web_interface_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void web_interface_init( data_logger_t * const me );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _WEBS_INTERFACE_H_ */
