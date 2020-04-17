/* Copyright 2019, Mauricio Barroso
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

/* Date: 10/12/19 */

/*==================[inlcusions]============================================*/

#include "at24c32.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]=========================*/

extern esp_err_t i2c_init( void )
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.
    //ESP_ERROR_CHECK( i2c_driver_delete( i2c_master_port ) );
    ESP_ERROR_CHECK( i2c_driver_install( i2c_master_port, conf.mode ) );
    ESP_ERROR_CHECK( i2c_param_config( i2c_master_port, &conf ));

    return ESP_OK;
}

extern esp_err_t at24c32_write( uint16_t reg_address, uint8_t * data )
{
	int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start( cmd ); // start condition
    i2c_master_write_byte( cmd, AT24C32_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN ); // device address
    i2c_master_write_byte( cmd, ( uint8_t )( ( reg_address & 0xFF00 ) >> 8 ), ACK_CHECK_EN ); // first word address
    i2c_master_write_byte( cmd, ( uint8_t )( reg_address & 0x00FF ), ACK_CHECK_EN ); // second word address
   	i2c_master_write( cmd, data, 1, ACK_CHECK_EN );
   	i2c_master_stop( cmd ); // stop condition
    ret = i2c_master_cmd_begin( I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS );
    i2c_cmd_link_delete( cmd );
    write_cycle_delay();

    return ret;
}

extern esp_err_t at24c32_page_write( uint16_t reg_address, uint8_t * data, size_t data_len )
{
	int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start( cmd ); // start condition
    i2c_master_write_byte( cmd, AT24C32_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN ); // device address
    i2c_master_write_byte( cmd, ( uint8_t )( reg_address & 0x00FF ), ACK_CHECK_EN ); // second word address
    i2c_master_write_byte( cmd, ( uint8_t )( ( reg_address & 0xFF00 ) >> 8 ), ACK_CHECK_EN ); // first word address
   	i2c_master_write( cmd, data, data_len, ACK_CHECK_EN );
   	i2c_master_stop( cmd ); // stop condition
    ret = i2c_master_cmd_begin( I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS );
    i2c_cmd_link_delete( cmd );
    write_cycle_delay();

    return ret;
}

/*extern esp_err_t at24c32_current_address_read( i2c_port_t i2c_num, uint8_t * data ){}*/

extern esp_err_t at24c32_read( uint16_t reg_address, uint8_t * data )
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte( cmd, AT24C32_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN ); // device address
	i2c_master_write_byte( cmd, ( uint8_t )( ( reg_address & 0xFF00 ) >> 8 ), ACK_CHECK_EN ); // second word address
	i2c_master_write_byte( cmd, ( uint8_t )( reg_address & 0x00FF ), ACK_CHECK_EN ); // first word address
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin( I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if( ret != ESP_OK )
    	return ret;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte( cmd, AT24C32_ADDR << 1 | READ_BIT, ACK_CHECK_EN ); // device address
    i2c_master_read(cmd, data, 1, LAST_NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin( I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

/*extern esp_err_t at24c32_sequential_read( i2c_port_t i2c_num, uint16_t reg_address, uint8_t * data, size_t data_len ){}*/

/*==================[internal functions definition]==========================*/

/*==================[end of file]============================================*/
