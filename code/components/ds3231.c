/* Copyright 2020, Mauricio Barroso
 * All rights reserved.
 *
 * This file is part of arquitecturaDeMicroprocesadores.
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

/* Date: 11/02/20 */

/*==================[inlcusions]============================================*/

#include <ds3231.h>

/*==================[macros]=================================================*/

/* Device REGess */
#define DS3231_ADDR			0x68	/*!< slave REGess of DS3231 RTC */

/* Timekeeping Registers */
#define SECONDS_REG			0x0		/*!< Seconds register REGess of DS3231 RTC */
#define MINUTES_REG			0x1		/*!< Minutes register REGess of DS3231 RTC */
#define HOURS_REG			0x2		/*!< Hours register REGess of DS3231 RTC */
#define DAY_REG				0x3		/*!< Day register REGess of DS3231 RTC */
#define DATE_REG 			0x4		/*!< Date register REGess of DS3231 RTC */
#define MONTH_REG			0x5		/*!< Month register REGess of DS3231 RTC */
#define YEAR_REG			0x6		/*!< Year register REGess of DS3231 RTC */
#define ALARM1_SECONDS_REG	0x7		/*!< Alarm 1 Seconds register REGess of DS3231 RTC */
#define ALARM1_MINUTES_REG	0x8		/*!< Alarm 1 Minutes register REGess of DS3231 RTC */
#define ALARM1_HOURS_REG	0x9		/*!< Alarm 1 Hours register REGess of DS3231 RTC */
#define ALARM1_DAYDATE_REG	0xA		/*!< Alarm 1 Day/Date register REGess of DS3231 RTC */
#define ALARM2_MINUTES_REG	0xB		/*!< Alarm 2 Minutes register REGess of DS3231 RTC */
#define ALARM2_HOURS_REG	0xC		/*!< Alarm 2 Hours register REGess of DS3231 RTC */
#define ALARM2_DAYDATE_REG	0xD		/*!< Alarm 2 Day/Date register REGess of DS3231 RTC */
#define CONTROL_REG			0xE		/*!< Control register REGess of DS3231 RTC */
#define CONTROL_STATUS_REG	0xF		/*!< Control/Status register REGess of DS3231 RTC */
#define AGING_OFFSET_REG	0x10	/*!< Aging Offfset register REGess of DS3231 RTC */
#define MSB_TEMP_REG		0x11	/*!< MSB of Temp register REGess of DS3231 RTC */
#define LSB_TEMP_REG		0x12	/*!< LSB of Temp register REGess of DS3231 RTC */

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]=========================*/

esp_err_t ds3231_get_time( ds3231_t * const me )
{
	esp_err_t ret;
	uint8_t data[ 3 ];

	ret = i2c_read_reg8( DS3231_ADDR, SECONDS_REG, data, sizeof( data ) );

	me->time.seconds = data[ 0 ];
	me->time.minutes = data[ 1 ];
	me->time.hours = data[ 2 ];

	return ret;
}

esp_err_t ds3231_get_date( ds3231_t * const me )
{
	esp_err_t ret;
	uint8_t data[ 4 ];

	ret = i2c_read_reg8( DS3231_ADDR, DAY_REG, data, sizeof( data ) );

	me->date.day = data[ 0 ];
	me->date.date = data[ 1 ];
	me->date.month = data[ 2 ];
	me->date.year = data[ 3 ];

	return ret;
}

esp_err_t ds3231_set_time( ds3231_t * const me )
{
	esp_err_t ret;
	uint8_t data[ 3 ];

	data[ 0 ] = me->time.seconds;
	data[ 1 ] = me->time.minutes;
	data[ 2 ] = me->time.hours;

	ret = i2c_write_reg8( DS3231_ADDR, SECONDS_REG, data, sizeof( data ) );

	return ret;
}

