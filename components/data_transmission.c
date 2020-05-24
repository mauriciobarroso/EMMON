/*
 * data_transmission.c
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

/*==================[inlcusions]============================================*/

#include <data_transmission.h>

/*==================[macros]=================================================*/

#define MAX_TX_PACKET_LEN	14
#define MAX_RX_PACKET_LEN	255

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

static const char * TAG = "data_transmission";

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static void send_to_gateway( void * arg );
static void receive_from_gateway( void * arg );
static void tx_mode( void );
static void rx_mode( void );

/*==================[external functions definition]=========================*/

bool data_transmission_init( data_transmission_t * const me )
{


	me->id = 0x12345678;
	me->broadcast = 0xFFFFFFFF;

	/* se inicializa el módulo lora */
	if( lora_begin( 433E6 ) == 0 )
	{
		ESP_LOGI( TAG, "failed to start lora" );
		return 0;
	}

	/* se crea la cola */
	me->queue = xQueueCreate( 10, sizeof( uint16_t ) );
	if( me->queue == NULL )
	{
		ESP_LOGI( TAG, "failed to create mutex" );
		return 0;
	}

	/* se crea el mutex */
	me->mutex = NULL;
	me->mutex = xSemaphoreCreateMutex();
	if( me->mutex == NULL )
	{
		ESP_LOGI( TAG, "failed to create mutex" );
		return 0;
	}

	/* se crean las tareas */
	xTaskCreate( send_to_gateway, "Send to gateway Task", configMINIMAL_STACK_SIZE * 2, ( void * )me, tskIDLE_PRIORITY + 1, NULL );
	xTaskCreate( receive_from_gateway, "Receive from gateway Task", configMINIMAL_STACK_SIZE * 2, ( void * )me, tskIDLE_PRIORITY + 1, NULL );

	return 1;
}

/*==================[internal functions definition]==========================*/

static void send_to_gateway( void * arg )
{
	data_transmission_t * data_transmission = ( data_transmission_t * )arg;
	char buffer[ MAX_TX_PACKET_LEN ];
	uint16_t pulses;

	for( ;; )
	{
		/* se bloquea esperando paquetes a transmitir */
		xQueueReceive( data_transmission->queue, &pulses, portMAX_DELAY );

		/* se consulta el estado de la red lora */

		/* se arma el paquete a transmitir */
		sprintf( buffer, "%05d,%08X", pulses, data_transmission->id );
		ESP_LOGI( TAG, buffer );

		/* send to LoRa gateway */
		if( xSemaphoreTake( data_transmission->mutex, portMAX_DELAY ) == pdTRUE )	/* se toma el mutex */
		{
			ESP_LOGI( TAG, "tx mode" );
			tx_mode();
			lora_begin_packet( false );
			lora_write( ( const uint8_t * )buffer, MAX_TX_PACKET_LEN );
			lora_end_packet( false );

			xSemaphoreGive( data_transmission->mutex );	/* se da el mutex */
		}
	}
}

static void receive_from_gateway( void * arg )
{
	int who, me, action;
	char param[ 32 ];
	char buf[ MAX_RX_PACKET_LEN ];
	int packet_size;
	data_transmission_t * data_transmission = ( data_transmission_t * )arg;

	for( ;; )
	{
		/* receive form LoRa gateway */
		if( xSemaphoreTake( data_transmission->mutex, portMAX_DELAY ) == pdTRUE )	/* se toma el mutex */
		{
//			ESP_LOGI( TAG, "rx mode" );
			rx_mode();

			packet_size = lora_parse_packet( 0 );
			if( packet_size )
			{
				ESP_LOGI( TAG, "message received!" );
				ESP_LOGI( TAG, "size:%d", packet_size );
				uint8_t i = 0;
				while( lora_available() )
				{
					buf[ i ] = ( char )lora_read();
					i++;
				}
				ESP_LOGI( TAG, buf );
				sscanf( buf, "%u,%u,%u,%s", &who, &me, &action, param );

				if( ( who == 0x0 ) && ( me == data_transmission->id || me == 0xFFFFFFFF ) )
				{
					switch( action )
					{
						/* cambio de fc_kwh y fc_price */
						case 0x1:
						{
							float kwh, price;	/* cambiar por una variables global */
							sscanf( param, "%f,%f", &kwh, &price );

							FILE * f = NULL;

							f = fopen( "spiffs/params.txt", "w" );
							if( f != 0 )
							{
								fprintf( f, "kwh=%f,price=%f", kwh, price );
								fclose( f );
							}

							break;
						}

						/* notificaciones */
						case 0x2:
						{
							FILE * f = NULL;

							f = fopen( "spiffs/notifications.txt", "w" );
							if( f != 0 )
							{
								fprintf( f, "notification=%s", param );
								fclose( f );
							}

							break;
						}

						default:
							break;
					}
				}
			}

			xSemaphoreGive( data_transmission->mutex );	/* se da el mutex */
		}

		vTaskDelay( pdMS_TO_TICKS( 2000 ) );
	}
}

static void tx_mode( void )
{
	lora_idle();
	lora_disable_invert_iq();
}

static void rx_mode( void )
{
	lora_enable_invert_iq();
//	lora_receive( 0 );
}

/*==================[end of file]============================================*/
