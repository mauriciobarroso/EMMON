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

uint16_t usDataLoggerGetIndex( void )
{
	return usEepromGetValue( EEPROM_INDEX_ADDR );
}

uint16_t usDataLoggerGetCount( uint16_t usAddress )
{
	return usEepromGetValue( usAddress );
}

uint16_t usDataLoggerGetDaysCount( void )
{
	return usEepromGetValue( EEPROM_QTY_DAYS );
}

uint32_t ulDataLoggerGetID( void )
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

void vDataLoggerSetIndex( uint16_t usIndex )
{
	vEepromSetValue( EEPROM_INDEX_ADDR, usIndex );
}

void vDataLoggerSetCount( uint16_t usAdress, uint16_t usCount )
{
	vEepromSetValue( usAdress, usCount );
}

void vDataLoggerSetDaysCount( uint16_t usDaysCount )
{
	vEepromSetValue( EEPROM_QTY_DAYS, usDaysCount );
}

void vDataLoggerSetID( uint32_t ulID )
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

void vDataLoggerSetCurrentDate( void )
{
	ds3231_t xRTC;
	uint16_t usCurrentIndex;

	usCurrentIndex = usDataLoggerGetIndex();
	ds3231_get_date( &xRTC );

	at24c32_write( usCurrentIndex + 2, &xRTC.date.date );
	at24c32_write( usCurrentIndex + 3, &xRTC.date.month );
	at24c32_write( usCurrentIndex + 4, &xRTC.date.year );
}

void vDataLoggerInit( DataLogger_t *xData )
{
	/* inicilaización de I2C */
	i2c_init();

	/* inicialización de la variable para el índice de conteo de pulsos */
	xData->usIndex = usDataLoggerGetIndex();
	if( xData->usIndex < EEPROM_BASE_INDEX_ADDR || xData->usIndex == 0xFFFF )
		vDataLoggerSetIndex( EEPROM_BASE_INDEX_ADDR );

	/* inicialización de la variable para el conteo de pulsos */
	xData->usCount = usDataLoggerGetCount( xData->usIndex );
	if( xData->usCount == 0xFFFF )
		xData->usCount = 0;

	/* inicialización de la variable para el conteo de días monitoreados */
	xData->usDaysCount = usDataLoggerGetDaysCount();
	if( xData->usDaysCount == 0xFF )
		xData->usDaysCount = 0;

	/* inicialización de la variable para el ID del usuario */
	xData->ulID = ulDataLoggerGetID();
	if( xData->ulID == 0xFFFFFFFF )
		xData->ulID = 0x12345678;

	/* configuración de la alarma 1 del RTC */
	xData->xRtc.alarm1.seconds = 0x0;
	xData->xRtc.alarm1.minutes = 0x17;
	xData->xRtc.alarm1.hours = 0x0;
	xData->xRtc.alarm1.daydate = 0x0;
	xData->xRtc.alarm1.mode = MINUTES_SECONDS_MATCH;
	ds3231_set_alarm1( &xData->xRtc );

	/* habilitación de la interrupción por alarma del RTC */
	xData->xRtc.alarm_interrupt_mode = ENABLE_ALARM1;
	ds3231_set_alarm_interrupt( &xData->xRtc );
}

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
