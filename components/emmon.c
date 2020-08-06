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

#include <emmon.h>

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/* tag for debug */
static const char * TAG = "emmon";

/*==================[external data declaration]==============================*/

void emmon_init( emmon_t * const me )
{
	spiffs_init();

	spiffs_get_settings( &me->data_logger.settings );
	ESP_LOGI( TAG, "data_frequency=%d", me->data_logger.settings.frequency );
	ESP_LOGI( TAG, "pulse_to_kwh=%f", me->data_logger.settings.pulses_to_kwh );
	ESP_LOGI( TAG, "wifi_data=%s", me->data_logger.settings.wifi_data );
	spiffs_get_settings( &me->web_server.settings );
	spiffs_get_settings( &me->data_transmission.settings );

	web_server_init( &me->web_server );
	data_logger_init( &me->data_logger );
	data_transmission_init( &me->data_transmission );

	me->data_logger.queue = me->data_transmission.queue;

	/* se crean las tareas de data_logger */
	xTaskCreate( data_loggger_pulses_task, "Pulses Task", configMINIMAL_STACK_SIZE * 2, ( void * )&me->data_logger, tskIDLE_PRIORITY + 3, &me->data_logger.pulses_handle );	/* tarea para el conteo de pulsos */
	xTaskCreate( data_loggger_alarm_task, "Alarm Task", configMINIMAL_STACK_SIZE * 2, ( void * )&me->data_logger, tskIDLE_PRIORITY + 2, &me->data_logger.alarm_handle );		/* tarea para ejecutar las acciones cuadno ocurre la alarma */
	ESP_LOGI( TAG, "Tasks created!" );
//
	/* se crea la tarea de data_transmission */
	xTaskCreate( send_to_gateway, "Send to gateway Task", configMINIMAL_STACK_SIZE * 2, ( void * )&me->data_transmission, tskIDLE_PRIORITY + 1, NULL );
	ESP_LOGI( TAG, "Created task!" );
}

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]=========================*/

/*==================[internal functions definition]==========================*/

/*==================[end of file]============================================*/
