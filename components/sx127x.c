/*==================[inlcusions]============================================*/

#include "sx127x.h"

/*==================[macros]=================================================*/

/* registers */
#define REG_FIFO                 	0x00
#define REG_OP_MODE              	0x01
#define REG_FRF_MSB              	0x06
#define REG_FRF_MID              	0x07
#define REG_FRF_LSB              	0x08
#define REG_PA_CONFIG            	0x09
#define REG_OCP                  	0x0b
#define REG_LNA                  	0x0c
#define REG_FIFO_ADDR_PTR        	0x0d
#define REG_FIFO_TX_BASE_ADDR    	0x0e
#define REG_FIFO_RX_BASE_ADDR    	0x0f
#define REG_FIFO_RX_CURRENT_ADDR 	0x10
#define REG_IRQ_FLAGS            	0x12
#define REG_RX_NB_BYTES          	0x13
#define REG_PKT_SNR_VALUE        	0x19
#define REG_PKT_RSSI_VALUE       	0x1a
#define REG_MODEM_CONFIG_1       	0x1d
#define REG_MODEM_CONFIG_2       	0x1e
#define REG_PREAMBLE_MSB         	0x20
#define REG_PREAMBLE_LSB         	0x21
#define REG_PAYLOAD_LENGTH       	0x22
#define REG_MODEM_CONFIG_3       	0x26
#define REG_FREQ_ERROR_MSB       	0x28
#define REG_FREQ_ERROR_MID       	0x29
#define REG_FREQ_ERROR_LSB       	0x2a
#define REG_RSSI_WIDEBAND        	0x2c
#define REG_DETECTION_OPTIMIZE   	0x31
#define REG_INVERTIQ             	0x33
#define REG_DETECTION_THRESHOLD  	0x37
#define REG_SYNC_WORD            	0x39
#define REG_INVERTIQ2            	0x3b
#define REG_DIO_MAPPING_1        	0x40
#define REG_VERSION              	0x42
#define REG_PA_DAC               	0x4d

/* modes */
#define MODE_LONG_RANGE_MODE     	0x80
#define MODE_SLEEP               	0x00
#define MODE_STDBY               	0x01
#define MODE_TX                  	0x03
#define MODE_RX_CONTINUOUS       	0x05
#define MODE_RX_SINGLE           	0x06

/* PA config */
#define PA_BOOST                 	0x80

/* IRQ masks */
#define IRQ_TX_DONE_MASK           	0x08
#define IRQ_PAYLOAD_CRC_ERROR_MASK	0x20
#define IRQ_RX_DONE_MASK           	0x40

/* max packet length */
#define MAX_PKT_LENGTH				255

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

static long __frequency;
static int __packet_index;
static int __implicit_header_mode;
//static void ( *__onReceive )( int );
static void ( *__onTxDone )( void );

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/
/* read and write registers */
static void write_reg( uint8_t reg, uint8_t value );
static uint8_t read_reg( uint8_t reg );

/* header mode */
static void explicit_header_mode( void );
static void implicit_header_mode( void );

//static void lora_handleDio0Rise( void );
static bool is_transmitting( void );

static int get_spreading_factor( void );
static long get_signal_bandwidth( void );

static void set_ldo_flag( void );

/*==================[external functions definition]=========================*/

