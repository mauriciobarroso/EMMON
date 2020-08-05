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

/*==================[external functions definition]=========================*/

/*==================[internal functions definition]==========================*/

/*==================[end of file]============================================*/
