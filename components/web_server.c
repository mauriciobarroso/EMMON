/*
 * web_server.c
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

/*==================[inlcusions]============================================*/

#include <web_server.h>

/*==================[macros]=================================================*/

/* Max length a file path can have on storage */
#define FILE_PATH_MAX 					( ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN )

#define SCRATCH_BUF_SIZE				8192

#define IS_FILE_EXT( filename, ext )	( strcasecmp( &filename[ strlen( filename ) - sizeof( ext ) + 1 ], ext ) == 0 )

/*==================[typedef]================================================*/



/*==================[internal data declaration]==============================*/

/* */
static const char * TAG = "web_server";

/**/
static EventGroupHandle_t wifi_event_group;
static int s_retry_num = 0;

//static char base_path[ ESP_VFS_PATH_MAX + 1 ];	/**< base path of tile storage */
static char scratch[ SCRATCH_BUF_SIZE ];	/**< scratch buffer for temporary storage during file transfer */

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/* http server functions */
static esp_err_t http_server_init( web_server_t * const me );
static void register_uri_handlers( web_server_t * const me );

/* download files functions */
static esp_err_t download_get_handler( httpd_req_t* req );
static esp_err_t set_content_type_from_file( httpd_req_t* req, const char* filename );
static const char * get_path_from_uri( char* dest, const char* base_path, const char* uri, size_t dest_size );

/* uri handlers */
static esp_err_t set_wifi_data( httpd_req_t * req );

/* wifi connect funcion */
static void wifi_connect( char * buf );

/* wifi event handlers */
static void wifi_event_handler( void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data );
static void ip_event_handler( void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data );

/*==================[external functions definition]=========================*/

void web_server_init( web_server_t * const me )
{
	/* se inicializan los componentes de red */
	nvs_flash_init();
    tcpip_adapter_init();
    esp_netif_init();
    esp_event_loop_create_default();

    /* se inicializa wifi con la configuración por defecto */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init( &cfg );

    /* se registran los handler de los eventos de wifi e ip */
    esp_event_handler_register( WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL );
    esp_event_handler_register( IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, NULL );

    /* se intenta conectar con los datos almacenados en settings.txt */
    wifi_connect( me->settings.wifi_data );

    /* se inicializa el servidor http */
	http_server_init( me );
}


/*==================[internal functions definition]==========================*/

static void wifi_connect( char * buf )
{
	wifi_event_group = xEventGroupCreate();

	char ssid[ 32 ];
	char pass[ 32 ];

	sscanf( buf, "%[^,],%s", ssid, pass );

	/* se definen los parámetros de configuración del modo STA */
	wifi_config_t wifi_config_sta = { 0 };
	strcpy( ( char * )wifi_config_sta.sta.ssid, ssid );
	strcpy( ( char * )wifi_config_sta.sta.password, pass );

	/* se detiene wifi */
	esp_wifi_stop();

	/* se define el modo de wifi en AP y STA */
	ESP_LOGI( TAG, "wifi set to STA mode" );
	esp_wifi_set_mode( WIFI_MODE_STA );

	/* se configuran los parámetros del modo STA */
	esp_wifi_set_config( ESP_IF_WIFI_STA, &wifi_config_sta );

	/* se inicializa wifi */
	esp_wifi_start();

	EventBits_t bits = xEventGroupWaitBits( wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually happened. */
	if( bits & WIFI_CONNECTED_BIT )
		ESP_LOGI( TAG, "Connected to AP with SSID:%s and password:%s", ssid, pass );
	else if ( bits & WIFI_FAIL_BIT )
	{
		ESP_LOGI(TAG, "Failed to connect to AP with SSID:%s and password:%s", ssid, pass);

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
	    esp_wifi_set_mode( WIFI_MODE_AP );

	    /* se configuran los parámetros del modo AP */
		esp_wifi_set_config( ESP_IF_WIFI_AP, &wifi_config_ap );

	    /* se inicializa wifi */
	    esp_wifi_start();

		ESP_LOGI(TAG, "SoftAP mode configured with SSID:%s and password:%s", AP_WIFI_SSID, AP_WIFI_PASS );
	}
	else
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
}

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
				s_retry_num = 0;
				xEventGroupSetBits( wifi_event_group, WIFI_CONNECTED_BIT );
				break;
			}

			default:
				break;
    	}
    }
}

static esp_err_t http_server_init( web_server_t * const me )
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 15;

    /* Start the httpd server */
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if ( httpd_start( &me->server, &config ) != ESP_OK )
    {
        ESP_LOGE(TAG, "Failed to start file server!");
        return ESP_FAIL;
    }

    register_uri_handlers( me->server );
    ESP_LOGI( TAG, "URI handlers registered!" );

    return ESP_OK;
}