int lora_begin( long frequency )
{
	/* reset lora */
    if (SX127X_RESET_PIN != -1) {
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = SX127X_RESET_PIN_MASK;
        io_conf.pull_down_en = 0;
        io_conf.pull_up_en = 0;
        gpio_config( &io_conf );

        // perform reset
        gpio_set_level( SX127X_RESET_PIN, 0 );
        vTaskDelay( pdMS_TO_TICKS( 10 ) );
        gpio_set_level( SX127X_RESET_PIN, 1 );
        vTaskDelay( pdMS_TO_TICKS( 10 ) );
    }

    /* start SPI */
    spi_config_t spi_config;

    spi_config.interface.val = SPI_DEFAULT_INTERFACE;
    spi_config.interface.bit_tx_order = 0;
    spi_config.interface.byte_tx_order = 1;
    spi_config.interface.bit_rx_order = 0;
    spi_config.interface.byte_rx_order = 0;

    spi_config.intr_enable.val = SPI_MASTER_DEFAULT_INTR_ENABLE;

    spi_config.mode = SPI_MASTER_MODE;

    spi_config.clk_div = SPI_8MHz_DIV;
    spi_config.event_cb = NULL;
    spi_init(HSPI_HOST, &spi_config);

    /* check version */
    uint8_t version = read_reg( REG_VERSION );
    if ( version != 0x12 )
    	return 0;

    /* put in sleep mode */
    lora_sleep();

    /* set frequency */
    lora_set_frequency( frequency );

    /* set base addresses */
    write_reg( REG_FIFO_TX_BASE_ADDR, 0 );
    write_reg( REG_FIFO_RX_BASE_ADDR, 0 );

    /* set LNA boost */
    write_reg( REG_LNA, read_reg( REG_LNA ) | 0x03 );

    /* set auto AGC */
    write_reg( REG_MODEM_CONFIG_3, 0x04 );

    /* set output power to 17 dBm */
    lora_set_tx_power( 17, PA_OUTPUT_PA_BOOST_PIN );

    /* put in standby mode */
    lora_idle();

    return 1;
}

void lora_end()
{
  /* put in sleep mode */
  lora_sleep();
}

int lora_begin_packet( int implicitHeader )
{
    if ( is_transmitting() )
    	return 0;

    /* put in standby mode */
    lora_idle();

    if ( implicitHeader )
    	implicit_header_mode();
    else
    	explicit_header_mode();

    /* reset FIFO address and paload length */
    write_reg( REG_FIFO_ADDR_PTR, 0 );
    write_reg( REG_PAYLOAD_LENGTH, 0 );

    return 1;
}

int lora_end_packet( bool async )
{

    if ( ( async ) && ( __onTxDone ) )
        write_reg( REG_DIO_MAPPING_1, 0x40 ); // DIO0 => TXDONE

    /* put in TX mode */
    write_reg( REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX );

    if ( !async )
    {
    	/* wait for TX done */
        while ( ( read_reg( REG_IRQ_FLAGS ) & IRQ_TX_DONE_MASK ) == 0)
        	vTaskDelay( pdMS_TO_TICKS( 1 ) );

        /* clear IRQ's */
        write_reg( REG_IRQ_FLAGS, IRQ_TX_DONE_MASK );
    }

    return 1;
}

int lora_parse_packet( int size )
{
    int packet_length = 0;
    int irq_flags = read_reg( REG_IRQ_FLAGS );

    if( size > 0 )
    {
        implicit_header_mode();
        write_reg(REG_PAYLOAD_LENGTH, size & 0xff);
    }
    else
    	explicit_header_mode();

    /* clear IRQ's */
    write_reg(REG_IRQ_FLAGS, irq_flags);

    if( ( irq_flags & IRQ_RX_DONE_MASK ) && ( irq_flags & IRQ_PAYLOAD_CRC_ERROR_MASK ) == 0)
    {

		/* received a packet */
		__packet_index = 0;

		/* read packet length */
		if ( __implicit_header_mode )
			packet_length = read_reg( REG_PAYLOAD_LENGTH );
		else
			packet_length = read_reg( REG_RX_NB_BYTES );

		/* set FIFO address to current RX address */
		write_reg( REG_FIFO_ADDR_PTR, read_reg( REG_FIFO_RX_CURRENT_ADDR ) );

		/* put in standby mode */
		lora_idle();
    }
    else if( read_reg( REG_OP_MODE ) != ( MODE_LONG_RANGE_MODE | MODE_RX_SINGLE ) )
    {
		/* not currently in RX mode */

		/* reset FIFO address */
		write_reg( REG_FIFO_ADDR_PTR, 0 );

		// put in single RX mode
		write_reg( REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_SINGLE );
    }

    return packet_length;
}

