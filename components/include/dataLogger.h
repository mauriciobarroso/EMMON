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

#include "driver/gpio.h"

#include "esp_system.h"
#include "esp_log.h"

#include "i2c_conf.h"
#include "at24c32.h"
#include "ds3231.h"

#include "dataTransmission.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/*  debug mode*/
#define DEBUG_MESSAGES

/* definicion de pines de interrupcion */
#define GPIO_PULSES			0	/*!< pulses interrupt pin */
#define GPIO_ALARM			3	/*!< alarm interrupt pin */

/*==================[typedef]================================================*/

typedef struct
{
	ds3231_t rtc;						/*!< RTC data */
	data_transmission_t transmission;	/*!< transmission data */
	uint16_t daily_pulses;				/*!< daily counted pulses */
	uint32_t monthly_pulses;			/*!< monthly rquantity monthly of logged days */
	uint16_t monthly_logged_days;		/*!< monthly quantity of logged days */
	uint16_t total_logged_days;			/*!< total quantity of logged days */
	uint32_t id;						/*!< user ID */
	uint16_t index;						/*!< eeprom index data */
} data_logger_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Data Logger initialization
 */
void data_logger_init( data_logger_t * const me );

// implementar una función para borrar eeprom
// implementar una función para obtener log historico

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _DATALOGGER_H_ */