static void register_uri_handlers( web_server_t * const me )
{
    /* URI handler for web interface */
	httpd_uri_t uri_handler;

	uri_handler.method    = HTTP_GET;
	uri_handler.handler   = download_get_handler;
	uri_handler.user_ctx  = scratch;

	uri_handler.uri = "/index.html";
	httpd_register_uri_handler( &me->server, &uri_handler );

    /* URI handler to download favicon.ico */
	uri_handler.uri = "/favicon.ico";
	httpd_register_uri_handler( &me->server, &uri_handler );

    /* URI handler to download jquerymobilecss.css */
	uri_handler.uri = "/jquerymobilecss.css";
	httpd_register_uri_handler( &me->server, &uri_handler );

	/* URI handler to download jqueryjs.js */
	uri_handler.uri = "/jqueryjs.js";
	httpd_register_uri_handler( &me->server, &uri_handler );

	/* URI handler to download jquerymobilejs.js */
	uri_handler.uri = "/jquerymobilejs.js";
	httpd_register_uri_handler( &me->server, &uri_handler );

	/* URI handler to download jquerymobilemap.map */
	uri_handler.uri = "/jquerymobilemap.map";
	httpd_register_uri_handler( &me->server, &uri_handler );

	/* URI handler to download highchartsjs.js */
	uri_handler.uri = "/highchartsjs.js";
	httpd_register_uri_handler( &me->server, &uri_handler );

	/* URI handler for download highchartsmap.map */
	uri_handler.uri = "/highchartsmap.map";
	httpd_register_uri_handler( &me->server, &uri_handler );

	/* URI handler to download ajax-loadergif.gif */
	uri_handler.uri = "/ajax-loadergif.gif";
	httpd_register_uri_handler( &me->server, &uri_handler );

    /* URI handler to download kwh.csv */
	uri_handler.uri = "/kwh.csv";
	httpd_register_uri_handler( &me->server, &uri_handler );

	/* URI handler to change wifi settings */
	uri_handler.uri = "/wifi_data";
	uri_handler.method    = HTTP_POST;
	uri_handler.handler   = set_wifi_data;
	uri_handler.user_ctx  = NULL;
	httpd_register_uri_handler( &me->server, &uri_handler );
}

static esp_err_t set_content_type_from_file( httpd_req_t * req, const char* filename )
{
	/* gzip compression */
	#ifdef USE_GZIP
		httpd_resp_set_hdr( req, "Content-Encoding", "gzip" );
	#endif

	/* content type header */
    if ( IS_FILE_EXT( filename, ".pdf" ) )
    	return httpd_resp_set_type( req, "application/pdf" );
    else if( IS_FILE_EXT( filename, ".html" ) )
       	return httpd_resp_set_type( req, "text/html" );
    else if( IS_FILE_EXT( filename, ".jpeg" ) )
    	return httpd_resp_set_type( req, "image/jpeg" );
    else if( IS_FILE_EXT( filename, ".ico" ) )
    	return httpd_resp_set_type( req, "image/x-icon" );
    else if( IS_FILE_EXT( filename, ".css" ) )
    {
    	httpd_resp_set_hdr( req, "Content-Encoding", "gzip" );
		return httpd_resp_set_type( req, "text/css" );
    }
    else if( IS_FILE_EXT( filename, ".js" ) )
    {
    	httpd_resp_set_hdr( req, "Content-Encoding", "gzip" );
    	return httpd_resp_set_type( req, "application/javascript" );
    }
    else if( IS_FILE_EXT( filename, ".map" ) )
    {
    	httpd_resp_set_hdr( req, "Content-Encoding", "gzip" );
    	return httpd_resp_set_type( req, "application/octet-stream" );
    }
    else if( IS_FILE_EXT( filename, ".gif" ) )
    	return httpd_resp_set_type( req, "application/octet-stream" );
    else if( IS_FILE_EXT( filename, ".csv" ) )
    	return httpd_resp_set_type( req, "text/csv" );

    /* for any other type always set as plain text */
    return httpd_resp_set_type( req, "text/plain" );
}

static const char * get_path_from_uri( char * dest, const char* base_path, const char* uri, size_t dest_size )
{
	const size_t base_path_len = strlen( base_path );
	size_t path_len = strlen( uri );

	const char* quest = strchr( uri, '?' );
	if( quest )
		path_len = MIN( path_len, quest - uri );

	const char* hash = strchr( uri, '#' );
		path_len = MIN( path_len, hash - uri );

	if( base_path_len + path_len + 1 > dest_size )
		return NULL;

	strcpy( dest, base_path );
	strlcpy( dest + base_path_len, uri, path_len + 1 );

	return dest + base_path_len;
}