int lora_packet_rssi( void )
{
    return ( read_reg(REG_PKT_RSSI_VALUE ) - ( __frequency < 868E6 ? 164 : 157 ) );
}

float lora_packet_snr( void )
{
    return ( ( int8_t )read_reg( REG_PKT_SNR_VALUE ) ) * 0.25;
}
/*
long lora_packetFrequencyError()
{
    int32_t freqError = 0;
    freqError = static_cast<int32_t>(read_reg(REG_FREQ_ERROR_MSB) & B111);
    freqError <<= 8L;
    freqError += static_cast<int32_t>(read_reg(REG_FREQ_ERROR_MID));
    freqError <<= 8L;
    freqError += static_cast<int32_t>(read_reg(REG_FREQ_ERROR_LSB));

    if (read_reg(REG_FREQ_ERROR_MSB) & B1000) { // Sign bit is on
        freqError -= 524288; // B1000'0000'0000'0000'0000
    }

    const float fXtal = 32E6; // FXOSC: crystal oscillator (XTAL) frequency (2.5. Chip Specification, p. 14)
    const float fError = ((static_cast<float>(freqError) * (1L << 24)) / fXtal) * (getSignalBandwidth() / 500000.0f); // p. 37

    return static_cast<long>(fError);
}*/

size_t lora_write( const uint8_t *buffer, size_t size )
{
	int current_length = read_reg( REG_PAYLOAD_LENGTH );

    /* check size */
    if( ( current_length + size ) > MAX_PKT_LENGTH )
    	size = MAX_PKT_LENGTH - current_length;

    /* write data */
    for ( size_t i = 0; i < size; i++ )
    	write_reg( REG_FIFO, buffer[ i ] );

    /* update length */
    write_reg( REG_PAYLOAD_LENGTH, current_length + size );

    return size;
}

int lora_available( void )
{
    return ( read_reg( REG_RX_NB_BYTES ) - __packet_index );
}

int lora_read( void )
{
    if ( !lora_available() )
    	return -1;

    __packet_index++;

    return read_reg( REG_FIFO );
}

int lora_peek( void )
{
    if ( !lora_available() )
    	return -1;

    /* store current FIFO address */
    int current_address = read_reg( REG_FIFO_ADDR_PTR );

    /* read */
    uint8_t b = read_reg( REG_FIFO );

    /* restore FIFO address */
    write_reg( REG_FIFO_ADDR_PTR, current_address );

    return b;
}

void lora_flush( void )
{
}

void lora_receive( int size )
{
	/* DIO0 => RXDONE */
    write_reg( REG_DIO_MAPPING_1, 0x00 );

    if ( size > 0 )
    {
    	implicit_header_mode();
        write_reg( REG_PAYLOAD_LENGTH, size & 0xff );
    }
    else
    	explicit_header_mode();

    write_reg( REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS );
}

void lora_idle( void )
{
    write_reg( REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY );
}

void lora_sleep( void )
{
    write_reg( REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP );
}

void lora_set_tx_power(int level, int output_pin)
{
    if( PA_OUTPUT_RFO_PIN == output_pin )
    {
		/* RFO */
		if( level < 0 )
			level = 0;
		else if( level > 14 )
			level = 14;

		write_reg( REG_PA_CONFIG, 0x70 | level );
    }
    else
    {
        /* PA BOOST */
        if ( level > 17 )
        {
            if ( level > 20 )
            	level = 20;

            /* subtract 3 from level, so 18 - 20 maps to 15 - 17 */
            level -= 3;

            /* High Power +20 dBm Operation (Semtech SX1276/77/78/79 5.4.3.) */
            write_reg( REG_PA_DAC, 0x87 );
            lora_set_ocp( 140 );
        }
        else
        {
            if ( level < 2 )
            	level = 2;

            /* Default value PA_HF/LF or +17dBm */
            write_reg( REG_PA_DAC, 0x84 );
            lora_set_ocp( 100 );
        }

    write_reg( REG_PA_CONFIG, PA_BOOST | ( level - 2 ) );
    }
}

