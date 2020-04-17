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

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static esp_err_t ds3231_write_reg( i2c_port_t i2c_num, uint8_t reg_address, uint8_t * data, size_t data_len );
static esp_err_t ds3231_read_reg( i2c_port_t i2c_num, uint8_t reg_address, uint8_t * data, size_t data_len );
static esp_err_t i2c_init();

/*==================[external functions definition]=========================*/

extern esp_err_t ds3231_init( ds3231_t *ds3231_data )
{
	esp_err_t ret;

	ret = i2c_init();

	return ret;
}

extern esp_err_t ds3231_get_time( ds3231_t *ds3231_data )
{
	esp_err_t ret;
	uint8_t data[ 3 ];

	ret = ds3231_read_reg( I2C_MASTER_NUM, DS3231_SECONDS_ADDR, data, sizeof( data ) );

	ds3231_data->time.seconds = data[ 0 ];
	ds3231_data->time.minutes = data[ 1 ];
	ds3231_data->time.hours = data[ 2 ];

	return ret;
}

extern esp_err_t ds3231_get_date( ds3231_t *ds3231_data )
{
	esp_err_t ret;
	uint8_t data[ 4 ];

	ret = ds3231_read_reg( I2C_MASTER_NUM, DS3231_DAY_ADDR, data, sizeof( data ) );

	ds3231_data->date.day = data[ 0 ];
	ds3231_data->date.date = data[ 1 ];
	ds3231_data->date.month = data[ 2 ];
	ds3231_data->date.year = data[ 3 ];

	return ret;
}

extern esp_err_t ds3231_set_time( ds3231_t *ds3231_data )
{
	esp_err_t ret;
	uint8_t data[ 3 ];

	data[ 0 ] = ds3231_data->time.seconds;
	data[ 1 ] = ds3231_data->time.minutes;
	data[ 2 ] = ds3231_data->time.hours;

	ret = ds3231_write_reg( I2C_MASTER_NUM, DS3231_SECONDS_ADDR, data, sizeof( data ) );

	return ret;
}

extern esp_err_t ds3231_set_date( ds3231_t *ds3231_data )
{
	esp_err_t ret;
	uint8_t data[ 4 ];

	data[ 0 ] = ds3231_data->date.day;
	data[ 1 ] = ds3231_data->date.date;
	data[ 2 ] = ds3231_data->date.month;
	data[ 3 ] = ds3231_data->date.year;

	ret = ds3231_write_reg( I2C_MASTER_NUM, DS3231_DAY_ADDR, data, sizeof( data ) );

	return ret;
}

extern esp_err_t ds3231_get_alarm1( ds3231_t *ds3231_data )
{
	esp_err_t ret;

	ret = ds3231_read_reg( I2C_MASTER_NUM, DS3231_ALARM1_SECONDS_ADDR, ( uint8_t *)&ds3231_data->alarm1, 4 );


	return ret;
}
/*
extern esp_err_t ds3231_get_alarm2( ds3231_alarm2_t * alarm )
{
	esp_err_t ret;
	uint8_t data_alarm [ 4 ];

	ret = ds3231_read_reg( I2C_MASTER_NUM, DS3231_ALARM2_MINUTES_ADDR, data_alarm, 4 );

	alarm->minutes = data_alarm[ 0 ];
	alarm->hours = data_alarm[ 1 ];
	alarm->dydt = data_alarm[ 2 ];

	return ret;
}
*/
extern esp_err_t ds3231_set_alarm1( ds3231_t *ds3231_data )
{
	esp_err_t ret;
	uint8_t data[ 4 ];

	data[ 0 ] = ds3231_data->alarm1.seconds | ( ( ds3231_data->alarm1.mode & 0x1 ) << 7 );
	data[ 1 ] = ds3231_data->alarm1.minutes | ( ( ds3231_data->alarm1.mode & 0x2 ) << 6 );
	data[ 2 ] = ds3231_data->alarm1.hours | ( ( ds3231_data->alarm1.mode & 0x4 ) << 5 );
	data[ 3 ] = ds3231_data->alarm1.daydate | ( ( ds3231_data->alarm1.mode & 0x8 ) << 4 );
	//data[ 3 ] = ds3231_data->alarm1.daydate | ( ( ds3231_data->alarm1.mode & 0x10 ) << 2 );

	ret = ds3231_write_reg( I2C_MASTER_NUM, DS3231_ALARM1_SECONDS_ADDR, data, 4 );

	return ret;
}