static esp_err_t download_get_handler( httpd_req_t * req )
{
	char filepath[ FILE_PATH_MAX ];
	FILE * f = NULL;
	struct stat file_stat;

	const char * filename = get_path_from_uri( filepath, "/spiffs", req->uri, sizeof( filepath ) );

	char filepath_aux[ FILE_PATH_MAX ];
	strcpy( filepath_aux, filepath );

	/* gzip compression */
	#ifdef USE_GZIP
		uint8_t i = 0;
		for( ; filepath_aux[ i ] != '.'; i++ ){}
		filepath_aux[ i + 1 ] = 'g';
		filepath_aux[ i + 2 ] = 'z';
		filepath_aux[ i + 3 ] = '\0';
	#endif

	char* extension = strchr( filename, '.' );

//	if( !strcmp( extension, ".html" ) )
//	{
//		uint8_t i = 0;
//		for( ; filepath_aux[ i ] != '.'; i++ ){}
//		filepath_aux[ i + 1 ] = 'g';
//		filepath_aux[ i + 2 ] = 'z';
//		filepath_aux[ i + 3 ] = '\0';
//	}
	if( !strcmp( extension, ".js" ) )
	{
		uint8_t i = 0;
		for( ; filepath_aux[ i ] != '.'; i++ ){}
		filepath_aux[ i + 1 ] = 'g';
		filepath_aux[ i + 2 ] = 'z';
		filepath_aux[ i + 3 ] = '\0';
	}
	else if( !strcmp( extension, ".css" ) )
	{
		uint8_t i = 0;
		for( ; filepath_aux[ i ] != '.'; i++ ){}
		filepath_aux[ i + 1 ] = 'g';
		filepath_aux[ i + 2 ] = 'z';
		filepath_aux[ i + 3 ] = '\0';
	}
	else if( !strcmp( extension, ".map" ) )
	{
		uint8_t i = 0;
		for( ; filepath_aux[ i ] != '.'; i++ ){}
		filepath_aux[ i + 1 ] = 'g';
		filepath_aux[ i + 2 ] = 'z';
		filepath_aux[ i + 3 ] = '\0';
	}
//	else if( !strcmp( extension, ".gif" ) )
//	{
//		uint8_t i = 0;
//		for( ; filepath_aux[ i ] != '.'; i++ ){}
//		filepath_aux[ i + 1 ] = 'g';
//		filepath_aux[ i + 2 ] = 'z';
//		filepath_aux[ i + 3 ] = '\0';
//	}

	/* check filename length */
	if( !filename )
	{
		ESP_LOGE( TAG, "Filename is too long" );

		/* Respond with 500 Internal Server Error */
		httpd_resp_send_500( req );
		return ESP_FAIL;
	}

	if( stat( filepath_aux, &file_stat ) == -1 )
	{
        ESP_LOGE( TAG, "Failed to stat file : %s", filepath_aux );
        /* Respond with 404 Not Found */
        httpd_resp_send_404( req );
        return ESP_FAIL;
	}

    f = fopen( filepath_aux, "r" );
    if ( !f )
    {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath_aux);

        /* Respond with 500 Internal Server Error */
        httpd_resp_send_500( req );
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filename, file_stat.st_size);

    set_content_type_from_file( req, filename );
    httpd_resp_set_hdr( req, "Access-Control-Allow-Origin", "*" );


    /* Retrieve the pointer to scratch buffer for temporary storage */
    char * chunk = ( char * )req->user_ctx;
    size_t chunksize;
    do
    {
    	/* Read file in chunks into the scratch buffer */
        chunksize = fread( chunk, 1, SCRATCH_BUF_SIZE, f );

        if( chunksize > 0 )
        {
            /* Send the buffer contents as HTTP response chunk */
            if( httpd_resp_send_chunk( req, chunk, chunksize ) != ESP_OK )
            {
				fclose( f );
				ESP_LOGE( TAG, "File sending failed!" );

				/* Respond with 500 Internal Server Error */
				httpd_resp_send_500( req );
				return ESP_FAIL;
           }
        }
    }
    /* Keep looping till the whole file is sent */
    while( chunksize != 0 );

    /* Close file after sending complete */
    fclose( f );
    ESP_LOGI( TAG, "File sending complete" );

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk( req, NULL, 0 );
    return ESP_OK;
}

static esp_err_t set_wifi_data( httpd_req_t * req )
{

	/* implementart ESPTOUCH!!! */

    char buf[ 65 ];
    int ret, remaining = req->content_len;
    while ( remaining > 0 )
    {
        /* Read the data for the request */
        if ( ( ret = httpd_req_recv(req, buf, MIN( remaining, sizeof( buf ) ) ) ) <= 0 )
        {
            if ( ret == HTTPD_SOCK_ERR_TIMEOUT )
            	continue;	/* retry receiving if timeout occurred */
            return ESP_FAIL;
        }

        buf[ ret ] = '\0';

        wifi_connect( buf );
    }

    return ESP_OK;
}

/*==================[end of file]============================================*/
