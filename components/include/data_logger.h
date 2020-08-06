/*
 * data_logger.h
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

#ifndef _DATALOGGER_H_
#define _DATALOGGER_H_

/*==================[inclusions]=============================================*/

#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "queue.h"

#include "esp_system.h"
#include "esp_log.h"

#include "driver/gpio.h"

#include "i2c_conf.h"
#include "at24cx.h"
#include "ds3231.h"
#include "spiffs.h"
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

typedef struct
{
	ds3231_t rtc;				/*!< rtc data */
	uint16_t pulses;			/*!< daily counted pulses */
	uint16_t logged_days;		/*!< quantity of logged days */
	uint16_t index;				/*!< eeprom index */
	QueueHandle_t queue;		/*!<  */
	TaskHandle_t pulses_handle;	/*!< pulses_task handle */
	TaskHandle_t alarm_handle;	/*!< alarm task handle */
	spiffs_t settings;			/*!< data from settings.txt */

} data_logger_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Data Logger initialization
 */
void data_logger_init( data_logger_t * const me );

/**
 * @brief Data Logger initialization
 */
void data_loggger_pulses_task( void * arg );

/**
 * @brief Data Logger initialization
 */
void data_loggger_alarm_task( void * arg );

/**
 * @brief Data Logger initialization
 */
void data_logger_get_csv( data_logger_t * const me );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _DATALOGGER_H_ */
