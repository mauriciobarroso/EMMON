/*
 * control.c
 *
 * Created on: Sep 11, 2020
 * Author: Mauricio Barroso Benavides
 */

/*==================[inlcusions]============================================*/

#include <control.h>

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/*==================[external data declaration]==============================*/

//void control_get_csv( data_logger_t * const me )
//{
//	FILE * f = NULL;
//
//	f = fopen( "/spiffs/kwh.csv", "w" );
//
//	if( f != 0 )
//	{
//		uint16_t pulses;
//		uint8_t date, month, year;
//
////		ESP_LOGI( TAG, "===============================" );
////		ESP_LOGI( TAG, "address |  date  | kwh" );
////		ESP_LOGI( TAG, "===============================" );
//		fprintf( f, "date,kwh\n" );
//		for( uint16_t i = BASE_INDEX; i < BASE_INDEX + ( me->logged_days * DATA_SIZE ) + 1; i += DATA_SIZE )
//		{
////			uint8_t i = BASE_INDEX + ( me->logged_days * DATA_SIZE );
//
//			eeprom_read16( i, &pulses );
//			eeprom_read8( i + 2, &date );
//			eeprom_read8( i + 3, &month );
//			eeprom_read8( i + 4, &year );
//			ESP_LOGI( TAG, " E[%03X] | %02x-%02x-%02x | %06.2f", i, date, month, year, pulses * me->settings.pulses_to_kwh );
//			fprintf( f, "20%02x-%02x-01,%06.2f\n", year, month, pulses * me->settings.pulses_to_kwh );
//		}
//		fclose( f );
////		ESP_LOGI( TAG, "===============================" );
//	}
//}

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]=========================*/

/*==================[internal functions definition]==========================*/

/*==================[end of file]============================================*/
