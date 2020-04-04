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

#include "dataLogger.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static uint16_t usEepromGetValue( uint16_t usAddress );
static void vEepromSetValue( uint16_t usAddress, uint16_t usValue );

/*==================[external functions definition]=========================*/

extern uint16_t usDataLoggerGetIndex( void )
{
	return usEepromGetValue( EEPROM_INDEX_ADDR );
}

extern uint16_t usDataLoggerGetCount( uint16_t usAddress )
{
	return usEepromGetValue( usAddress );
}

//extern uint32_t ulDataLoggerGetID( void )

extern void vDataLoggerSetIndex( uint16_t usIndex )
{
	vEepromSetValue( EEPROM_INDEX_ADDR, usIndex );
}

extern void vDataLoggerSetCount( uint16_t usAdress, uint16_t usCount )
{
	vEepromSetValue( usAdress, usCount );
}

//extern void vDataLoggerSetID( uint32_t ulID );

extern void vDataLoggerInit( DataLogger_t *xData )
{
	i2c_init();
	//vDataLoggerSetIndex( EEPROM_BASE_INDEX_ADDR );

	xData->usIndex = usDataLoggerGetIndex();
	//if( xData->usCountIndex == 0xFFFF )
		//vDataLoggerSetCurrentCountIndex( xData->usCountIndex );

	xData->usCount = usDataLoggerGetCount( xData->usIndex );
	//if( xData->usCount == 0xFFFF )
		//xData->usCount = 0;
}

/*
extern uint16_t usDataLoggerGetRandomCount( uint16_t usAddress )
{
	return usEepromGetValue( usAddress );
}

extern uint16_t usDataLoggerGetCurrentCount( void )
{
	return usEepromGetValue( usEepromGetValue( EEPROM_CURRENT_INDEX_ADDR ) );
}

extern uint16_t usDataLoggerGetCountIndex( void )
{
	return usEepromGetValue( EEPROM_CURRENT_INDEX_ADDR );
}

extern uint16_t usDataLoggerGetDateIndex( void )
{
	uint16_t usDateIndex;
	uint16_t usCountIndex;

	usCountIndex = usDataLoggerGetCountIndex();
	usDateIndex = EEPROM_DATE_ADDR + usCountIndex + ( usCountIndex / 2 );

	return usDateIndex;
}

extern uint32_t ulDataLoggerGetID( void )
{
	uint32_t ulID;
	uint8_t ucID[ 4 ];

	at24c32_read( EEPROM_USER_ID_ADDR, &ucID[ 0 ] );
	at24c32_read( EEPROM_USER_ID_ADDR + 1, &ucID[ 1 ] );
	at24c32_read( EEPROM_USER_ID_ADDR + 2, &ucID[ 2 ] );
	at24c32_read( EEPROM_USER_ID_ADDR + 3, &ucID[ 3 ] );

	ulID = ( uint32_t )( ucID[ 0 ] << 24 ) |
		   ( uint32_t )( ucID[ 1 ] << 16 ) |
		   ( uint32_t )( ucID[ 2 ] << 8 ) |
		   ( uint32_t )( ucID[ 3 ] );

	return ulID;
}

extern void vDataLoggerSetCurrentCount( uint16_t usCount, DataLogger_t *xData )
{
	vEepromSetValue( xData->usCountIndex, usCount );
}

extern void vDataLoggerSetID( uint32_t ulID )
{
	uint8_t ucID[ sizeof( ulID ) ];

	ucID[ 0 ] = ( uint8_t )( ( ulID & 0xFF000000 ) >> 24 );
	ucID[ 1 ] = ( uint8_t )( ( ulID & 0x00FF0000 ) >> 16 );
	ucID[ 2 ] = ( uint8_t )( ( ulID & 0x0000FF00 ) >> 8 );
	ucID[ 3 ] = ( uint8_t )( ulID & 0x000000FF );

	at24c32_write( EEPROM_USER_ID_ADDR, &ucID[ 0 ] );
	at24c32_write( EEPROM_USER_ID_ADDR + 1, &ucID[ 1 ] );
	at24c32_write( EEPROM_USER_ID_ADDR + 2, &ucID[ 2 ] );
	at24c32_write( EEPROM_USER_ID_ADDR + 3, &ucID[ 3 ] );
}

extern void vDataLoggerSetCurrentCountIndex( uint16_t usCountIndex )
{
	vEepromSetValue( EEPROM_CURRENT_INDEX_ADDR, usCountIndex );
}

extern void vDataLoggerSetDateIndex( uint16_t usDateIndex )
{
	vEepromSetValue( usDataLoggerGetDateIndex(), usDateIndex );
}

extern void vDataLoggerInit( DataLogger_t *xData )
{
	xMutex = xSemaphoreCreateMutex();

	//vDataLoggerSetCurrentCountIndex( EEPROM_PULSE_COUNT_ADDR );

	xData->usCountIndex = usDataLoggerGetCountIndex();
	//if( xData->usCountIndex == 0xFFFF )
		//vDataLoggerSetCurrentCountIndex( xData->usCountIndex );

	xData->usCount = usDataLoggerGetCurrentCount();
	//if( xData->usCount == 0xFFFF )
		//xData->usCount = 0;

	xData->usDateIndex = usDataLoggerGetDateIndex();

	vDataLoggerSetID( 65535 );
	xData->ulID = ulDataLoggerGetID();
}
*/
/*==================[internal functions definition]==========================*/


static uint16_t usEepromGetValue( uint16_t usAddress )
{
	uint16_t usValue;
	uint8_t ucValue[ 2 ];

	at24c32_read( usAddress, &ucValue[ 0 ] );
	at24c32_read( usAddress + 1, &ucValue[ 1 ] );

	usValue = ( uint16_t )( ucValue[ 0 ] << 8 ) |
		      ( uint16_t )( ucValue[ 1 ] );

	return usValue;
}

static void vEepromSetValue( uint16_t usAddress, uint16_t usValue )
{
	uint8_t ucValue[ 2 ];

	ucValue[ 0 ] = ( uint8_t )( ( usValue & 0xFF00 ) >> 8 );
	ucValue[ 1 ] = ( uint8_t )( usValue & 0xFF );

	at24c32_write( usAddress, &ucValue[ 0 ] );
	at24c32_write( usAddress + 1, &ucValue[ 1 ] );
}

/*==================[end of file]============================================*/
