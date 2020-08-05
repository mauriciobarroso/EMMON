/*
 * data_logger.c
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

/*==================[inlcusions]============================================*/

#include <data_logger.h>

/*==================[macros]=================================================*/

/* mask to configure interrupt pins */
#define DATA_LOGGER_PINS_MASK		( ( 1ULL << GPIO_PULSES ) | ( 1ULL << GPIO_ALARM ) )	/*!< data logger mask pins */

/* addresses */
#define CURRENT_INDEX_ADDR			0x0	/*!< dirección en la eeprom donde se guarda la fecha */
#define	TOTAL_LOGGED_DAYS_ADDR		0x2	/*!< dirección en la eeprom donde se guarda la cantidad de dias monitoreados */

/* values */
#define BASE_INDEX					0x10	/*!< índice base para el almacenamiento de pulsos (cambiar por EEPROM_OFFSET */
#define DATA_SIZE					5		/*!< tamaño en bytes de los datos guardados(pulsos diarios y fecha) en la EEPROM */

/**/
#define MAX_LOGGED_DAYS				( ( EEPROM_SIZE - BASE_INDEX ) / DATA_SIZE )

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/* tag for debug */
static const char * TAG = "data_logger";

static uint8_t edge_status = 0;
static TickType_t current_time = 0;
static TickType_t elapsed_time = 0;

/*==================[external data declaration]==============================*/

/* handler tasks */
static TaskHandle_t pulses_handle, alarm_handle;

/*==================[internal functions declaration]=========================*/

/* isr handlers */
static void IRAM_ATTR pulses_isr( void * arg );
static void IRAM_ATTR alarm_isr( void * arg );

/* function tasks */
static void pulses_task( void * arg );
static void alarm_task( void * arg );

/* alarm settings function */
static void set_data_frequency( data_logger_t * const me );

/* get settings from setting.txt function */
static void get_settings( data_logger_t * const me );

/*==================[external functions definition]=========================*/

