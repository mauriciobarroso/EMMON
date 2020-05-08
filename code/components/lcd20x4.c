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

#include "lcd20x4.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

static const uint8_t line_addr[] = { LINE_1_ADDR, LINE_2_ADDR, LINE_3_ADDR, LINE_4_ADDR };
static const uint8_t lcd_lines = 4;
static const hd44780_font_t lcd_font = HD44780_FONT_5X8;

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static esp_err_t pcf8574_write( uint8_t data );
static void hd44780_write_nibble( uint8_t byte, bool rs, bool bl );
static void hd44780_write_byte( uint8_t byte, bool rs, bool bl );

/*==================[external functions definition]=========================*/

void lcd_init( void )
{
    // switch to 4 bit mode
    for ( uint8_t i = 0; i < 3; i ++ )
    {
    	hd44780_write_nibble( ( CMD_FUNC_SET | ARG_FS_8_BIT ) >> 4, false, true );
        init_delay();
    }

    hd44780_write_nibble( CMD_FUNC_SET >> 4, false, true );
    short_delay();

    // Specify the number of display lines and character font
    hd44780_write_byte( CMD_FUNC_SET |
    					( lcd_lines > 1 ? ARG_FS_2_LINES : 0 ) |
						( lcd_font == HD44780_FONT_5X10 ? ARG_FS_FONT_5X10 : 0 ), false, true );
    short_delay();
    // Display off
    lcd_control( false, false, false );
    // Clear
    lcd_clear();
    // Entry mode set
    hd44780_write_byte( CMD_ENTRY_MODE | ARG_EM_INCREMENT, false, true );
    short_delay();
    // Display on
    lcd_control( true, false, false );
}

void lcd_control( bool on, bool cursor, bool cursor_blink )
{
	hd44780_write_byte( CMD_DISPLAY_CTRL |
						( on ? ARG_DC_DISPLAY_ON : 0) |
						( cursor ? ARG_DC_CURSOR_ON : 0) |
						( cursor_blink ? ARG_DC_CURSOR_BLINK : 0 ), false, true );
    short_delay();
}

void lcd_clear( void )
{
	hd44780_write_byte( CMD_CLEAR, false, true );
	long_delay();
}

void lcd_gotoxy( uint8_t col, uint8_t line)
{
	hd44780_write_byte( CMD_DDRAM_ADDR + line_addr[line - 1] + col - 1, false, true );
    short_delay();
}

void lcd_putc( char character )
{
	hd44780_write_byte( character, true, true );
    short_delay();
}

void lcd_puts( const char * string )
{
    while ( * string )
    {
        lcd_putc( * string );
        string++;
    }
}

/*==================[internal functions definition]=========================*/

static esp_err_t pcf8574_write( uint8_t data )
{
	int ret;

	ret = i2c_write_reg0( PCF8574_ADDR, &data, 1 );

    return ret;
}

static void hd44780_write_nibble( uint8_t byte, bool rs, bool bl )
{
	uint8_t data = ( ( ( byte >> 3 ) & 1 ) << 7 ) |
				   ( ( ( byte >> 2 ) & 1 ) << 6 ) |
				   ( ( ( byte >> 1 ) & 1 ) << 5 ) |
				   ( ( byte & 1 ) << 4 ) |
				   ( rs ? 1 : 0 ) |
				   ( bl ? 1 << 3 : 0);
    //CHECK(lcd->write_cb(data | (1 << lcd->pins.e)));
	pcf8574_write( data | 1 << 2 );
    toggle_delay();
    //CHECK(lcd->write_cb(data));
    pcf8574_write( data );
}

static void hd44780_write_byte( uint8_t byte, bool rs, bool bl )
{
	hd44780_write_nibble( byte >> 4, rs, bl);
	hd44780_write_nibble( byte, rs, bl );

}

/*==================[end of file]============================================*/
