/*
 * data_logger.c
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

/*==================[inlcusions]============================================*/

#include <data_logger.h>

/*==================[macros]=================================================*/

/* addresses */
#define CURRENT_INDEX_ADDR			0x4	/*!< dirección en la eeprom donde se guarda la fecha */
#define	TOTAL_LOGGED_DAYS_ADDR		0x6	/*!< dirección en la eeprom donde se guarda la cantidad de dias monitoreados */

/* values */
#define BASE_INDEX					0x30	/*!< índice base para el almacenamiento de pulsos (cambiar por EEPROM_OFFSET */
#define DATA_SIZE					5		/*!< tamaño en bytes de los datos guardados(pulsos diarios y fecha) en la EEPROM */

/**/
#define MAX_LOGGED_DAYS				( ( EEPROM_SIZE - BASE_INDEX ) / DATA_SIZE )

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/* tag for debug */
static const char * TAG = "data_logger";

/* variables for the pulses filter */
static uint8_t edge_status = 0;
static TickType_t current_time = 0;
static TickType_t elapsed_time = 0;

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/* isr handlers */
static void IRAM_ATTR pulses_isr( void * arg );
static void IRAM_ATTR alarm_isr( void * arg );

/* tasks functions */
static void pulses_task( void * arg );
static void alarm_task( void * arg );

/* alarm settings function */
static void set_data_frequency( data_logger_t * const me );

/*==================[external functions definition]=========================*/

void data_logger_init( data_logger_t * const me )
{
	/* se inicializa eeprom */
	eeprom_init();

//	uint8_t data = 0;
//	for( uint8_t i = 0x0; i <= 0x7F; i++ )
//		eeprom_write8(i, &data);
//
//	me->rtc.date.date = 0x12;
//	me->rtc.date.month = 0x9;
//	me->rtc.date.year = 0x20;
//	rtc_set_date( &me->rtc );

//	me->rtc.time.hours = 0x20;
//	me->rtc.time.minutes = 0x54;
//	me->rtc.time.seconds = 0x50;
//	rtc_set_time( &me->rtc );

	/* se inicializa la variable para el índice de conteo de pulsos */
	eeprom_read16( CURRENT_INDEX_ADDR, &me->index );
	if( me->index < BASE_INDEX || me->index == 0xFFFF )
	{
		me->index = BASE_INDEX;
		eeprom_write16( BASE_INDEX, &me->index );
	}
	ESP_LOGI( TAG, "index:0x%X", me->index );

	/* se inicializa la variable para el conteo de pulsos */
	eeprom_read16( me->index, &me->pulses );
	if( me->pulses == 0xFFFF )
	{
		me->pulses = 0;
		eeprom_write16( me->index, &me->pulses );
	}
//	me->pulses = 15856;
//	eeprom_write16( me->index, &me->pulses );
	ESP_LOGI( TAG, "pulses:%d", me->pulses );

	/* se inicializa la variable para el conteo total de días monitoreados */
	eeprom_read16( TOTAL_LOGGED_DAYS_ADDR, &me->logged_days );
	if( me->logged_days == 0xFFFF )
	{
		me->logged_days = 0;
		eeprom_write16( TOTAL_LOGGED_DAYS_ADDR, &me->logged_days );
	}
	ESP_LOGI( TAG, "logged_days:%d", me->logged_days );

	/* se obtiene la fecha del rtc y se almacena en la eeprom */
	rtc_get_date( &me->rtc );
	eeprom_write8( me->index + 2, &me->rtc.date.date );
	eeprom_write8( me->index + 3, &me->rtc.date.month );
	eeprom_write8( me->index + 4, &me->rtc.date.year );
	ESP_LOGI( TAG, "date:%02x/%02x/%02x", me->rtc.date.date, me->rtc.date.month, me->rtc.date.year );

	/* se configuran GPIOs */
	ESP_LOGI( TAG, "Configuring pulses GPIO..." );
	gpio_config_t gpio_conf;
	gpio_conf.pin_bit_mask = 1ULL << GPIO_ALARM;
	gpio_conf.mode = GPIO_MODE_INPUT;
	gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
	gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	gpio_conf.intr_type = GPIO_INTR_NEGEDGE;
	gpio_config( &gpio_conf );
	ESP_LOGI( TAG, "Alarm GPIO configured!" );

	/* se configuran GPIOs */
	ESP_LOGI( TAG, "Configuring pulses GPIO..." );
	gpio_conf.pin_bit_mask = 1ULL << GPIO_PULSES;
	gpio_conf.mode = GPIO_MODE_INPUT;
	gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
	gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	gpio_conf.intr_type = GPIO_INTR_POSEDGE;
	gpio_config( &gpio_conf );
	ESP_LOGI( TAG, "Pulses GPIO configured!" );

	/* se configuran las interrupciones y los handlers */
	gpio_install_isr_service( 0 );
	gpio_isr_handler_add( GPIO_PULSES, pulses_isr, ( void * )me );
	gpio_isr_handler_add( GPIO_ALARM, alarm_isr, ( void * )me );

	/* se crean las tareas de data_logger */
	xTaskCreate( pulses_task, "Pulses Task", configMINIMAL_STACK_SIZE * 2, ( void * )me, tskIDLE_PRIORITY + 3, &me->pulses_handle );	/* tarea para el conteo de pulsos */
	xTaskCreate( alarm_task, "Alarm Task", configMINIMAL_STACK_SIZE * 2, ( void * )me, tskIDLE_PRIORITY + 2, &me->alarm_handle );		/* tarea para ejecutar las acciones cuadno ocurre la alarma */
	ESP_LOGI( TAG, "Tasks created!" );

	/* se genera el archivo csv */
	data_logger_get_csv( me );
}

