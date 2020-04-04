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

#include "sx127x.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

typedef enum {
    SPI_SEND = 0,
    SPI_RECV
} spi_master_mode_t;

/*==================[internal data declaration]==============================*/

static int __implicit;
static long __frequency;

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static void spi_transmit( spi_master_mode_t trans_mode, uint32_t data, uint32_t addr );
static void sx127x_write_reg( uint32_t reg, uint32_t data );
static uint32_t sx127x_read_reg( uint32_t reg );

/*==================[external functions definition]=========================*/

extern void lora_reset( void )
{
	gpio_set_level( SX127X_RESET_PIN, 0 );
	vTaskDelay( pdMS_TO_TICKS( 1 ) );
	gpio_set_level( SX127X_RESET_PIN, 1 );
	vTaskDelay( pdMS_TO_TICKS( 10 ) );
}

extern void lora_explicit_header_mode( void )
{
	__implicit = 0;

	sx127x_write_reg( SX127X_REG_MODEM_CONFIG_1, sx127x_read_reg( SX127X_REG_MODEM_CONFIG_1 ) & 0xFE );
}

extern void lora_implicit_header_mode( int size )
{
	__implicit = 1;

	sx127x_write_reg( SX127X_REG_MODEM_CONFIG_1, sx127x_read_reg( SX127X_REG_MODEM_CONFIG_1 ) | 0x01 );
	sx127x_write_reg( SX127X_REG_PAYLOAD_LENGTH, size );
}

extern void lora_idle( void )
{
	sx127x_write_reg( SX127X_REG_OP_MODE, LONG_RANGE_MODE | STDBY );
}

extern void lora_sleep( void )
{
	sx127x_write_reg( SX127X_REG_OP_MODE, LONG_RANGE_MODE | SLEEP );
}

extern void lora_receive( void )
{
	sx127x_write_reg( SX127X_REG_OP_MODE, LONG_RANGE_MODE | RX_CONTINUOUS );
}

extern void lora_set_tx_power( int level )
{
	if( level < 2 )
		level = 2;
	else if( level > 17 )
		level = 17;

	sx127x_write_reg( SX127X_REG_PA_CONFIG, PA_BOOST | ( level - 2 ) );
}

extern void lora_set_frequency( long frequency )
{
	__frequency = frequency;

	uint64_t frf = ( ( uint64_t )frequency << 19 ) / 32000000;

	sx127x_write_reg( SX127X_REG_FRF_MSB, ( uint8_t )( frf >> 16 ) );
	sx127x_write_reg( SX127X_REG_FRF_MID, ( uint8_t )( frf >> 8 ) );
	sx127x_write_reg( SX127X_REG_FRF_LSB, ( uint8_t )( frf >> 0 ) );
}

extern void lora_set_spreading_factor( int sf )
{
	if( sf < 6)
		sf = 6;
	else if( sf < 12 )
		sf = 12;

	if( sf == 6 )
	{
		sx127x_write_reg( SX127X_REG_DETECTION_OPTIMIZE, 0xC5);
		sx127x_write_reg( SX127X_REG_DETECTION_THRESHOLD, 0x0C);
	}

	else
	{
		sx127x_write_reg( SX127X_REG_DETECTION_OPTIMIZE, 0xC3);
		sx127x_write_reg( SX127X_REG_DETECTION_THRESHOLD, 0x0A);
	}

	sx127x_write_reg( SX127X_REG_MODEM_CONFIG_2, ( sx127x_read_reg( SX127X_REG_MODEM_CONFIG_2) & 0x0F ) | ( ( sf << 4) & 0xF0 ) );
}

