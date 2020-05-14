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

#include "dataTransmission.h"

/*==================[macros]=================================================*/

#define MAX_TX_PACKET_LEN	14

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static const char * TAG = "data_transmission";
static void send_to_network( void * arg );
//static void check_network_status( void * arg );

/*==================[external functions definition]=========================*/

void data_transmission_init( data_transmission_t * const me )
{
	lora_begin( 433E6 );

	me->queue = xQueueCreate( 10, sizeof( data_transmission_packet_t ) );

	xTaskCreate( send_to_network, "Data Transmission Task", configMINIMAL_STACK_SIZE * 2, ( void * )me, tskIDLE_PRIORITY + 1, NULL );
}

/*==================[internal functions definition]==========================*/

static void send_to_network( void * arg )
{
	data_transmission_t * data_transmission = ( data_transmission_t * )arg;
	char buffer[ MAX_TX_PACKET_LEN ];
	data_transmission_packet_t packet;

	for( ;; )
	{
		/* se bloquea esperando paquetes a transmitir */
		xQueueReceive( data_transmission->queue, &packet, portMAX_DELAY );

		/* se consulta el estado de la red lora */

		/* se arma el paquete a transmitir */
		sprintf( buffer, "%05d,%08X", packet.pulses, packet.id );
		ESP_LOGI( TAG, buffer );

		/* send to LoRa network */
		lora_begin_packet( false );
		lora_write( ( const uint8_t * )buffer, MAX_TX_PACKET_LEN );
		lora_end_packet( false );
	}
}

/*
static void check_network_status( void * arg )
{
	for( ;; )
	{
		// if lora available then send task notification to vDataTransmissionSend
	}
}*/

/*==================[end of file]============================================*/