esp_err_t ds3231_set_date( ds3231_t * const me )
{
	esp_err_t ret;
	uint8_t data[ 4 ];

	data[ 0 ] = me->date.day;
	data[ 1 ] = me->date.date;
	data[ 2 ] = me->date.month;
	data[ 3 ] = me->date.year;

	ret = i2c_write_reg8( DS3231_ADDR, DAY_REG, data, sizeof( data ) );

	return ret;
}

esp_err_t ds3231_get_alarm( ds3231_t * const me, ds3231_alarm_number_t number )
{
	esp_err_t ret;

	if( number == ALARM1 )
		ret = i2c_read_reg8( DS3231_ADDR, ALARM1_SECONDS_REG, ( uint8_t *)&me->alarm1, 4 );
	else
		ret = i2c_read_reg8( DS3231_ADDR, ALARM1_SECONDS_REG, ( uint8_t *)&me->alarm2, 3 );

	return ret;
}

esp_err_t ds3231_set_alarm( ds3231_t * const me, ds3231_alarm_number_t number )
{
	esp_err_t ret;

	if( number == ALARM1 )
	{
		uint8_t data[ 4 ];
		data[ 0 ] = me->alarm1.seconds | ( ( me->alarm1.mode & 0x1 ) << 7 );
		data[ 1 ] = me->alarm1.minutes | ( ( me->alarm1.mode & 0x2 ) << 6 );
		data[ 2 ] = me->alarm1.hours | ( ( me->alarm1.mode & 0x4 ) << 5 );
		data[ 3 ] = me->alarm1.daydate | ( ( me->alarm1.mode & 0x8 ) << 4 );
		//data[ 3 ] = me->alarm1.daydate | ( ( me->alarm1.mode & 0x10 ) << 2 );

		ret = i2c_write_reg8( DS3231_ADDR, ALARM1_SECONDS_REG, data, 4 );
	}
	else
	{
		uint8_t data[ 3 ];

		data[ 0 ] = me->alarm2.minutes | ( ( me->alarm2.mode << 7 ) & 0x80 );
		data[ 1 ] = me->alarm2.hours | ( ( me->alarm2.mode << 6 ) & 0x80 );
		data[ 2 ] = me->alarm2.daydate | ( ( me->alarm2.mode << 5 ) & 0x80 );
		//data[ 2 ] = me->alarm2.daydate | ( ( me->alarm2.mode << 3 ) & 0x40 );

		ret = i2c_write_reg8( DS3231_ADDR, ALARM2_MINUTES_REG, data, 3 );
	}

	return ret;
}

esp_err_t ds3231_set_alarm_interrupt( ds3231_t * const me )
{
	esp_err_t ret;
	uint8_t data;

	data = me->alarm_interrupt_mode;

	ret = i2c_write_reg8( DS3231_ADDR, CONTROL_REG, &data, 1 );

	return ret;
}
/*
esp_err_t ds3231_set_sqw_output( ds3231_sqw_output_t mode )
{
	esp_err_t ret;
	uint8_t data;

	data = mode;

	ret = i2c_write_reg8( DS3231_ADDR, CONTROL_REG, &data, 1 );

	return ret;
}*/

void ds3231_get_control( uint8_t *data )
{
	i2c_read_reg8( DS3231_ADDR, CONTROL_REG, data, 1 );
}

void ds3231_get_control_status( uint8_t *data )
{
	i2c_read_reg8( DS3231_ADDR, CONTROL_STATUS_REG, data, 1 );
}

bool ds3231_get_alarm_flag( uint8_t alarm )
{
	uint8_t data;

	ds3231_get_control_status( &data );

	if( alarm == 1 )
	{
		if( !( data & 0x1 ) )
			return false;
	}
	else
	{
		if( !( data & 0x2 ) )
			return false;
	}

	return true;
}

void ds3231_clear_alarm_flag( uint8_t alarm )
{
	uint8_t data;

	ds3231_get_control_status( &data );

	if( alarm == 1 )
		data &= 0xFE;
	else
		data &= 0xFD;

	i2c_write_reg8( DS3231_ADDR, CONTROL_STATUS_REG, &data, 1 );
}

/*==================[internal functions definition]==========================*/

/*==================[end of file]============================================*/
