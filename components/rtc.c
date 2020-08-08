/*
 * rtc.c
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */


/*==================[inlcusions]============================================*/

#include <rtc.h>

/*==================[macros]=================================================*/

/* Device REGess */
#define RTC_ADDR			0x68	/*!< slave REGess of RTC RTC */

/* Timekeeping Registers */
#define SECONDS_REG			0x0		/*!< Seconds register REGess of RTC RTC */
#define MINUTES_REG			0x1		/*!< Minutes register REGess of RTC RTC */
#define HOURS_REG			0x2		/*!< Hours register REGess of RTC RTC */
#define DAY_REG				0x3		/*!< Day register REGess of RTC RTC */
#define DATE_REG 			0x4		/*!< Date register REGess of RTC RTC */
#define MONTH_REG			0x5		/*!< Month register REGess of RTC RTC */
#define YEAR_REG			0x6		/*!< Year register REGess of RTC RTC */
#define ALARM1_SECONDS_REG	0x7		/*!< Alarm 1 Seconds register REGess of RTC RTC */
#define ALARM1_MINUTES_REG	0x8		/*!< Alarm 1 Minutes register REGess of RTC RTC */
#define ALARM1_HOURS_REG	0x9		/*!< Alarm 1 Hours register REGess of RTC RTC */
#define ALARM1_DAYDATE_REG	0xA		/*!< Alarm 1 Day/Date register REGess of RTC RTC */
#define ALARM2_MINUTES_REG	0xB		/*!< Alarm 2 Minutes register REGess of RTC RTC */
#define ALARM2_HOURS_REG	0xC		/*!< Alarm 2 Hours register REGess of RTC RTC */
#define ALARM2_DAYDATE_REG	0xD		/*!< Alarm 2 Day/Date register REGess of RTC RTC */
#define CONTROL_REG			0xE		/*!< Control register REGess of RTC RTC */
#define CONTROL_STATUS_REG	0xF		/*!< Control/Status register REGess of RTC RTC */
#define AGING_OFFSET_REG	0x10	/*!< Aging Offfset register REGess of RTC RTC */
#define MSB_TEMP_REG		0x11	/*!< MSB of Temp register REGess of RTC RTC */
#define LSB_TEMP_REG		0x12	/*!< LSB of Temp register REGess of RTC RTC */

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

//static const char * TAG = "rtc";

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

//static esp_err_t i2c_init( void );
static esp_err_t i2c_write_reg8( uint8_t device_address, uint8_t reg_address, uint8_t *data, size_t data_len );
static esp_err_t i2c_read_reg8( uint8_t device_address, uint8_t reg_address, uint8_t *data, size_t data_len );

/*==================[external functions definition]=========================*/
//
//void rtc_init( void )
//{
//
//}

esp_err_t rtc_get_time( rtc_t * const me )
{
	esp_err_t ret;
	uint8_t data[ 3 ];

	ret = i2c_read_reg8( RTC_ADDR, SECONDS_REG, data, sizeof( data ) );

	me->time.seconds = data[ 0 ];
	me->time.minutes = data[ 1 ];
	me->time.hours = data[ 2 ];

	return ret;
}

esp_err_t rtc_get_date( rtc_t * const me )
{
	esp_err_t ret;
	uint8_t data[ 4 ];

	ret = i2c_read_reg8( RTC_ADDR, DAY_REG, data, sizeof( data ) );

	me->date.day = data[ 0 ];
	me->date.date = data[ 1 ];
	me->date.month = data[ 2 ];
	me->date.year = data[ 3 ];

	return ret;
}

esp_err_t rtc_set_time( rtc_t * const me )
{
	esp_err_t ret;
	uint8_t data[ 3 ];

	data[ 0 ] = me->time.seconds;
	data[ 1 ] = me->time.minutes;
	data[ 2 ] = me->time.hours;

	ret = i2c_write_reg8( RTC_ADDR, SECONDS_REG, data, sizeof( data ) );

	return ret;
}

esp_err_t rtc_set_date( rtc_t * const me )
{
	esp_err_t ret;
	uint8_t data[ 4 ];

	data[ 0 ] = me->date.day;
	data[ 1 ] = me->date.date;
	data[ 2 ] = me->date.month;
	data[ 3 ] = me->date.year;

	ret = i2c_write_reg8( RTC_ADDR, DAY_REG, data, sizeof( data ) );

	return ret;
}

esp_err_t rtc_get_alarm( rtc_t * const me, rtc_alarm_number_t number )
{
	esp_err_t ret;

	if( number == ALARM1 )
		ret = i2c_read_reg8( RTC_ADDR, ALARM1_SECONDS_REG, ( uint8_t *)&me->alarm1, 4 );
	else
		ret = i2c_read_reg8( RTC_ADDR, ALARM1_SECONDS_REG, ( uint8_t *)&me->alarm2, 3 );

	return ret;
}

