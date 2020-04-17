/* Copyright 2020, Mauricio Barroso
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

#ifndef _DATALOGGER_H_
#define _DATALOGGER_H_

/*==================[inclusions]=============================================*/

#include <stdint.h>
#include <stdbool.h>

#include "at24c32.h"
#include "ds3231.h"


/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

#define EEPROM_SETTINGS_ADDR	0x0		// direccion en la eeprom donde se guarda los device settings
#define EEPROM_USER_ID_ADDR		0x10	// direccion en la eeprom donde se guarda el user ID
#define EEPROM_INDEX_ADDR		0x20	// dirección en la eeprom donde se guarda la fecha
#define	EEPROM_QTY_DAYS			0x22	// dirección en la eeprom donde se guarda la cantidad de dias monitoreados
#define EEPROM_BASE_INDEX_ADDR	0x30	// dirección en la eeprom donde se guarda el conteo de pulsos
#define DATA_SIZE				5		// tamaño en Bytes de los datos guardados en la EEPROM



/*==================[typedef]================================================*/

typedef struct
{
	uint16_t usCount;
	uint32_t ulID;
	uint16_t usIndex;
	uint16_t usDaysCount;
	ds3231_t xRtc;
} DataLogger_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

uint16_t usDataLoggerGetIndex( void );
uint16_t usDataLoggerGetCount( uint16_t usAddress );
uint16_t usDataLoggerGetDaysCount( void );
uint32_t ulDataLoggerGetID( void );
void vDataLoggerSetIndex( uint16_t usIndex );
void vDataLoggerSetCount( uint16_t usAdress, uint16_t usCount );
void vDataLoggerSetDaysCount( uint16_t usDaysCount );
void vDataLoggerSetID( uint32_t ulID );
void vDataLoggerSetCurrentDate( void );
void vDataLoggerInit( DataLogger_t *xData );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _DATALOGGER_H_ */
