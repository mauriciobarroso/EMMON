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
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "at24c32.h"
#include "ds3231.h"


/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

#define EEPROM_SETTINGS_ADDR	0x0	// direccion en la eeprom donde se guarda los device settings
#define EEPROM_USER_ID_ADDR		0x10	// direccion en la eeprom donde se guarda el user ID
#define EEPROM_INDEX_ADDR		0x20	// dirección en la eeprom donde se guarda la fecha
#define EEPROM_BASE_INDEX_ADDR	0x30		// dirección en la eeprom donde se guarda el conteo de pulsos



/*==================[typedef]================================================*/

typedef struct
{
	Ds3231Date_t xDate;
	uint16_t usCount;
	uint32_t ulID;
	uint16_t usIndex;
} DataLogger_t;

/*==================[external data declaration]==============================*/

//extern vPulseCountInit();

/*==================[external functions declaration]=========================*/

extern uint16_t usDataLoggerGetIndex( void );
extern uint16_t usDataLoggerGetCount( uint16_t usAddress );
//extern uint32_t ulDataLoggerGetID( void );
extern void vDataLoggerSetIndex( uint16_t usIndex );
extern void vDataLoggerSetCount( uint16_t usAdress, uint16_t usCount );
//extern void vDataLoggerSetID( uint32_t ulID );
extern void vDataLoggerInit( DataLogger_t *xData );
/*
extern uint16_t usDataLoggerGetRandomCount( uint16_t usAddress );*
extern uint16_t usDataLoggerGetCurrentCount( void );*
extern uint16_t usDataLoggerGetCountIndex( void );*
extern uint16_t usDataLoggerGetDateIndex( void );*

extern void vDataLoggerSetCurrentCount( uint16_t usCount, DataLogger_t *xData );*
extern void vDataLoggerSetCurrentCountIndex( uint16_t usCountIndex );*
extern void vDataLoggerSetDateIndex( uint16_t usDateIndex );*
*/

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _DATALOGGER_H_ */