void lora_set_frequency( long frequency )
{
    __frequency = frequency;

    uint64_t frf = ( ( uint64_t )frequency << 19 ) / 32000000;

    write_reg(REG_FRF_MSB, ( uint8_t )( frf >> 16 ) );
    write_reg(REG_FRF_MID, ( uint8_t )( frf >> 8 ) );
    write_reg(REG_FRF_LSB, ( uint8_t )( frf >> 0 ) );
}

void lora_set_spreading_factor( int sf )
{
    if ( sf < 6 )
    	sf = 6;
	else if ( sf > 12 )
		sf = 12;

    if (sf == 6)
    {
        write_reg( REG_DETECTION_OPTIMIZE, 0xc5 );
        write_reg( REG_DETECTION_THRESHOLD, 0x0c );
    }
    else
    {
        write_reg( REG_DETECTION_OPTIMIZE, 0xc3 );
        write_reg( REG_DETECTION_THRESHOLD, 0x0a );
    }

    write_reg( REG_MODEM_CONFIG_2, ( read_reg( REG_MODEM_CONFIG_2 ) & 0x0f ) | ( ( sf << 4 ) & 0xf0 ) );
    set_ldo_flag();
}

void lora_set_signal_bandwidth( long sbw )
{
    int bw;

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
    	/*if (sbw <= 250E3)*/
    	bw = 9;

    write_reg( REG_MODEM_CONFIG_1, ( read_reg( REG_MODEM_CONFIG_1 ) & 0x0f ) | ( bw << 4 ) );
    set_ldo_flag();
}

void lora_set_coding_rate4( int denominator )
{
    if( denominator < 5 )
    	denominator = 5;
    else if( denominator > 8 )
    	denominator = 8;

    int cr = denominator - 4;

    write_reg( REG_MODEM_CONFIG_1, ( read_reg( REG_MODEM_CONFIG_1 ) & 0xf1 ) | ( cr << 1 ) );
}

void lora_set_preamble_length( long length )
{
    write_reg( REG_PREAMBLE_MSB, ( uint8_t )( length >> 8 ) );
    write_reg( REG_PREAMBLE_LSB, ( uint8_t )( length >> 0 ) );
}

void lora_set_sync_word( int sw )
{
    write_reg( REG_SYNC_WORD, sw );
}

void lora_enable_crc( void )
{
    write_reg( REG_MODEM_CONFIG_2, read_reg( REG_MODEM_CONFIG_2 ) | 0x04 );
}

void lora_disable_crc( void )
{
    write_reg( REG_MODEM_CONFIG_2, read_reg( REG_MODEM_CONFIG_2 ) & 0xfb );
}

void lora_enable_invert_iq( void )
{
    write_reg( REG_INVERTIQ,  0x66 );
    write_reg( REG_INVERTIQ2, 0x19 );
}

void lora_disable_invert_iq( void )
{
    write_reg( REG_INVERTIQ,  0x27 );
    write_reg( REG_INVERTIQ2, 0x1d );
}

void lora_set_ocp(uint8_t current)
{
    uint8_t ocp_trim = 27;

    if( current <= 120 )
    	ocp_trim = ( current - 45 ) / 5;
    else if( current <=240 )
    	ocp_trim = ( current + 30 ) / 10;

    write_reg( REG_OCP, 0x20 | ( 0x1F & ocp_trim ) );
}

uint8_t lora_random( void )
{
    return read_reg( REG_RSSI_WIDEBAND );
}


