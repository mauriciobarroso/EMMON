/*
 * data_transmission.c
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

/*==================[inlcusions]============================================*/

#include <data_communication.h>

/*==================[macros]=================================================*/

#define MAX_PACKET_LEN	255

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

static const char * TAG = "data_communication";

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static void lora_task( void * arg );

/*==================[external functions definition]=========================*/

esp_err_t data_transmission_init( data_transmission_t * const me )
{
	ESP_LOGI( TAG, "Configuring lora transceiver..." );

	/* se inicializa el módulo lora */
	if( lora_begin( 433E6 ) == 0 )
	{
		ESP_LOGI( TAG, "failed to start lora" );
		return ESP_FAIL;
	}
	ESP_LOGI( TAG, "lora frequency 433 MHz" );

	ESP_LOGI( TAG, "Configured lora!" );

	/* se crea la cola */
	me->queue = xQueueCreate( 10, sizeof( uint16_t ) );
	if( me->queue == NULL )
	{
		ESP_LOGI( TAG, "failed to create queue" );
		return ESP_FAIL;
	}
	ESP_LOGI( TAG, "Created queue!" );

	/* se crea la tarea de data_transmission */
	xTaskCreate( lora_task, "LoRa Task", configMINIMAL_STACK_SIZE * 2, ( void * )me, tskIDLE_PRIORITY + 1, NULL );
	ESP_LOGI( TAG, "Created task!" );

	return ESP_OK;
}

/*==================[internal functions definition]==========================*/

/* send/receive LoRa task */
static void lora_task( void * arg )
{
	data_transmission_t * data_transmission = ( data_transmission_t * )arg;
	char buf[ MAX_PACKET_LEN ];
	uint16_t pulses;

	for( ;; )
	{
		/* se bloquea esperando paquetes a transmitir */
		if( xQueueReceive( data_transmission->queue, &pulses, portMAX_DELAY ) == pdTRUE )	/*revisar!*/
		{
			/* se construye e imprime el paque a transmitir */
			sprintf( buf, "%08u,0,%06.2f", data_transmission->settings.id, pulses * data_transmission->settings.pulses_to_kwh );	/* se arma el paquete a transmitir */
			ESP_LOGI( TAG, "sending %08u0%06.2f,", data_transmission->settings.id, pulses * data_transmission->settings.pulses_to_kwh );

			/* se configura lora en modo de transmisión y se transmite
			 * el paquete antes construido */
			lora_disable_invert_iq();
			vPortEnterCritical();					// se cierra sección crítica
			lora_send_packet( buf, strlen( buf ) );
			vPortExitCritical();					// se cierra sección crítica
			ESP_LOGI( TAG, "packet sent!" );
		}
	}
}

/*==================[end of file]============================================*/
