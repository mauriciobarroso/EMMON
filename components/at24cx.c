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

#include "at24cx.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]=========================*/

uint8_t at24cx_read8( uint16_t address, uint8_t *data )
{
	if( address > EEPROM_SIZE - sizeof( uint8_t ) )
		return false;

	i2c_read_reg16( at24cx_ADDR, address, data, 1 );

	return true;
}

uint8_t at24cx_read16( uint16_t address, uint16_t *data )
{
	if( address > EEPROM_SIZE - sizeof( uint16_t ) )
		return false;

	uint8_t data8[ 2 ];

	i2c_read_reg16( at24cx_ADDR, address, data8, 2 );

	*data = ( uint16_t )( data8[ 0 ] << 8 ) |
			 ( uint16_t )data8[ 1 ];

	return true;
}

uint8_t at24cx_read32( uint16_t address, uint32_t *data )
{
	if( address > EEPROM_SIZE - sizeof( uint32_t ) )
		return false;

	uint8_t data8[ 4 ];

	i2c_read_reg16( at24cx_ADDR, address, data8, 4 );

	*data = ( uint32_t )( data8[ 0 ] << 24 ) |
			 ( uint32_t )( data8[ 1 ] << 16 ) |
			 ( uint32_t )( data8[ 2 ] << 8 ) |
			 ( uint32_t )data8[ 3 ];

	return true;
}

uint8_t at24cx_write8( uint16_t address, uint8_t *data )
{
	if( address > EEPROM_SIZE - sizeof( uint8_t ) )
		return false;

	i2c_write_reg16( at24cx_ADDR, address, data, 1 );

	return true;
}

uint8_t at24cx_write16( uint16_t address, uint16_t *data )
{
	if( address > EEPROM_SIZE - sizeof( uint16_t ) )
		return false;

	uint8_t data8[ 2 ];

	data8[ 0 ] = ( uint8_t )( ( *data & 0xFF00 ) >> 8 );
	data8[ 1 ] = ( uint8_t )( *data & 0xFF );

	i2c_write_reg16( at24cx_ADDR, address, &data8[ 0 ], 1 );
	i2c_write_reg16( at24cx_ADDR, address + 1, &data8[ 1 ], 1 );

	return true;
}

uint8_t at24cx_write32( uint16_t address, uint32_t *data )
{
	if( address > EEPROM_SIZE - sizeof( uint32_t ) )
		return false;

	uint8_t data8[ 4 ];

	data8[ 0 ] = ( uint8_t )( ( *data & 0xFF000000 ) >> 24 );
	data8[ 1 ] = ( uint8_t )( ( *data & 0x00FF0000 ) >> 16 );
	data8[ 2 ] = ( uint8_t )( ( *data & 0x0000FF00 ) >> 8 );
	data8[ 3 ] = ( uint8_t )( *data & 0x000000FF );

	i2c_write_reg16( at24cx_ADDR, address, &data8[ 0 ], 1 );
	i2c_write_reg16( at24cx_ADDR, address + 1, &data8[ 1 ], 1 );
	i2c_write_reg16( at24cx_ADDR, address + 2, &data8[ 2 ], 1 );
	i2c_write_reg16( at24cx_ADDR, address + 3, &data8[ 3 ], 1 );

	return true;
}

/*==================[internal functions definition]==========================*/

/*==================[end of file]============================================*/