void data_logger_init( data_logger_t * const me )
{
	/* se inicializa la interfaz i2c */
	i2c_init();

	/* se inicializan los spiffs */
	spiffs_init();

	/* función para cargar settings de config.txt
	 *
	 *
	 *  */
	me->frequency = 1;

//	uint8_t data = 0;
//	uint8_t algo = 0;
//	for( uint16_t i = 0; i < 0xF; i++ )
//	{
//		at24cx_write8( i, &data );
//		at24cx_read8( i, &algo );
//		ESP_LOGI( TAG, "E[0x%02X]=%d", i, algo );
//	}

	me->rtc.time.hours = 0x23;
	me->rtc.time.minutes = 0x59;
	me->rtc.time.seconds = 0x57;
	ds3231_set_time( &me->rtc );
//
//	me->rtc.date.date = 0x2;
//	me->rtc.date.month = 0x8;
//	me->rtc.date.year = 0x20;
//	ds3231_set_date( &me->rtc );

	/* se inicializa la variable para el índice de conteo de pulsos */
	at24cx_read16( CURRENT_INDEX_ADDR, &me->index );
	if( me->index < BASE_INDEX || me->index == 0xFFFF )
		me->index = BASE_INDEX;
	ESP_LOGI( TAG, "index:0x%X", me->index );

	/* se inicializa la variable para el conteo de pulsos */
	at24cx_read16( me->index, &me->pulses );
	if( me->pulses == 0xFFFF )
		me->pulses = 0;
	ESP_LOGI( TAG, "pulses:%d", me->pulses );

	/* se inicializa la variable para el conteo total de días monitoreados */
	at24cx_read16( TOTAL_LOGGED_DAYS_ADDR, &me->logged_days );
	if( me->logged_days == 0xFFFF )
		me->logged_days = 0;
	ESP_LOGI( TAG, "logged days:%d", me->logged_days );

	/* se obtiene la hora y fecha del RTC y se almacena en la eeprom */
	ds3231_get_date( &me->rtc );
	at24cx_write8( me->index + 2, &me->rtc.date.date );
	at24cx_write8( me->index + 3, &me->rtc.date.month );
	at24cx_write8( me->index + 4, &me->rtc.date.year );
	ESP_LOGI( TAG, "date:%02x/%02x/%02x", me->rtc.date.date, me->rtc.date.month, me->rtc.date.year );

	/* se configura la alarma 1 del RTC */
	set_data_frequency( me );

	/* se configuran los GPIOs */
	ESP_LOGI( TAG, "Configuring GPIOs..." );
	gpio_config_t gpio_conf;
	gpio_conf.pin_bit_mask = DATA_LOGGER_PINS_MASK;
	gpio_conf.mode = GPIO_MODE_INPUT;
	gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;		/* habilitar si fisicamente no hay resistencias de pull-up */
	gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	gpio_conf.intr_type = GPIO_INTR_NEGEDGE;	/* para PULSES POSEDGE!!! */
	gpio_config( &gpio_conf );
	ESP_LOGI( TAG, "GPIOs configured!" );

	/* se configuran las interrupciones y los handlers */
	gpio_install_isr_service( 0 );
	gpio_isr_handler_add( GPIO_PULSES, pulses_isr, NULL );
	gpio_isr_handler_add( GPIO_ALARM, alarm_isr, NULL );

	/* se crean las tareas */
	xTaskCreate( pulses_task, "Pulses Task", configMINIMAL_STACK_SIZE * 2, ( void * )me, tskIDLE_PRIORITY + 3, &pulses_handle );	/* tarea para el conteo de pulsos */
	xTaskCreate( alarm_task, "Alarm Task", configMINIMAL_STACK_SIZE * 2, ( void * )me, tskIDLE_PRIORITY + 2, &alarm_handle );		/* tarea para ejecutar las acciones cuadno ocurre la alarma */
	ESP_LOGI( TAG, "Tasks created!" );

	/* se consulta si la alarma está activa y se notifica a la tarea encargada */
	if( ds3231_get_alarm_flag( 1 ) )
		xTaskNotifyGive( alarm_handle );

	/* se obtienen la configuración de settings.txt */
	get_settings( me );
}

void data_logger_get_history( data_logger_t * const me )
{
	FILE * f = NULL;

	f = fopen( "/spiffs/pulses.csv", "w" );

	if( f != 0 )
	{
		uint16_t pulses;
		float kwh;
		uint8_t date, month, year;

		ESP_LOGI( TAG, "===============================" );
		ESP_LOGI( TAG, "address,date,month,year,kwh" );
		fprintf( f, "date,kwh\n" );
		for( uint16_t i = BASE_INDEX; i < BASE_INDEX + ( me->logged_days * DATA_SIZE ) + 1; i += DATA_SIZE )
		{
			at24cx_read16( i, &pulses );
			kwh = ( float )pulses * me->pulses_to_kwh;
			at24cx_read8( i + 2, &date );
			at24cx_read8( i + 3, &month );
			at24cx_read8( i + 4, &year );
			ESP_LOGI( TAG, "E[%03X] ,%02x  ,%02x   ,%02x  ,%06.2f", i, date, month, year, kwh );
			fprintf( f, "20%02x-%02x-01,%06.2f\n", year, month, kwh );
		}
		fclose( f );
		ESP_LOGI( TAG, "===============================" );
	}
//	f = fopen( "/spiffs/config.txt", "w" );
//	if( f != 0 )
//	{
//		fprintf( f, "4\n0.000625\nCASAwifi,orcobebe" );
//		fclose( f );
//	}
}

/*==================[internal functions definition]==========================*/

