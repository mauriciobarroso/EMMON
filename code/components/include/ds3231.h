/*
 * ds3231.h
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

#ifndef _DS3231_H_
#define _DS3231_H_

/*==================[inclusions]=============================================*/

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c.h"
#include "i2c_conf.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/



/*==================[typedef]================================================*/

/* Square-Wave Output Frequency */
typedef enum
{
	OUTPUT_1HZ = 0x40,		/*!< set SQW/OUT output to 1Hz */
	OUTPUT_1024KHZ = 0x48,	/*!< set SQW/OUT output to 1024KHz */
	OUTPUT_4096KHZ = 0x50,	/*!< set SQW/OUT output to 4096KHz */
	OUTPUT_8192KHZ = 0x58,	/*!< set SQW/OUT output to 8192KHz */
} ds3231_sqw_out_frequency_t;

/* Alarm numbers */
typedef enum
{
	ALARM1,	/*! Alarm 1 number */
	ALARM2,	/*! Alarm 2 number */
} ds3231_alarm_number_t;

/* Alarm Interrupt Status */
typedef enum
{
	DISABLE_ALL = 0x4,		/*!< disable interrupt of alarm 1 and alarm 2 */
	ENABLE_ALARM1 = 0x5,	/*!< enable interrupt of alarm 1 */
	ENABLE_ALARM2 = 0x6,	/*!< enable interrupt of alarm 2 */
	ENABLE_ALL = 0x07,		/*!< enable interrupt of alarm 1 and alarm 2 */
} ds3231_alarm_interrupt_mode_t;

/* Alarm 1 modes */
typedef enum
{
	ONCE_PER_SECOND = 0xF,					/*!< set alarm 1 once per second */
	SECONDS_MATCH = 0xE,					/*!< set alarm 1 when seconds match */
	MINUTES_SECONDS_MATCH = 0xC,			/*!< set alarm 1 when minutes and seconds match */
	HOURS_MINUTES_SECONDS_MATCH = 0x8,		/*!< set alarm 1 when hours, minutes and seconds match */
	DATE_HOURS_MINUTES_SECONDS_MATCH = 0x0,	/*!< set alarm 1 when date, hours, minutes and seconds match */
	DAY_HOURS_MINUTES_SECONDS_MATCH = 0x10,	/*!< set alarm 1 when day, hours, minutes and seconds match */

} ds3231_alarm1_mode_t;

/* Alarm 2 modes */
typedef enum
{
	ONCE_PER_MINUTE = 0x7,			/*!< set alarm 2 once per minute */
	MINUTES_MATCH = 0x6,			/*!< set alarm 2 when minutes match */
	HOURS_MINUTES_MATCH = 0x4,		/*!< set alarm 2 when hours and minutes match */
	DATE_HOURS_MINUTES_MATCH = 0x0,	/*!< set alarm 2 when date, hours and minutes */
	DAY_HOURS_MINUTES_MATCH = 0x8,	/*!< set alarm 2 when day, hours and minutes */
} ds3231_alarm2_mode_t;

/* days of the week */
typedef enum
{
	SUNDAY = 1,		/*! first day of week */
	MONDAY = 2,		/*! second day of week */
	TUESDAY = 3,	/*! third day of week */
	WEDNESDAY = 4,	/*! fourth day of week */
	THURSDAY = 5,	/*! fifth day of week */
	FRIDAY = 6,		/*! sixth day of week */
	SATURDAY = 7,	/*! seventh day of week */
} ds3231_day_t;

typedef struct
{
	uint8_t seconds;	/*!< seconds of RTC*/
	uint8_t minutes;	/*!< minutes of RTC */
	uint8_t hours;		/*!< hours of RTC */
} ds3231_time_t;

typedef struct
{
	ds3231_day_t day;	/*!< day of RTC */
	uint8_t date;		/*!< date of RTC */
	uint8_t month;		/*!< month of RTC */
	uint8_t year;		/*!< year of RTC */
} ds3231_date_t;

typedef struct
{
	uint8_t seconds;			/*!< seconds of alarm 1 */
	uint8_t minutes;			/*!< minutes of alarm 1 */
	uint8_t hours;				/*!< horus of alarm 1 */
	uint8_t daydate;			/*!< day or date of alarm 1 */
	ds3231_alarm1_mode_t mode;	/*!< mode of alarm 1 */
} ds3231_alarm1_t;

typedef struct
{
	uint8_t minutes;			/*!< minutes of alarm 2 */
	uint8_t hours;				/*!< hours of alarm 2 */
	uint8_t daydate;			/*!< day or date of alarm 2 */
	ds3231_alarm2_mode_t mode;	/*!< mode of alarm 2 */
} ds3231_alarm2_t;

typedef struct
{
	ds3231_time_t time;
	ds3231_date_t date;
	ds3231_alarm1_t alarm1;
	ds3231_alarm2_t alarm2;
	ds3231_sqw_out_frequency_t sqw_out;
	ds3231_alarm_interrupt_mode_t alarm_interrupt_mode;
} ds3231_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

esp_err_t ds3231_init( ds3231_t * const me );
esp_err_t ds3231_get_time( ds3231_t * const me );
esp_err_t ds3231_get_date( ds3231_t * const me );
esp_err_t ds3231_set_time( ds3231_t * const me );
esp_err_t ds3231_set_date( ds3231_t * const me );
esp_err_t ds3231_get_alarm( ds3231_t * const me, ds3231_alarm_number_t number );
esp_err_t ds3231_set_alarm( ds3231_t * const me, ds3231_alarm_number_t number );
esp_err_t ds3231_set_alarm_interrupt( ds3231_t * const me );
esp_err_t ds3231_set_sqw_output( ds3231_sqw_out_frequency_t mode );
void ds3231_get_control( uint8_t *data );
void ds3231_get_control_status( uint8_t *data );
bool ds3231_get_alarm_flag( uint8_t alarm );
void ds3231_clear_alarm_flag( uint8_t alarm );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _DS3231_H_ */