extern esp_err_t ds3231_set_alarm2( ds3231_t *ds3231_data )
{
	esp_err_t ret;
	uint8_t data_alarm[ 3 ];

	data_alarm[ 0 ] = ds3231_data->alarm2.minutes | ( ( ds3231_data->alarm2.mode << 7 ) & 0x80 );
	data_alarm[ 1 ] = ds3231_data->alarm2.hours | ( ( ds3231_data->alarm2.mode << 6 ) & 0x80 );
	data_alarm[ 2 ] = ds3231_data->alarm2.daydate | ( ( ds3231_data->alarm2.mode << 5 ) & 0x80 );
	data_alarm[ 2 ] = ds3231_data->alarm2.daydate | ( ( ds3231_data->alarm2.mode << 3 ) & 0x40 );

	ret = ds3231_write_reg( I2C_MASTER_NUM, DS3231_ALARM2_MINUTES_ADDR, data_alarm, 4 );

	return ret;
}

extern esp_err_t ds3231_set_alarm_interrupt( ds3231_t *ds3231_data )
{
	esp_err_t ret;
	uint8_t data;

	data = ds3231_data->alarm_interrupt_mode;

	ret = ds3231_write_reg( I2C_MASTER_NUM, DS3231_CONTROL_ADDR, &data, 1 );

	return ret;
}
/*
extern esp_err_t ds3231_set_sqw_output( ds3231_sqw_output_t mode )
{
	esp_err_t ret;
	uint8_t data_mode;

	data_mode = mode;

	ret = ds3231_write_reg( I2C_MASTER_NUM, DS3231_CONTROL_ADDR, &data_mode, 1 );

	return ret;
} */

void get_control( uint8_t *data )
{
	ds3231_read_reg( I2C_MASTER_NUM, DS3231_CONTROL_ADDR, data, 1 );
}

void get_control_status( uint8_t *data )
{
	ds3231_read_reg( I2C_MASTER_NUM, DS3231_CONTROL_STATUS_ADDR, data, 1 );
}

bool get_alarm1_flag( void )
{
	uint8_t data;

	get_control_status( &data );

	if( !( data & 0x1 ) )
		return false;

	return true;
}

void clear_alarm1_flag( void )
{
	uint8_t data;

	get_control_status( &data );

	data &= 0xFE;

	ds3231_write_reg( I2C_MASTER_NUM, DS3231_CONTROL_STATUS_ADDR, &data, 1 );
}

/*==================[internal functions definition]==========================*/

static esp_err_t ds3231_write_reg( i2c_port_t i2c_num, uint8_t reg_address, uint8_t * data, size_t data_len )
{
	int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start( cmd );
    i2c_master_write_byte( cmd, DS3231_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN );
    i2c_master_write_byte( cmd, reg_address, ACK_CHECK_EN );
    i2c_master_write( cmd, data, data_len, ACK_CHECK_EN );
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin( i2c_num, cmd, 1000 / portTICK_RATE_MS );
    i2c_cmd_link_delete( cmd );

    return ret;
}

static esp_err_t ds3231_read_reg( i2c_port_t i2c_num, uint8_t reg_address, uint8_t * data, size_t data_len )
{
	int ret;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start( cmd );
	i2c_master_write_byte( cmd, DS3231_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN );
	i2c_master_write_byte( cmd, reg_address, ACK_CHECK_EN );
	i2c_master_stop( cmd );
	ret = i2c_master_cmd_begin( i2c_num, cmd, 1000 / portTICK_RATE_MS );
	i2c_cmd_link_delete( cmd );

	if ( ret != ESP_OK )
		return ret;

	cmd = i2c_cmd_link_create();
	i2c_master_start( cmd );
	i2c_master_write_byte( cmd, DS3231_ADDR << 1 | READ_BIT, ACK_CHECK_EN );
	i2c_master_read( cmd, data, data_len, LAST_NACK_VAL );
	i2c_master_stop( cmd );
	ret = i2c_master_cmd_begin( i2c_num, cmd, 1000 / portTICK_RATE_MS );
	i2c_cmd_link_delete( cmd );

	return ret;
}

static esp_err_t i2c_init()
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.
    ESP_ERROR_CHECK( i2c_driver_delete( i2c_master_port ) );
    ESP_ERROR_CHECK( i2c_driver_install( i2c_master_port, conf.mode ) );
    ESP_ERROR_CHECK( i2c_param_config( i2c_master_port, &conf ));

    return ESP_OK;
}

/*==================[end of file]============================================*/
