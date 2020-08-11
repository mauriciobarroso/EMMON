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

static void actions( char * buf );

/*==================[external functions definition]=========================*/

bool data_transmission_init( data_transmission_t * const me )
{
	ESP_LOGI( TAG, "Configuring lora transceiver..." );
	/* se inicializa el módulo lora */
	if( lora_begin( 433E6 ) == 0 )
	{
		ESP_LOGI( TAG, "failed to start lora" );
		return 0;
	}
	ESP_LOGI( TAG, "lora frequency 433 MHz" );

//	lora_set_signal_bandwidth( 41.7E3 );
//	lora_set_spreading_factor( 12 );
//	lora_set_coding_rate4( 5 );

	ESP_LOGI( TAG, "Configured lora!" );

	/* se crea la cola */
	me->queue = xQueueCreate( 10, sizeof( uint16_t ) );
	if( me->queue == NULL )
	{
		ESP_LOGI( TAG, "failed to create queue" );
		return 0;
	}
	ESP_LOGI( TAG, "Created queue!" );

	return 1;
}

void lora_task( void * arg )	/* cambiar nombre! */
{
	data_transmission_t * data_transmission = ( data_transmission_t * )arg;
	char buf[ MAX_PACKET_LEN ];
	uint16_t pulses;

	for( ;; )
	{
		while( uxQueueMessagesWaiting( data_transmission->queue ) != 0 )
		{
			ESP_LOGI( TAG, "ENTRO!" );
			/* se bloquea esperando paquetes a transmitir */
			if( xQueueReceive( data_transmission->queue, &pulses, portMAX_DELAY ) == pdTRUE )	/*revisar!*/
			{
				/* send to LoRa gateway REVISAR!!! */
				sprintf( buf, "%08X,1,%06.2f", HOST_ADDR, pulses * 0.000625 );	/* se arma el paquete a transmitir */

				lora_disable_invert_iq();
				lora_send_packet( buf, strlen( buf ) );

				ESP_LOGI( TAG, buf );	/* se imprime por consola el paquete a enviar */
			}
		}

		lora_enable_invert_iq();
		lora_recv();
		while( lora_received() )
		{
			int len = 0;
			len = lora_receive_packet( buf, sizeof( buf ) );
			buf[ len ] = '\0';

			ESP_LOGI( TAG, buf );
			ESP_LOGI( TAG, "RSSI: %d", lora_packet_rssi() );

			/* se ejecutan las acciones requeridas por el gateway */
			actions( buf );
		}

		vTaskDelay( pdMS_TO_TICKS( 500 ) );
	}
}


/*==================[internal functions definition]==========================*/

static void actions( char * buf )
{
	int address, operation;
	char data[ 32 ];

	sscanf( buf, "%u,%u,%s", &address, &operation, data );

	ESP_LOGI( TAG, "address=%u", address );
	ESP_LOGI( TAG, "operation=%u", operation );
	ESP_LOGI( TAG, "data=%s", data );

	if( address == 12345678 || address == BROADCAST_ADDR )
	{
		ESP_LOGI( TAG, "address ok!" );
		switch( operation )
		{
			/* cambio de fc_kwh y fc_price */
			case 0x1:
			{
				ESP_LOGI( TAG, "operation 1" );
//
//				FILE * f = NULL;
//
//				f = fopen( "spiffs/params_data.txt", "w" );
//				if( f != 0 )
//				{
//					fprintf( f, "%f&%f", kwh, price );
//					fclose( f );
//				}
				break;
			}

			default:
				ESP_LOGI( TAG, "default, error" );
				break;
		}
	}
	else
		ESP_LOGI( TAG, "error" );
}

/*==================[end of file]============================================*/