esp_err_t rtc_set_alarm( rtc_t * const me, rtc_alarm_number_t number )
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

		ret = i2c_write_reg8( RTC_ADDR, ALARM1_SECONDS_REG, data, 4 );
	}
	else
	{
		uint8_t data[ 3 ];

		data[ 0 ] = me->alarm2.minutes | ( ( me->alarm2.mode << 7 ) & 0x80 );
		data[ 1 ] = me->alarm2.hours | ( ( me->alarm2.mode << 6 ) & 0x80 );
		data[ 2 ] = me->alarm2.daydate | ( ( me->alarm2.mode << 5 ) & 0x80 );
		//data[ 2 ] = me->alarm2.daydate | ( ( me->alarm2.mode << 3 ) & 0x40 );

		ret = i2c_write_reg8( RTC_ADDR, ALARM2_MINUTES_REG, data, 3 );
	}

	return ret;
}

esp_err_t rtc_set_alarm_interrupt( rtc_t * const me )
{
	esp_err_t ret;
	uint8_t data;

	data = me->alarm_interrupt_mode;

	ret = i2c_write_reg8( RTC_ADDR, CONTROL_REG, &data, 1 );

	return ret;
}
/*
esp_err_t rtc_set_sqw_output( rtc_sqw_output_t mode )
{
	esp_err_t ret;
	uint8_t data;

	data = mode;

	ret = i2c_write_reg8( RTC_ADDR, CONTROL_REG, &data, 1 );

	return ret;
}*/

void rtc_get_control( uint8_t *data )
{
	i2c_read_reg8( RTC_ADDR, CONTROL_REG, data, 1 );
}

void rtc_get_control_status( uint8_t *data )
{
	i2c_read_reg8( RTC_ADDR, CONTROL_STATUS_REG, data, 1 );
}

bool rtc_get_alarm_flag( uint8_t alarm )
{
	uint8_t data;

	rtc_get_control_status( &data );

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

void rtc_clear_alarm_flag( uint8_t alarm )
{
	uint8_t data;

	rtc_get_control_status( &data );

	if( alarm == 1 )
		data &= 0xFE;
	else
		data &= 0xFD;

	i2c_write_reg8( RTC_ADDR, CONTROL_STATUS_REG, &data, 1 );
}

/*==================[internal functions definition]==========================*/


//static esp_err_t i2c_init( void )
//{
//	ESP_LOGI( TAG, "Configuring pins I2C interface" );
//
//    int i2c_master_port = I2C_MASTER_NUM;
//    i2c_config_t conf;
//
//    conf.mode = I2C_MODE_MASTER;
//    ESP_LOGI( TAG, "I2C in master mode" );
//    conf.sda_io_num = I2C_MASTER_SDA_IO;
//    conf.sda_pullup_en = 1;
//    conf.scl_io_num = I2C_MASTER_SCL_IO;
//    conf.scl_pullup_en = 1;
//    conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.
//    ESP_ERROR_CHECK( i2c_driver_install( i2c_master_port, conf.mode ) );
//    ESP_LOGI( TAG, "Installed I2C driver" );
//    ESP_ERROR_CHECK( i2c_param_config( i2c_master_port, &conf ));
//    ESP_LOGI( TAG, "Configured I2C parameters" );
//
//    ESP_LOGI( TAG, "I2C interface configured!" );
//
//    return ESP_OK;
//}

static esp_err_t i2c_write_reg8( uint8_t device_address, uint8_t reg_address, uint8_t *data, size_t data_len )
{
	int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start( cmd ); // start condition
    i2c_master_write_byte( cmd, device_address << 1 | WRITE_BIT, ACK_CHECK_EN ); // device address
    i2c_master_write_byte( cmd, reg_address, ACK_CHECK_EN ); // reg address
   	i2c_master_write( cmd, data, data_len, ACK_CHECK_EN );
   	i2c_master_stop( cmd ); // stop condition
    ret = i2c_master_cmd_begin( I2C_MASTER_NUM, cmd, pdMS_TO_TICKS( 1000 ) );
    i2c_cmd_link_delete( cmd );

    return ret;
}

static esp_err_t i2c_read_reg8( uint8_t device_address, uint8_t reg_address, uint8_t *data, size_t data_len )
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte( cmd, device_address << 1 | WRITE_BIT, ACK_CHECK_EN ); // device address
    i2c_master_write_byte( cmd, reg_address, ACK_CHECK_EN ); // reg address
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin( I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if( ret != ESP_OK )
    	return ret;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte( cmd, device_address << 1 | READ_BIT, ACK_CHECK_EN ); // device address
    i2c_master_read(cmd, data, data_len, LAST_NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin( I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

/*==================[end of file]============================================*/
