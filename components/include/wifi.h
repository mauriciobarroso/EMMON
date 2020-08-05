/*
 * wifi.c
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

#ifndef _WIFI_H_
#define _WIFI_H_

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

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

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/


#define AP_WIFI_SSID		"EMMON"
#define AP_WIFI_PASS		"mauriciobarroso"
#define AP_MAX_STA_CONN		10

#define ESP_MAX_RETRY		3

#define WIFI_CONNECTED_BIT	BIT0
#define WIFI_FAIL_BIT      	BIT1

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void wifi_init( char * wifi_data );
void wifi_sta_mode( char * buf, size_t len );
void wifi_ap_mode( void );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _WIFI_H_ */