/*
void lora_handleDio0Rise()
{
    int irqFlags = read_reg(REG_IRQ_FLAGS);

    // clear IRQ's
    write_reg(REG_IRQ_FLAGS, irqFlags);

        if ((irqFlags & IRQ_PAYLOAD_CRC_ERROR_MASK) == 0) {

        if ((irqFlags & IRQ_RX_DONE_MASK) != 0) {
            // received a packet
            __packet_index = 0;

            // read packet length
            int packetLength = __implicit_header_mode ? read_reg(REG_PAYLOAD_LENGTH) : read_reg(REG_RX_NB_BYTES);

            // set FIFO address to current RX address
            write_reg(REG_FIFO_ADDR_PTR, read_reg(REG_FIFO_RX_CURRENT_ADDR));

            if (_onReceive) {
                _onReceive(packetLength);
            }
        }
        else if ((irqFlags & IRQ_TX_DONE_MASK) != 0) {
            if (_onTxDone) {
                _onTxDone();
            }
        }
    }
}*/

/*==================[internal functions definition]==========================*/

static void write_reg( uint8_t reg, uint8_t value )
{
	spi_trans_t trans;
	uint32_t addr_buf = (uint32_t)( ( reg | 0x80 ) << 24);
	uint32_t mosi_buf = (uint32_t)( value << 24 );

	memset( &trans, 0x00, sizeof( trans ) );
	trans.bits.val = 0;

	trans.bits.mosi = 8;
	trans.mosi = &mosi_buf;

	trans.bits.addr = 8;
	trans.addr = &addr_buf;

	ESP_ERROR_CHECK( spi_trans( HSPI_HOST, &trans ) );
}

static uint8_t read_reg( uint8_t reg )
{
	uint32_t value = 0x0;
	uint32_t addr_buf = (uint32_t)( ( reg & 0x7F ) << 24 );
	spi_trans_t trans;

	memset( &trans, 0x00, sizeof( trans ) );
	trans.bits.val = 0;

	trans.bits.miso = 8;
	trans.miso = &value;

	trans.bits.addr = 8;
	trans.addr = &addr_buf;

	ESP_ERROR_CHECK( spi_trans( HSPI_HOST, &trans ) );
	return ( uint8_t )( value );
}

static void explicit_header_mode( void )
{
    __implicit_header_mode = 0;

    write_reg( REG_MODEM_CONFIG_1, read_reg( REG_MODEM_CONFIG_1 ) & 0xfe );
}

static void implicit_header_mode( void )
{
    __implicit_header_mode = 1;

    write_reg( REG_MODEM_CONFIG_1, read_reg( REG_MODEM_CONFIG_1 ) | 0x01 );
}

static bool is_transmitting( void )
{
    if ( ( read_reg( REG_OP_MODE ) & MODE_TX ) == MODE_TX )
    	return true;

    if ( read_reg( REG_IRQ_FLAGS ) & IRQ_TX_DONE_MASK )
    	// clear IRQ's
        write_reg( REG_IRQ_FLAGS, IRQ_TX_DONE_MASK );

    return false;
}

static int get_spreading_factor( void )
{
    return read_reg( REG_MODEM_CONFIG_2 ) >> 4;
}

static long get_signal_bandwidth( void )
{
    uint8_t bw = ( read_reg( REG_MODEM_CONFIG_1 ) >> 4 );

    switch ( bw )
    {
        case 0:
        	return 7.8E3;

        case 1:
        	return 10.4E3;

        case 2:
        	return 15.6E3;

        case 3:
        	return 20.8E3;

        case 4:
        	return 31.25E3;

        case 5:
        	return 41.7E3;

        case 6:
        	return 62.5E3;

        case 7:
        	return 125E3;

        case 8:
        	return 250E3;

        case 9:
        	return 500E3;
    }

    return -1;
}

static void set_ldo_flag( void )
{
    /* Section 4.1.1.5 */
    long symbol_duration = 1000 / ( get_signal_bandwidth() / ( 1L << get_spreading_factor() ) ) ;

    /* Section 4.1.1.6 */
    bool ldo_on = symbol_duration > 16;

    uint8_t config3 = read_reg( REG_MODEM_CONFIG_3 );

    if( ldo_on )
    	config3 |= 0x80;
    else
    	config3 &= 0x7F;

    write_reg( REG_MODEM_CONFIG_3, config3 );
}

/*==================[end of file]============================================*/
