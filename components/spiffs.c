/*
 * web_interface.c
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

/*==================[inlcusions]============================================*/

#include "spiffs.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

static const char * TAG = "spiffs";

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]=========================*/

esp_err_t spiffs_init( void )
{
	ESP_LOGI( TAG, "Initializing SPIFFS" );

	esp_vfs_spiffs_conf_t conf =
	{
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 15,	/* This decides the maximum number of files that can be created on the storage */
		.format_if_mount_failed = true
	};

	esp_err_t ret = esp_vfs_spiffs_register( &conf );
	if( ret != ESP_OK )
	{
		if ( ret == ESP_FAIL )
			ESP_LOGE( TAG, "Failed to mount or format filesystem" );
		else if ( ret == ESP_ERR_NOT_FOUND )
			ESP_LOGE( TAG, "Failed to find SPIFFS partition" );
		else
			ESP_LOGE( TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name( ret ) );

		return ESP_FAIL;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info( NULL, &total, &used );
	if ( ret != ESP_OK )
	{
		ESP_LOGE( TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name( ret ) );
		return ESP_FAIL;
	}

	ESP_LOGI( TAG, "Partition size: total: %d, used: %d", total, used );

	return ESP_OK;
}

void spiffs_get_settings( spiffs_t * const me )
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
					sscanf( line, "%d", &me->frequency );
					ESP_LOGI( TAG, "data_frequency=%d", me->frequency );
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

/*==================[internal functions definition]==========================*/

/*==================[end of file]============================================*/
