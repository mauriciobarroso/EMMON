/*
 * wifi.c
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

/*==================[inlcusions]============================================*/

#include "wifi.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

static const char * TAG = "web_server";
static EventGroupHandle_t wifi_event_group;
static int s_retry_num = 0;

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static void wifi_event_handler( void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data );
static void ip_event_handler( void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data );

/*==================[external functions definition]=========================*/

void wifi_init( char * wifi_data )
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_netif_init() );
    ESP_ERROR_CHECK( esp_event_loop_create_default() );

    /* se inicializa wifi con la configuración por defecto */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init( &cfg ) );

    /* se registran los handler de los eventos de wifi e ip */
    ESP_ERROR_CHECK(esp_event_handler_register( WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL ) ); // sta
    ESP_ERROR_CHECK(esp_event_handler_register( IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, NULL ) ); // sta

    wifi_sta_mode( wifi_data, strlen( wifi_data ) );
}

void wifi_sta_mode( char * buf, size_t len )
{
	wifi_event_group = xEventGroupCreate();

	char ssid[ 32 ];
	char pass[ 32 ];

	sscanf( buf, "%[^,],%s", ssid, pass );

	/* se definen los parámetros de configuración del modo STA */
	wifi_config_t wifi_config_sta = { 0 };
	strcpy( ( char * )wifi_config_sta.sta.ssid, ssid );
	strcpy( ( char * )wifi_config_sta.sta.password, pass );

	/* se define el modo de wifi en AP y STA */
	ESP_LOGI( TAG, "wifi set to STA mode" );
	ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ) );

	/* se configuran los parámetros del modo STA */
	ESP_ERROR_CHECK( esp_wifi_set_config( ESP_IF_WIFI_STA, &wifi_config_sta ) );

	/* se inicializa wifi */
	ESP_ERROR_CHECK( esp_wifi_start() );

	EventBits_t bits = xEventGroupWaitBits( wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually happened. */
	if (bits & WIFI_CONNECTED_BIT)
	{
		/* se edita el archivo de configuración de wifi y si no existe se crea */
		FILE * f = NULL;
//		f = fopen( "/spiffs/sta_data.txt", "w" );
//		sprintf( buf, "%s,%s", wifi_config_sta.sta.ssid, wifi_config_sta.sta.password );
//		fprintf( f, buf );
//		fclose( f );

		f = fopen( "/spiffs/config.txt", "w" );
		if( f != 0 )
		{
			fprintf( f, "4\n0.000625\n%s,%s\n", wifi_config_sta.sta.ssid, wifi_config_sta.sta.password );
			fclose( f );
		}

		ESP_LOGI(TAG, "Connected to AP with SSID:%s and password:%s", ssid, pass);
	}
	else if ( bits & WIFI_FAIL_BIT )
	{
		ESP_LOGI(TAG, "Failed to connect to AP with SSID:%s and password:%s", ssid, pass);

		wifi_ap_mode();
	}
	else
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
}

void wifi_ap_mode( void )
{
    /* se definen los parámetros de configuración del modo AP */
    wifi_config_t wifi_config_ap =
    {
		.ap =
		{
			.ssid = AP_WIFI_SSID,
			.ssid_len = strlen( AP_WIFI_SSID ),
			.password = AP_WIFI_PASS,
			.max_connection = AP_MAX_STA_CONN,
			.authmode = WIFI_AUTH_WPA_WPA2_PSK
		},
    };

    /* se configura el modo AP */
    ESP_LOGI( TAG, "wifi set to SoftAP mode" );
    ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_AP ) );

    /* se configuran los parámetros del modo AP */
	ESP_ERROR_CHECK( esp_wifi_set_config( ESP_IF_WIFI_AP, &wifi_config_ap ) );

    /* se inicializa wifi */
    ESP_ERROR_CHECK( esp_wifi_start() );

	ESP_LOGI(TAG, "SoftAP mode configured with SSID:%s and password:%s", AP_WIFI_SSID, AP_WIFI_PASS );
}
/*==================[internal functions definition]==========================*/

static void wifi_event_handler( void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data )
{
	if( event_base == WIFI_EVENT )
	{
		switch( event_id )
		{
			/* eventos en modo sta */
			case WIFI_EVENT_STA_START:
			{
				esp_wifi_connect();
				break;
			}

			case WIFI_EVENT_STA_DISCONNECTED:
			{
				if ( s_retry_num < ESP_MAX_RETRY )
				{
					esp_wifi_connect();
					s_retry_num++;
					ESP_LOGI( TAG, "retry to connect to the AP" );
				}
				else
					xEventGroupSetBits( wifi_event_group, WIFI_FAIL_BIT );

				ESP_LOGI(TAG,"connect to the AP fail");
				break;
			}

			/* eventos en modo softap */
			case WIFI_EVENT_AP_STACONNECTED:
			{
				wifi_event_ap_staconnected_t* event = ( wifi_event_ap_staconnected_t* ) event_data;
				ESP_LOGI( TAG, "station "MACSTR" join, AID=%d", MAC2STR( event->mac ), event->aid );
				break;
			}

			case WIFI_EVENT_AP_STADISCONNECTED:
			{
		        wifi_event_ap_stadisconnected_t* event = ( wifi_event_ap_stadisconnected_t* ) event_data;
		        ESP_LOGI( TAG, "station "MACSTR" leave, AID=%d", MAC2STR( event->mac ), event->aid );
		        break;
			}

			default:
				break;
		}
	}
}

static void ip_event_handler( void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data )
{
    if ( event_base == IP_EVENT )
    {
    	switch( event_id )
    	{
			case IP_EVENT_STA_GOT_IP:
			{
				//ip_event_got_ip_t * event = ( ip_event_got_ip_t * ) event_data;
				//ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa( &event->ip_info.ip ) );
				s_retry_num = 0;
				xEventGroupSetBits( wifi_event_group, WIFI_CONNECTED_BIT );
				break;
			}

			default:
				break;
    	}
    }
}

/*==================[end of file]============================================*/
