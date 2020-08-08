/*
 * rtc.h
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

#ifndef _DS3231_H_
#define _DS3231_H_

/*==================[inclusions]=============================================*/

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "esp_err.h"

#include "esp_system.h"
#include "esp_log.h"

#include "driver/i2c.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

#ifndef I2C_MASTER_SCL_IO
#define I2C_MASTER_SCL_IO			2	 			/* gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO           0				/* gpio number for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0		/* I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE   0				/* I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0				/* I2C master do not need buffer */
#endif

#define WRITE_BIT                   I2C_MASTER_WRITE /* I2C master write */
#define READ_BIT                    I2C_MASTER_READ  /* I2C master read */
#define ACK_CHECK_EN                0x1              /* I2C master will check ack from slave*/
#define ACK_CHECK_DIS               0x0              /* I2C master will not check ack from slave */
#define ACK_VAL                     0x0              /* I2C ack value */
#define NACK_VAL                    0x1              /* I2C nack value */
#define LAST_NACK_VAL               0x2              /* I2C last_nack value */

/*==================[typedef]================================================*/

/* Square-Wave Output Frequency */
typedef enum
{
	OUTPUT_1HZ = 0x40,		/*!< set SQW/OUT output to 1Hz */
	OUTPUT_1024KHZ = 0x48,	/*!< set SQW/OUT output to 1024KHz */
	OUTPUT_4096KHZ = 0x50,	/*!< set SQW/OUT output to 4096KHz */
	OUTPUT_8192KHZ = 0x58,	/*!< set SQW/OUT output to 8192KHz */
} rtc_sqw_out_frequency_t;

/* Alarm numbers */
typedef enum
{
	ALARM1,	/*! Alarm 1 number */
	ALARM2,	/*! Alarm 2 number */
} rtc_alarm_number_t;

/* Alarm Interrupt Status */
typedef enum
{
	DISABLE_ALL = 0x4,		/*!< disable interrupt of alarm 1 and alarm 2 */
	ENABLE_ALARM1 = 0x5,	/*!< enable interrupt of alarm 1 */
	ENABLE_ALARM2 = 0x6,	/*!< enable interrupt of alarm 2 */
	ENABLE_ALL = 0x07,		/*!< enable interrupt of alarm 1 and alarm 2 */
} rtc_alarm_interrupt_mode_t;

/* Alarm 1 modes */
typedef enum
{
	ONCE_PER_SECOND = 0xF,					/*!< set alarm 1 once per second */
	SECONDS_MATCH = 0xE,					/*!< set alarm 1 when seconds match */
	MINUTES_SECONDS_MATCH = 0xC,			/*!< set alarm 1 when minutes and seconds match */
	HOURS_MINUTES_SECONDS_MATCH = 0x8,		/*!< set alarm 1 when hours, minutes and seconds match */
	DATE_HOURS_MINUTES_SECONDS_MATCH = 0x0,	/*!< set alarm 1 when date, hours, minutes and seconds match */
	DAY_HOURS_MINUTES_SECONDS_MATCH = 0x10,	/*!< set alarm 1 when day, hours, minutes and seconds match */

} rtc_alarm1_mode_t;

/* Alarm 2 modes */
typedef enum
{
	ONCE_PER_MINUTE = 0x7,			/*!< set alarm 2 once per minute */
	MINUTES_MATCH = 0x6,			/*!< set alarm 2 when minutes match */
	HOURS_MINUTES_MATCH = 0x4,		/*!< set alarm 2 when hours and minutes match */
	DATE_HOURS_MINUTES_MATCH = 0x0,	/*!< set alarm 2 when date, hours and minutes */
	DAY_HOURS_MINUTES_MATCH = 0x8,	/*!< set alarm 2 when day, hours and minutes */
} rtc_alarm2_mode_t;

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
} rtc_day_t;

typedef struct
{
	uint8_t seconds;	/*!< seconds of RTC*/
	uint8_t minutes;	/*!< minutes of RTC */
	uint8_t hours;		/*!< hours of RTC */
} rtc_time_t;

typedef struct
{
	rtc_day_t day;	/*!< day of RTC */
	uint8_t date;		/*!< date of RTC */
	uint8_t month;		/*!< month of RTC */
	uint8_t year;		/*!< year of RTC */
} rtc_date_t;

typedef struct
{
	uint8_t seconds;			/*!< seconds of alarm 1 */
	uint8_t minutes;			/*!< minutes of alarm 1 */
	uint8_t hours;				/*!< horus of alarm 1 */
	uint8_t daydate;			/*!< day or date of alarm 1 */
	rtc_alarm1_mode_t mode;	/*!< mode of alarm 1 */
} rtc_alarm1_t;

typedef struct
{
	uint8_t minutes;			/*!< minutes of alarm 2 */
	uint8_t hours;				/*!< hours of alarm 2 */
	uint8_t daydate;			/*!< day or date of alarm 2 */
	rtc_alarm2_mode_t mode;	/*!< mode of alarm 2 */
} rtc_alarm2_t;

typedef struct
{
	rtc_time_t time;
	rtc_date_t date;
	rtc_alarm1_t alarm1;
	rtc_alarm2_t alarm2;
	rtc_sqw_out_frequency_t sqw_out;
	rtc_alarm_interrupt_mode_t alarm_interrupt_mode;
} rtc_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void rtc_init( void );
esp_err_t rtc_get_time( rtc_t * const me );
esp_err_t rtc_get_date( rtc_t * const me );
esp_err_t rtc_set_time( rtc_t * const me );
esp_err_t rtc_set_date( rtc_t * const me );
esp_err_t rtc_get_alarm( rtc_t * const me, rtc_alarm_number_t number );
esp_err_t rtc_set_alarm( rtc_t * const me, rtc_alarm_number_t number );
esp_err_t rtc_set_alarm_interrupt( rtc_t * const me );
esp_err_t rtc_set_sqw_output( rtc_sqw_out_frequency_t mode );
void rtc_get_control( uint8_t *data );
void rtc_get_control_status( uint8_t *data );
bool rtc_get_alarm_flag( uint8_t alarm );
void rtc_clear_alarm_flag( uint8_t alarm );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _DS3231_H_ */
