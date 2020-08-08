/*
 * web_server.h
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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs.h"

#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "spiffs.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

//#define USE_GZIP

#define AP_WIFI_SSID		"EMMON"
#define AP_WIFI_PASS		"mauriciobarroso"
#define AP_MAX_STA_CONN		10

#define ESP_MAX_RETRY		3

#define WIFI_CONNECTED_BIT	BIT0
#define WIFI_FAIL_BIT      	BIT1

/*==================[typedef]================================================*/

typedef struct
{
	char wifi_sta[ 32 ];
	char wifi_ap[ 32 ];
} wifi_t;

typedef struct
{
	uint16_t port;
	uint16_t max_uri_handlers;
	spiffs_t settings;
} web_server_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void web_server_init( web_server_t * const me );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _WEBS_INTERFACE_H_ */