extern void lora_set_bandwidth( long sbw )
{
	uint32_t bw;

	if( sbw <= 7.8E3 )
		bw = 0;
	else if( sbw <= 10.4E3 )
		bw = 1;
	else if( sbw <= 15.6E3 )
		bw = 2;
	else if( sbw <= 20.8E3 )
		bw = 3;
	else if( sbw <= 31.25E3 )
		bw = 4;
	else if( sbw <= 41.7E3 )
		bw = 5;
	else if( sbw <= 62.5E3 )
		bw = 6;
	else if( sbw <= 125E3 )
		bw = 7;
	else if( sbw <= 250E3 )
		bw = 8;
	else
		bw = 9;

	sx127x_write_reg( SX127X_REG_MODEM_CONFIG_1, ( sx127x_read_reg( SX127X_REG_MODEM_CONFIG_1 ) & 0x0F ) | ( bw << 4 ) );
}

extern void lora_set_coding_rate( int denominator )
{
	if( denominator < 5)
		denominator = 5;
	else if( denominator > 8)
		denominator = 8;

	uint32_t cr = denominator - 4;

	sx127x_write_reg( SX127X_REG_MODEM_CONFIG_1, ( sx127x_read_reg( SX127X_REG_MODEM_CONFIG_1 ) & 0xF1 ) | ( cr << 1 ) );
}

extern void lora_set_preamble_length( long length )
{
	sx127x_write_reg( SX127X_REG_PREAMBLE_MSB, ( uint8_t )( length >> 8 ) );
	sx127x_write_reg( SX127X_REG_PREAMBLE_LSB, ( uint8_t )( length >> 0 ) );
}

extern void lora_set_sync_word( int sw )
{
	sx127x_write_reg( SX127X_REG_SYNC_WORD, sw );
}

extern void lora_enable_crc( void )
{
	sx127x_write_reg( SX127X_REG_MODEM_CONFIG_2, sx127x_read_reg( SX127X_REG_MODEM_CONFIG_2 | 0x04 ) );
}

extern void lora_disable_crc( void )
{
	sx127x_write_reg( SX127X_REG_MODEM_CONFIG_2, sx127x_read_reg( SX127X_REG_MODEM_CONFIG_2 | 0xFB ) );
}

extern esp_err_t lora_init( void )
{
	esp_err_t ret;

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = SX127X_RESET_PIN;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    spi_config_t spi_config;
    // Load default interface parameters
    // CS_EN:1, MISO_EN:1, MOSI_EN:1, BYTE_TX_ORDER:1, BYTE_TX_ORDER:1, BIT_RX_ORDER:0, BIT_TX_ORDER:0, CPHA:0, CPOL:0
    spi_config.interface.val = SPI_DEFAULT_INTERFACE;

    // Load default interrupt enable
    // TRANS_DONE: true, WRITE_STATUS: false, READ_STATUS: false, WRITE_BUFFER: false, READ_BUFFER: false
    spi_config.intr_enable.val = SPI_MASTER_DEFAULT_INTR_ENABLE;
    // Set SPI to master mode
    // ESP8266 Only support half-duplex
    spi_config.mode = SPI_MASTER_MODE;
    // Set the SPI clock frequency division factor
    spi_config.clk_div = SPI_10MHz_DIV;
    spi_config.event_cb = NULL;
    ret = spi_init(HSPI_HOST, &spi_config);

    assert( ret == ESP_OK);

    lora_reset();

    uint8_t version;
    uint8_t i = 0;

    while( i++ < TIMEOUT_RESET )
    {
    	version = sx127x_read_reg( SX127X_REG_VERSION );
    	if( version == 0x12 )
    		break;
    	vTaskDelay( 2 );
    }

    assert( i <= TIMEOUT_RESET + 1);

    lora_sleep();
    sx127x_write_reg( SX127X_REG_FIFO_RX_BASE_ADDR, 0 );
    sx127x_write_reg( SX127X_REG_FIFO_TX_BASE_ADDR, 0 );
    sx127x_write_reg( SX127X_REG_LNA, sx127x_read_reg( SX127X_REG_LNA ) | 0x03 );
    sx127x_write_reg( SX127X_REG_MODEM_CONFIG_3, 0x04 );
    lora_set_tx_power( 17 );

    lora_idle();

    return ret;
}