/* interrupt service routine for incoming digital pulses */
static void IRAM_ATTR pulses_isr( void *arg )
{
	BaseType_t higher_priority_task_woken = pdFALSE;

	if( !edge_status )	/* flanco positivo */
	{
		current_time = xTaskGetTickCountFromISR();
		gpio_set_intr_type( GPIO_PULSES, GPIO_INTR_NEGEDGE );
		edge_status = 1;
	}
	else	/* flanco negativo */
	{
		elapsed_time = xTaskGetTickCountFromISR() - current_time;
		gpio_set_intr_type( GPIO_PULSES, GPIO_INTR_POSEDGE );
		edge_status = 0;
	}

	if( elapsed_time >= 10 )
	{
		elapsed_time = 0;
		vTaskNotifyGiveFromISR( pulses_handle, &higher_priority_task_woken );
		portEND_SWITCHING_ISR( higher_priority_task_woken );
	}
}

/* interrupt service routine for incoming RTC alarm */
static void IRAM_ATTR alarm_isr( void *arg )
{
	BaseType_t higher_priority_task_woken = pdFALSE;
	vTaskNotifyGiveFromISR( alarm_handle, &higher_priority_task_woken );
	portEND_SWITCHING_ISR( higher_priority_task_woken );
}

static void pulses_task( void * arg )
{
	uint32_t event_to_process;
	data_logger_t * data_logger = ( data_logger_t * )arg;
//	ds3231_t last_time = { 0 };	/* variable temporal de la hora del último pulso registrado */
//	uint32_t counter_repeat = 0;

	for( ;; )
	{
		/* se espera la notificación del isr de los pulsos */
		event_to_process = ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

		if( event_to_process != 0 )
		{
			/* se aumenta en 1 el conteo de pulsos, se guarda en la EEPROM y se almacena en la eeprom */
			data_logger->pulses++;
			at24cx_write16( data_logger->index, &data_logger->pulses );

			ESP_LOGI( TAG, "[0x%X]=%d", data_logger->index, data_logger->pulses );
			ds3231_get_time( &data_logger->rtc );
			ESP_LOGI( TAG, "%02x/%02x/%02x,%02x:%02x:%02x", data_logger->rtc.date.date, data_logger->rtc.date.month, data_logger->rtc.date.year, data_logger->rtc.time.hours, data_logger->rtc.time.minutes, data_logger->rtc.time.seconds );
		}
	}
}

static void alarm_task( void * arg )
{
	uint32_t event_to_process;
	data_logger_t * data_logger = ( data_logger_t * )arg;

	for( ;; )
	{
		/* se espera la notificación del isr de la alarma */
		event_to_process = ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

		if( event_to_process != 0 )
		{
			ESP_LOGI( TAG, "Alarm!" );

			/* se abre sección crítica */
			taskENTER_CRITICAL();

			/* se constuye el paquete y se manda  a la cola de Data Transmission */

			float kwh = data_logger->pulses_to_kwh * ( float )data_logger->pulses;
			ESP_LOGI( TAG, "sending %f to queue", kwh );
			xQueueSend( data_logger->queue, &kwh, 0 );

			ds3231_get_time( &data_logger->rtc );
			if( data_logger->rtc.time.hours == 0x00 && data_logger->rtc.time.minutes == 0x00 )	/* si la alarma se genera una vez al día */
			{
				ESP_LOGI( TAG, "new day!" );
				/* se aumenta el indice en DATA_SIZE y se guarda en la eeprom */
				data_logger->index += DATA_SIZE;
				if( data_logger->index > ( EEPROM_SIZE - DATA_SIZE ) )
					data_logger->index = BASE_INDEX;
				at24cx_write16( CURRENT_INDEX_ADDR, &data_logger->index );

				/* se pone en 0 el conteo diario de pulsos y se guarda en la memoria eeprom */
				data_logger->pulses = 0;
				at24cx_write16( data_logger->index, &data_logger->pulses );

				/* se obtiene la nueva fecha y se guarda en la eeprom*/
				ds3231_get_date( &data_logger->rtc );
				at24cx_write8( data_logger->index + 2, &data_logger->rtc.date.date );
				at24cx_write8( data_logger->index + 3, &data_logger->rtc.date.month );
				at24cx_write8( data_logger->index + 4, &data_logger->rtc.date.year );

				/* se aumenta en 1 el conteo total de dias monitoreados y se guarda en la eeprom */
				if( data_logger->logged_days < MAX_LOGGED_DAYS ) //!!!
				{
					data_logger->logged_days++;
					at24cx_write16( TOTAL_LOGGED_DAYS_ADDR, &data_logger->logged_days );
				}
			}

			/* se cierra sección crítica */
			taskEXIT_CRITICAL();

			/* se borra el flag de la alarma */
			ESP_LOGI( TAG, "clear flag!" );
			ds3231_clear_alarm_flag( 1 );

			/* se genera el log histórico de pulsos */
//			data_logger_get_history( data_logger );
		}
	}
}