void data_logger_get_csv( data_logger_t * const me )
{
	FILE * f = NULL;

	f = fopen( "/spiffs/kwh.csv", "w" );

	if( f != 0 )
	{
		uint16_t pulses;
		uint8_t date, month, year;

//		ESP_LOGI( TAG, "===============================" );
//		ESP_LOGI( TAG, "address |  date  | kwh" );
//		ESP_LOGI( TAG, "===============================" );
		fprintf( f, "date,kwh\n" );
		for( uint16_t i = BASE_INDEX; i < BASE_INDEX + ( me->logged_days * DATA_SIZE ) + 1; i += DATA_SIZE )
		{
			eeprom_read16( i, &pulses );
			eeprom_read8( i + 2, &date );
			eeprom_read8( i + 3, &month );
			eeprom_read8( i + 4, &year );
			ESP_LOGI( TAG, " E[%03X] | %02x-%02x-%02x | %06.2f", i, date, month, year, pulses * me->settings.pulses_to_kwh );
			fprintf( f, "20%02x-%02x-01,%06.2f\n", year, month, pulses * me->settings.pulses_to_kwh );
		}
		fclose( f );
//		ESP_LOGI( TAG, "===============================" );
	}
}

/*==================[internal functions definition]==========================*/

/* interrupt service routine for incoming digital pulses */
static void IRAM_ATTR pulses_isr( void *arg )
{
	data_logger_t * data_logger = ( data_logger_t * )arg;
	BaseType_t higher_priority_task_woken = pdFALSE;

	if( !edge_status )	/* flanco positivo */
	{
		current_time = xTaskGetTickCountFromISR();
		gpio_set_intr_type( GPIO_PULSES, GPIO_INTR_NEGEDGE );
		edge_status = 1;
	}
	else	/* flanco negativo */
		elapsed_time = xTaskGetTickCountFromISR() - current_time;

	if( elapsed_time >= 10 )
	{
		vTaskNotifyGiveFromISR( data_logger->pulses_handle, &higher_priority_task_woken );
		elapsed_time = 0;
		gpio_set_intr_type( GPIO_PULSES, GPIO_INTR_POSEDGE );
		edge_status = 0;
		portEND_SWITCHING_ISR( higher_priority_task_woken );
	}
}

/* interrupt service routine for incoming RTC alarm */
static void IRAM_ATTR alarm_isr( void *arg )
{
	data_logger_t * data_logger = ( data_logger_t * )arg;
	BaseType_t higher_priority_task_woken = pdFALSE;

	vTaskNotifyGiveFromISR( data_logger->alarm_handle, &higher_priority_task_woken );
	portEND_SWITCHING_ISR( higher_priority_task_woken );
}

/* pulses adquisition task */
static void pulses_task( void * arg )
{
	uint32_t event_to_process;
	data_logger_t * data_logger = ( data_logger_t * )arg;

	for( ;; )
	{
		/* se espera la notificación del isr de los pulsos */
		event_to_process = ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

		if( event_to_process != 0 )
		{
			/* se abre sección crítica */
			taskENTER_CRITICAL();

			/* se aumenta en 1 el conteo de pulsos, se guarda en la EEPROM y se almacena en la eeprom */
			data_logger->pulses++;
			eeprom_write16( data_logger->index, &data_logger->pulses );

			/* se cierra sección crítica */
			taskEXIT_CRITICAL();

			/* se imprime el acumulador de pulsos junto con la fecha y hora */
			ESP_LOGI( TAG, "---------------------------");
			ESP_LOGI( TAG, "[0x%X]=%d", data_logger->index, data_logger->pulses );
			rtc_get_time( &data_logger->rtc );
			ESP_LOGI( TAG, "%02x/%02x/%02x,%02x:%02x:%02x", data_logger->rtc.date.date, data_logger->rtc.date.month, data_logger->rtc.date.year, data_logger->rtc.time.hours, data_logger->rtc.time.minutes, data_logger->rtc.time.seconds );
			ESP_LOGI( TAG, "---------------------------");
		}
	}
}

