/* Copyright 2019, Mauricio Barroso
 * All rights reserved.
 *
 * This file is part of EMMON.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* Date: 10/12/19 */

#ifndef _DS3231_H_
#define _DS3231_H_

/*==================[inclusions]=============================================*/

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/
#ifndef I2C_MASTER_SCL_IO
#define I2C_MASTER_SCL_IO						2					/* gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO						0					/* gpio number for I2C master data  */
#define I2C_MASTER_NUM             			 	I2C_NUM_0			/* I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE  			 	0					/* I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   			0					/* I2C master do not need buffer */
#endif

#ifndef WRITE_BIT
#define WRITE_BIT           					I2C_MASTER_WRITE	/*!< I2C master write */
#define READ_BIT            					I2C_MASTER_READ  	/*!< I2C master read */
#define ACK_CHECK_EN        					0x1              	/*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS       					0x0              	/*!< I2C master will not check ack from slave */
#define ACK_VAL             					0x0              	/*!< I2C ack value */
#define NACK_VAL            					0x1              	/*!< I2C nack value */
#define LAST_NACK_VAL       					0x2              	/*!< I2C last_nack value */
#endif
/* Timekeeping Registers */
#define DS3231_ADDR								0x68				/*!< slave address of DS3231 RTC */
#define DS3231_SECONDS_ADDR						0x0					/*!< Seconds register address of DS3231 RTC */
#define DS3231_MINUTES_ADDR						0x1					/*!< Minutes register address of DS3231 RTC */
#define DS3231_HOURS_ADDR						0x2					/*!< Hours register address of DS3231 RTC */
#define DS3231_DAY_ADDR							0x3					/*!< Day register address of DS3231 RTC */
#define DS3231_DATE_ADDR 						0x4					/*!< Date register address of DS3231 RTC */
#define DS3231_MONTH_ADDR						0x5					/*!< Month register address of DS3231 RTC */
#define DS3231_YEAR_ADDR						0x6					/*!< Year register address of DS3231 RTC */
#define DS3231_ALARM1_SECONDS_ADDR				0x7					/*!< Alarm 1 Seconds register address of DS3231 RTC */
#define DS3231_ALARM1_MINUTES_ADDR				0x8					/*!< Alarm 1 Minutes register address of DS3231 RTC */
#define DS3231_ALARM1_HOURS_ADDR				0x9					/*!< Alarm 1 Hours register address of DS3231 RTC */
#define DS3231_ALARM1_DAYDATE_ADDR				0xA					/*!< Alarm 1 Day/Date register address of DS3231 RTC */
#define DS3231_ALARM2_MINUTES_ADDR				0xB					/*!< Alarm 2 Minutes register address of DS3231 RTC */
#define DS3231_ALARM2_HOURS_ADDR				0xC					/*!< Alarm 2 Hours register address of DS3231 RTC */
#define DS3231_ALARM2_DAYDATE_ADDR				0xD					/*!< Alarm 2 Day/Date register address of DS3231 RTC */
#define DS3231_CONTROL_ADDR						0xE					/*!< Control register address of DS3231 RTC */
#define DS3231_CONTROL_STATUS_ADDR				0xF					/*!< Control/Status register address of DS3231 RTC */
#define DS3231_AGING_OFFSET_ADDR				0x10				/*!< Aging Offfset register address of DS3231 RTC */
#define DS3231_MSB_TEMP_ADDR					0x11				/*!< MSB of Temp register address of DS3231 RTC */
#define DS3231_LSB_TEMP_ADDR					0x12				/*!< LSB of Temp register address of DS3231 RTC */

/*==================[typedef]================================================*/
/* Square-Wave Output Frequency */
typedef enum
{
	OUTPUT_1HZ = 0x40,		/*!< set SQW/OUT output to 1Hz */
	OUTPUT_1024KHZ = 0x48,	/*!< set SQW/OUT output to 1024KHz */
	OUTPUT_4096KHZ = 0x50,	/*!< set SQW/OUT output to 4096KHz */
	OUTPUT_8192KHZ = 0x58,	/*!< set SQW/OUT output to 8192KHz */
} ds3231_sqw_out_frequency_t;

/* Alarm Interrupt Status */
typedef enum
{
	DISABLE_ALL = 0x4,		/*!< disable interrupt of alarm 1 and alarm 2 */
	ENABLE_ALARM1 = 0x5,	/*!< enable interrupt of alarm 1 */
	ENABLE_ALARM2 = 0x6,	/*!< enable interrupt of alarm 2 */
	ENABLE_ALL = 0x07,		/*!< enable interrupt of alarm 1 and alarm 2 */
} ds3231_alarm_interrupt_mode_t;

/* Alarm Mask Bits */
typedef enum
{
	ONCE_PER_SECOND = 0xF,					/*!< set alarm 1 once per second */
	SECONDS_MATCH = 0xE,					/*!< set alarm 1 when seconds match */
	MINUTES_SECONDS_MATCH = 0xC,			/*!< set alarm 1 when minutes and seconds match */
	HOURS_MINUTES_SECONDS_MATCH = 0x8,		/*!< set alarm 1 when hours, minutes and seconds match */
	DATE_HOURS_MINUTES_SECONDS_MATCH = 0x0,	/*!< set alarm 1 when date, hours, minutes and seconds match */
	DAY_HOURS_MINUTES_SECONDS_MATCH = 0x10,	/*!< set alarm 1 when day, hours, minutes and seconds match */

} ds3231_alarm1_mode_t;

typedef enum
{
	ONCE_PER_MINUTE = 0x7,			/*!< set alarm 2 once per minute */
	MINUTES_MATCH = 0x6,			/*!< set alarm 2 when minutes match */
	HOURS_MINUTES_MATCH = 0x4,		/*!< set alarm 2 when hours and minutes match */
	DATE_HOURS_MINUTES_MATCH = 0x0,	/*!< set alarm 2 when date, hours and minutes */
	DAY_HOURS_MINUTES_MATCH = 0x8,	/*!< set alarm 2 when day, hours and minutes */
} ds3231_alarm2_mode_t;

/*  */

typedef enum
{
	SUNDAY = 1,
	MONDAY = 2,
	TUESDAY = 3,
	WEDNESDAY = 4,
	THURSDAY = 5,
	FRIDAY = 6,
	SATURDAY = 7,
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

extern esp_err_t ds3231_init( ds3231_t *ds3231_data );
extern esp_err_t ds3231_get_time( ds3231_t *ds3231_data );
extern esp_err_t ds3231_get_date( ds3231_t *ds3231_data );
extern esp_err_t ds3231_set_time( ds3231_t *ds3231_data );
extern esp_err_t ds3231_set_date( ds3231_t *ds3231_data );
extern esp_err_t ds3231_get_alarm1( ds3231_t *ds3231_data );
//extern esp_err_t ds3231_get_alarm2( ds3231_t *ds3231_data );
extern esp_err_t ds3231_set_alarm1( ds3231_t *ds3231_data );
extern esp_err_t ds3231_set_alarm2( ds3231_t *ds3231_data );
extern esp_err_t ds3231_set_alarm_interrupt( ds3231_t *ds3231_data );
//extern esp_err_t ds3231_set_sqw_output( Ds3231_t *xDs3231Config );
void get_control( uint8_t *data );
void get_control_status( uint8_t *data );
bool get_alarm1_flag( void );
void clear_alarm1_flag( void );
/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _DS3231_H_ */
