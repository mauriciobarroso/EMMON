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

#ifndef _LCD2004_H_
#define _LCD2004_H_

/*==================[inclusions]=============================================*/

#include <stdint.h>
#include <stdbool.h>
#include "driver/i2c.h"
#include "rom/ets_sys.h"
#include "i2c_conf.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

#define PCF8574_ADDR				0x3F             /* slave address for LCD 20x4 */

#define LINE_1_ADDR					0x0
#define LINE_2_ADDR					0x40
#define LINE_3_ADDR					0x14
#define LINE_4_ADDR					0x54

#define MS							1000

#define BV(x)						( 1 << ( x ) )

#define DELAY_CMD_LONG  			( 3 * MS ) // >1.53ms according to datasheet
#define DELAY_CMD_SHORT 			( 60 )     // >39us according to datasheet
#define DELAY_TOGGLE    			( 1 )      // E cycle time >= 1μs, E pulse width >= 450ns, Data set-up time >= 195ns
#define DELAY_INIT      			( 5 * MS )

#define CMD_CLEAR        			0x01
#define CMD_RETURN_HOME  			0x02
#define CMD_ENTRY_MODE   			0x04
#define CMD_DISPLAY_CTRL 			0x08
#define CMD_SHIFT        			0x10
#define CMD_FUNC_SET     			0x20
#define CMD_CGRAM_ADDR   			0x40
#define CMD_DDRAM_ADDR   			0x80

// CMD_ENTRY_MODE
#define ARG_EM_INCREMENT    		BV(1)
#define ARG_EM_SHIFT        		(1)

// CMD_DISPLAY_CTRL
#define ARG_DC_DISPLAY_ON  			BV( 2 )
#define ARG_DC_CURSOR_ON   		 	BV( 1 )
#define ARG_DC_CURSOR_BLINK 		( 1 )

// CMD_FUNC_SET
#define ARG_FS_8_BIT        		BV( 4 )
#define ARG_FS_2_LINES      		BV( 3 )
#define ARG_FS_FONT_5X10    		BV( 2 )

#define init_delay()   				do { ets_delay_us( DELAY_INIT ); } while ( 0 )
#define short_delay()  				do { ets_delay_us( DELAY_CMD_SHORT ); } while ( 0 )
#define long_delay()   				do { ets_delay_us( DELAY_CMD_LONG ); } while ( 0 )
#define toggle_delay() 				do { ets_delay_us( DELAY_TOGGLE ); } while ( 0 )

/*==================[typedef]================================================*/
typedef enum
{
	HD44780_FONT_5X8 = 0,
	HD44780_FONT_5X10
} hd44780_font_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void lcd_init( void );
void lcd_control( bool on, bool cursor, bool cursor_blink );
void lcd_clear( void );
void lcd_gotoxy( uint8_t col, uint8_t line );
void lcd_putc( char character );
void lcd_puts( const char *string );
/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _LCD2004_H_ */
