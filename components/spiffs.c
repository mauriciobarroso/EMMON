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
		.max_files = 5,	/* This decides the maximum number of files that can be created on the storage */
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

//	FILE * f = NULL;
//	ESP_LOGI( TAG,"OK" );
//	f = fopen( "/spiffs/index.html", "w" );
//	fprintf( f, "<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>EMMON - Energy Meter Monitor</title><link rel=\"stylesheet\" href=\"jquerymobilecss.css\"> <script src=\"jqueryjs.js\"></script> <script src=\"jquerymobilejs.js\"></script> <script>var kwh_constant=0.02;var price_constant=0.1;</script> <script>$(document).ready(function(){$(\"#ref\").click(function(){$.get(\"http://192.168.1.114/pulses.txt\",function(data){data=parseInt(data)*kwh_constant*1.5;$(\"#kwh\").text(\"Consumo: \"+data.toFixed(2)+\" Kwh\");data=data*price_constant*1.5;$(\"#price\").text(\"Costo: \"+data.toFixed(2)+\" Bs\");});});$('#send-wifi-conf').click(function(){if($('#ssid').val()!=0) {$.post(\"http://192.168.1.114/wifi\",$('#wifi-form').serialize(),function(data){alert(data);});}});});$(document).on(\"pagecreate\",\"#main\",function(){$.get(\"http://192.168.1.114/pulses.txt\",function(data){data=parseInt(data)*kwh_constant*1.5;$(\"#kwh\").text(\"Consumo: \"+data.toFixed(2)+\" Kwh\");data=data*price_constant*1.5;$(\"#price\").text(\"Costo: \"+data.toFixed(2)+\" Bs\");});});</script> <style>.ui-mobile label.labelfs{left:0 !important;float:left;margin-right:10px;margin-top:10px}.ui-page{background:#e9e9e9}.ui-field-contain>label~[class*=ui-], .ui-field-contain .ui-controlgroup-controls{margin:4px}.modal{display:none;position:fixed;z-index:1000;top:0;left:0;height:100%%;width:100%%;background:rgba( 255, 255, 255, .8 ) url(\"ajax-loadergif.gif\") 50%% 50%% no-repeat}body.loading{overflow:hidden}body.loading .modal{display:block}</style></head><body><div data-role=\"page\" id=\"main\"><div data-role=\"header\" data-position=\"fixed\" data-theme=\"b\"><h1>EMMON v1.0.0</h1> <a href=\"#wifi\" class=\"ui-btn ui-btn-left ui-nodisc-icon ui-corner-all ui-btn-icon-notext ui-icon-gear\">Settings</a></div><div data-role=\"main\" class=\"ui-content\"><ul data-role=\"listview\"><li class=\"ui-field-contain\"> <input type=\"button\" value=\"Actualizar datos\" id=\"ref\" data-icon=\"refresh\"><h1 align=\"center\" style=\"font-size:60px;\" id=\"kwh\"></h1><h1 align=\"center\" style=\"font-size:60px;\" id=\"price\"></h1></li></ul></div><div data-role=\"footer\" data-position=\"fixed\" data-theme=\"b\"><h1>Mauricio Barroso 2020</h1></div></div><div data-role=\"page\" id=\"wifi\"><div data-role=\"header\" data-position=\"fixed\" data-theme=\"b\"><h1>Access Point</h1> <a href=\"#main\" class=\"ui-btn ui-btn-left ui-nodisc-icon ui-corner-all ui-btn-icon-notext ui-icon-back\">Back</a> <a class=\"ui-btn ui-btn-right ui-nodisc-icon ui-corner-all ui-btn-icon-notext ui-icon-check\" id=\"send-wifi-conf\">Save</a></div><div data-role=\"main\" class=\"ui-content\"><ul data-role=\"listview\" class=\"ui-alt-icon ui-nodisc-icon\"><li data-role=\"list-divider\"><p>Ingrese el el nombre ya la contraseña de la red Wi-Fi a la que quiera conectarse.</p></li><li><form id=\"wifi-form\"> <label for=\"ssid\">SSID:</label> <input type=\"text\" data-clear-btn=\"true\" name=\"ssid\" id=\"ssid\" value=\"\" autocomplete=\"off\" maxlength=\"20\" class=\"input-ap\"> <label for=\"pass\">Password:</label> <input type=\"text\" data-clear-btn=\"true\" name=\"pass\" id=\"pass\" value=\"\" autocomplete=\"off\" maxlength=\"20\" class=\"input-ap\"></form></li></ul></div></div><div class=\"modal\"></div></body></html>" );
//	fclose( f );

	return ESP_OK;
}

/*==================[external functions definition]=========================*/

/*==================[internal functions definition]==========================*/

/*==================[end of file]============================================*/