/* alarm management task */
void alarm_task( void * arg )
{
	uint32_t event_to_process;
	data_logger_t * data_logger = ( data_logger_t * )arg;

	/* se configura la frecuencia de la alarma */
	set_data_frequency( data_logger );

	/* se consulta si la alarma está activa y se notifica a la tarea encargada */
	if( rtc_get_alarm_flag( 1 ) )
		xTaskNotifyGive( data_logger->alarm_handle );

	for( ;; )
	{
		/* se espera la notificación del isr de la alarma */
		event_to_process = ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

		if( event_to_process != 0 )
		{
			ESP_LOGI( TAG, "Alarm!" );

			/* se obtiene e imprime la hora a la que ocurre la alarma */
			rtc_get_time( &data_logger->rtc );
			ESP_LOGI( TAG, "%02X:%02X:%02X",data_logger->rtc.time.hours, data_logger->rtc.time.minutes, data_logger->rtc.time.seconds );

			/* se abre sección crítica */
			taskENTER_CRITICAL();

			/* se manda el acumulador de pulsos por la cola */
			if( data_logger->queue != NULL )
			{
				ESP_LOGI( TAG, "Sending %d to queue", data_logger->pulses );
				xQueueSend( data_logger->queue, &data_logger->pulses, 0 );
			}

			/* se consulta si es un día nuevo */
			if( data_logger->rtc.time.hours == 0x0 && data_logger->rtc.time.minutes == 0x0 && data_logger->rtc.time.seconds == 0x0 )
			{
				ESP_LOGI( TAG, "new day!" );

				/* se aumenta el indice en DATA_SIZE y se guarda en la eeprom */
				data_logger->index += DATA_SIZE;
				if( data_logger->index > ( EEPROM_SIZE - DATA_SIZE ) )
					data_logger->index = BASE_INDEX;
				eeprom_write16( CURRENT_INDEX_ADDR, &data_logger->index );

				/* se pone en 0 el conteo diario de pulsos y se guarda en la memoria eeprom */
				data_logger->pulses = 0;
				eeprom_write16( data_logger->index, &data_logger->pulses );

				/* se obtiene la nueva fecha y se guarda en la eeprom*/
				rtc_get_date( &data_logger->rtc );
				eeprom_write8( data_logger->index + 2, &data_logger->rtc.date.date );
				eeprom_write8( data_logger->index + 3, &data_logger->rtc.date.month );
				eeprom_write8( data_logger->index + 4, &data_logger->rtc.date.year );

				/* se aumenta en 1 el conteo total de dias monitoreados y se guarda en la eeprom */
				if( data_logger->logged_days < MAX_LOGGED_DAYS )
				{
					data_logger->logged_days++;
					eeprom_write16( TOTAL_LOGGED_DAYS_ADDR, &data_logger->logged_days );
				}
			}

			/* se cierra sección crítica */
			taskEXIT_CRITICAL();

			/* se borra el flag de la alarma */
			ESP_LOGI( TAG, "clear flag!" );
			rtc_clear_alarm_flag( 1 );

			data_logger_get_csv( data_logger );
		}
	}
}

/* RTC alarm configuration function */
static void set_data_frequency( data_logger_t * const me )
{
	me->rtc.alarm1.seconds = 0x0;
	me->rtc.alarm1.minutes = 0x0;
	me->rtc.alarm1.hours = 0x0;
	me->rtc.alarm1.daydate = 0x0;

	/* se configura la alarma 1 del RTC con los valores obtenidos
	 * de config.txt */
	switch( me->settings.frequency )
	{
		case 1:
			me->rtc.alarm1.mode = SECONDS_MATCH;
			ESP_LOGI( TAG, "Alarm configured once per minute!" );
			break;
		case 2:
			me->rtc.alarm1.mode = MINUTES_SECONDS_MATCH;
			ESP_LOGI( TAG, "Alarm configured once per hour!" );
			break;
		case 3:
			me->rtc.alarm1.mode = HOURS_MINUTES_SECONDS_MATCH;
			ESP_LOGI( TAG, "Alarm configured once per day!" );
			break;
		default:
			me->rtc.alarm1.mode = HOURS_MINUTES_SECONDS_MATCH;
			ESP_LOGI( TAG, "Alarm configured once per day!" );
			break;
	}

	rtc_set_alarm( &me->rtc, ALARM1 );
	me->rtc.alarm_interrupt_mode = ENABLE_ALARM1;
	rtc_set_alarm_interrupt( &me->rtc );
}


/*==================[end of file]============================================*/