extern void lora_send_packet( uint8_t *buf, int size ) /* uin8_t o uint32_t? */
{
	lora_idle();
	sx127x_write_reg( SX127X_REG_FIFO_ADDR_PTR, 0x00);

	for( uint32_t i = 0; i < size; i++ )
		sx127x_write_reg( SX127X_REG_FIFO, *buf++);

	sx127x_write_reg( SX127X_REG_PAYLOAD_LENGTH, size );

	sx127x_write_reg( SX127X_REG_OP_MODE, LONG_RANGE_MODE | TX );

	while( ( sx127x_read_reg( SX127X_REG_IRQ_FLAGS ) & TX_DONE_MASK ) == 0 )
		vTaskDelay( 2 );

	sx127x_write_reg( SX127X_REG_IRQ_FLAGS, TX_DONE_MASK );
}

extern int lora_receive_packet( uint8_t *buf, int size )
{
	int len = 0;

	int irq = sx127x_read_reg( SX127X_REG_IRQ_FLAGS );
	sx127x_write_reg( SX127X_REG_IRQ_FLAGS, irq );

	if( ( irq & RX_DONE_MASK ) == 0 )
		return 0;
	if( irq & PAYLOAD_CRC_ERROR_MASK )
			return 0;

	if( __implicit )
		len = sx127x_read_reg( SX127X_REG_PAYLOAD_LENGTH );
	else
		len = sx127x_read_reg( SX127X_REG_RX_NB_BYTES );

	lora_idle();
	sx127x_write_reg( SX127X_REG_FIFO_ADDR_PTR, sx127x_read_reg( SX127X_REG_FIFO_RX_CURRENT_ADDR ) );
	if( len > size )
		len = size;
	for( uint32_t i = 0; i < len; i++)
		*buf++ = sx127x_read_reg( SX127X_REG_FIFO );

	return len;
}

extern int lora_received( void )
{
	if( sx127x_read_reg( SX127X_REG_IRQ_FLAGS ) & RX_DONE_MASK )
		return 1;

	return 0;
}

extern int lora_packet_rssi( void )
{
	return ( sx127x_read_reg( SX127X_REG_PKT_RSSI_VALUE ) - ( __frequency < 868E6 ? 164 : 157 ) );
}

extern float lora_packet_snr( void )
{
	return ( ( int8_t )sx127x_read_reg( SX127X_REG_PKT_SNR_VALUE ) ) * 0.25;
}

extern void lora_close( void )
{
	lora_sleep();
}

/*==================[internal functions definition]==========================*/

static void spi_transmit( spi_master_mode_t trans_mode, uint32_t data, uint32_t addr )
{
	spi_trans_t trans;
	uint16_t cmd;

	memset( &trans, 0x00, sizeof( trans ) );
	trans.bits.val = 0;

	if( trans_mode == SPI_SEND )
	{
		cmd = SPI_MASTER_WRITE_DATA_TO_SLAVE_CMD;
		trans.bits.mosi = 8;
		trans.mosi = &data;
	}

	else
	{
		cmd = SPI_MASTER_READ_DATA_FROM_SLAVE_CMD;
		trans.bits.miso = 8;
		trans.miso = &data;
	}

	trans.bits.cmd = 8;
	trans.bits.addr = 8;
	trans.cmd = &cmd;
	trans.addr = &addr;

	spi_trans( HSPI_HOST, &trans );
}

static void sx127x_write_reg( uint32_t reg, uint32_t value )
{
	spi_transmit( SPI_SEND, value, reg  );
}

static uint32_t sx127x_read_reg( uint32_t reg )
{
	uint32_t value = 0x00;

	spi_transmit( SPI_RECV, value, reg  );

	return value;
}

/*==================[end of file]============================================*/
