/*
 * eeprom.c
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

/*==================[inlcusions]============================================*/

#include "eeprom.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

static const char * TAG = "eeprom";

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static esp_err_t i2c_init( void );
static esp_err_t i2c_write_reg16( uint8_t device_address, uint16_t reg_address, uint8_t *data, size_t data_len );
static esp_err_t i2c_read_reg16( uint8_t device_address, uint16_t reg_address, uint8_t *data, size_t data_len );

/*==================[external functions definition]=========================*/

void eeprom_init( void )
{
	i2c_init();
}

uint8_t eeprom_read8( uint16_t address, uint8_t *data )
{
	if( address > EEPROM_SIZE - sizeof( uint8_t ) )
		return false;

	i2c_read_reg16( EEPROM_ADDR, address, data, 1 );

	return true;
}

uint8_t eeprom_read16( uint16_t address, uint16_t *data )
{
	if( address > EEPROM_SIZE - sizeof( uint16_t ) )
		return false;

	uint8_t data8[ 2 ];

	i2c_read_reg16( EEPROM_ADDR, address, data8, 2 );

	*data = ( uint16_t )( data8[ 0 ] << 8 ) |
			 ( uint16_t )data8[ 1 ];

	return true;
}

uint8_t eeprom_read32( uint16_t address, uint32_t *data )
{
	if( address > EEPROM_SIZE - sizeof( uint32_t ) )
		return false;

	uint8_t data8[ 4 ];

	i2c_read_reg16( EEPROM_ADDR, address, data8, 4 );

	*data = ( uint32_t )( data8[ 0 ] << 24 ) |
			 ( uint32_t )( data8[ 1 ] << 16 ) |
			 ( uint32_t )( data8[ 2 ] << 8 ) |
			 ( uint32_t )data8[ 3 ];

	return true;
}

uint8_t eeprom_write8( uint16_t address, uint8_t *data )
{
	if( address > EEPROM_SIZE - sizeof( uint8_t ) )
		return false;

	i2c_write_reg16( EEPROM_ADDR, address, data, 1 );

	return true;
}

uint8_t eeprom_write16( uint16_t address, uint16_t *data )
{
	if( address > EEPROM_SIZE - sizeof( uint16_t ) )
		return false;

	uint8_t data8[ 2 ];

	data8[ 0 ] = ( uint8_t )( ( *data & 0xFF00 ) >> 8 );
	data8[ 1 ] = ( uint8_t )( *data & 0xFF );

	i2c_write_reg16( EEPROM_ADDR, address, &data8[ 0 ], 1 );
	i2c_write_reg16( EEPROM_ADDR, address + 1, &data8[ 1 ], 1 );

	return true;
}

uint8_t eeprom_write32( uint16_t address, uint32_t *data )
{
	if( address > EEPROM_SIZE - sizeof( uint32_t ) )
		return false;

	uint8_t data8[ 4 ];

	data8[ 0 ] = ( uint8_t )( ( *data & 0xFF000000 ) >> 24 );
	data8[ 1 ] = ( uint8_t )( ( *data & 0x00FF0000 ) >> 16 );
	data8[ 2 ] = ( uint8_t )( ( *data & 0x0000FF00 ) >> 8 );
	data8[ 3 ] = ( uint8_t )( *data & 0x000000FF );

	i2c_write_reg16( EEPROM_ADDR, address, &data8[ 0 ], 1 );
	i2c_write_reg16( EEPROM_ADDR, address + 1, &data8[ 1 ], 1 );
	i2c_write_reg16( EEPROM_ADDR, address + 2, &data8[ 2 ], 1 );
	i2c_write_reg16( EEPROM_ADDR, address + 3, &data8[ 3 ], 1 );

	return true;
}

/*==================[internal functions definition]==========================*/


static esp_err_t i2c_init( void )
{
	ESP_LOGI( TAG, "Configuring pins I2C interface" );

    int i2c_master_port = I2C_NUM_0;
    i2c_config_t conf;

    conf.mode = I2C_MODE_MASTER;
    ESP_LOGI( TAG, "I2C in master mode" );
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.
    ESP_ERROR_CHECK( i2c_driver_install( i2c_master_port, conf.mode ) );
    ESP_LOGI( TAG, "Installed I2C driver" );
    ESP_ERROR_CHECK( i2c_param_config( i2c_master_port, &conf ));
    ESP_LOGI( TAG, "Configured I2C parameters" );

    ESP_LOGI( TAG, "I2C interface configured!" );

    return ESP_OK;
}

static esp_err_t i2c_write_reg16( uint8_t device_address, uint16_t reg_address, uint8_t *data, size_t data_len )
{
	int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start( cmd ); // start condition
    i2c_master_write_byte( cmd, device_address << 1 | WRITE_BIT, ACK_CHECK_EN ); // device address
    i2c_master_write_byte( cmd, ( uint8_t )( ( reg_address & 0xFF00 ) >> 8 ), ACK_CHECK_EN ); // first word address
    i2c_master_write_byte( cmd, ( uint8_t )( reg_address & 0x00FF ), ACK_CHECK_EN ); // second word address
   	i2c_master_write( cmd, data, data_len, ACK_CHECK_EN );
   	i2c_master_stop( cmd ); // stop condition
    ret = i2c_master_cmd_begin( I2C_NUM_0, cmd, pdMS_TO_TICKS( 1000 ) );
    i2c_cmd_link_delete( cmd );
    write_cycle_delay();

    return ret;
}

static esp_err_t i2c_read_reg16( uint8_t device_address, uint16_t reg_address, uint8_t *data, size_t data_len )
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte( cmd, device_address << 1 | WRITE_BIT, ACK_CHECK_EN ); // device address
	i2c_master_write_byte( cmd, ( uint8_t )( ( reg_address & 0xFF00 ) >> 8 ), ACK_CHECK_EN ); // second word address
	i2c_master_write_byte( cmd, ( uint8_t )( reg_address & 0x00FF ), ACK_CHECK_EN ); // first word address
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin( I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if( ret != ESP_OK )
    	return ret;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte( cmd, device_address << 1 | READ_BIT, ACK_CHECK_EN ); // device address
    i2c_master_read(cmd, data, data_len, LAST_NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin( I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

/*==================[end of file]============================================*/