static void set_data_frequency( data_logger_t * const me )
{
	me->rtc.alarm1.seconds = 0x0;
	me->rtc.alarm1.minutes = 0x0;
	me->rtc.alarm1.hours = 0x0;
	me->rtc.alarm1.daydate = 0x0;

	/* set alarm */
	switch( me->frequency )
	{
		case 1:
			/* se configura la alarma 1 del RTC */
			me->rtc.alarm1.mode = SECONDS_MATCH;
			ds3231_set_alarm( &me->rtc, ALARM1 );
			me->rtc.alarm_interrupt_mode = ENABLE_ALARM1;
			ds3231_set_alarm_interrupt( &me->rtc );
			ESP_LOGI( TAG, "Alarm configured once per minute!" );
			break;
		case 2:
			/* se configura la alarma 1 del RTC */
			me->rtc.alarm1.mode = MINUTES_SECONDS_MATCH;
			ds3231_set_alarm( &me->rtc, ALARM1 );
			me->rtc.alarm_interrupt_mode = ENABLE_ALARM1;
			ds3231_set_alarm_interrupt( &me->rtc );
			ESP_LOGI( TAG, "Alarm configured once per hour!" );
			break;
		case 3:
			/* se configura la alarma 1 del RTC */
			me->rtc.alarm1.mode = HOURS_MINUTES_SECONDS_MATCH;
			ds3231_set_alarm( &me->rtc, ALARM1 );
			me->rtc.alarm_interrupt_mode = ENABLE_ALARM1;
			ds3231_set_alarm_interrupt( &me->rtc );
			ESP_LOGI( TAG, "Alarm configured once per day!" );
			break;
		default:
			/* se configura la alarma 1 del RTC */
			me->rtc.alarm1.mode = HOURS_MINUTES_SECONDS_MATCH;
			ds3231_set_alarm( &me->rtc, ALARM1 );
			me->rtc.alarm_interrupt_mode = ENABLE_ALARM1;
			ds3231_set_alarm_interrupt( &me->rtc );
			ESP_LOGI( TAG, "Alarm configured once per day!" );
			break;
	}
}

static void get_settings( data_logger_t * const me )
{
	FILE * f = NULL;
	f = fopen( "/spiffs/config.txt", "r" );
	if( f != 0 )
	{
		char line[ 32 ];
		for( uint8_t i = 0; fgets( line, sizeof( line ), f ) != NULL; i++ )
		{
			switch( i )
			{
				case 0:
				{
					sscanf( line, "%u", &me->frequency );
					ESP_LOGI( TAG, "data_frequency=%u", me->frequency );
					break;
				}
				case 1:
				{
					sscanf( line, "%f", &me->pulses_to_kwh );
					ESP_LOGI( TAG, "pulse_to_kwh=%f", me->pulses_to_kwh );
					break;
				}
				case 2:
				{
					sscanf( line, "%s", me->wifi_data );
					ESP_LOGI( TAG, "wifi_data=%s", me->wifi_data );
					break;
				}
				default:
				{
					ESP_LOGI( TAG, "ERROR" );
					break;
				}
			}
		}

		fclose( f );
	}
}

/*==================[end of file]============================================*/
