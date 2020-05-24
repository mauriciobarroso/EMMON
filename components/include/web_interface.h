/*
 * web_interface.h
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */


#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

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

#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

#define AP_WIFI_SSID		"EMMON"
#define AP_WIFI_PASS		"123456789"
#define AP_MAX_STA_CONN		10

#define ESP_MAX_RETRY		3

#define WIFI_CONNECTED_BIT	BIT0
#define WIFI_FAIL_BIT      	BIT1

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

esp_err_t start_webserver( void );

/* sta wifi */

void web_interface_init( void );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _WEBSERVER_H_ */
