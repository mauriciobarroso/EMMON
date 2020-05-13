/*
 * web_interface.c
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

/*==================[inlcusions]============================================*/

#include "webServer.h"

/*==================[macros]=================================================*/

/* Max length a file path can have on storage */
#define FILE_PATH_MAX 					( ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN )

#define SCRATCH_BUFSIZE					8192

#define IS_FILE_EXT( filename, ext )	( strcasecmp( &filename[ strlen( filename ) - sizeof( ext ) + 1 ], ext ) == 0 )

/*==================[typedef]================================================*/

	char base_path[ ESP_VFS_PATH_MAX + 1 ];	/**< base path of tile storage */
	char scratch[ SCRATCH_BUFSIZE ];		/**< scratch buffer for temporary storage during file transfer */

/*==================[internal data declaration]==============================*/

static EventGroupHandle_t wifi_event_group;

/* others */
static int s_retry_num = 0;

static const char *TAG = "webServer";

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static esp_err_t spiffs_init( void );
static void wifi_event_handler( void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data );
static void ip_event_handler( void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data );

static esp_err_t set_content_type_from_file( httpd_req_t* req, const char* filename );
static const char* get_path_from_uri( char* dest, const char* base_path, const char* uri, size_t dest_size );
static esp_err_t download_get_handler( httpd_req_t* req );
static void wifi_init( void );

static esp_err_t post_handler( httpd_req_t * req );
void sta_connect( char * buf, size_t len );
void ap_connect( void );

/*==================[external functions definition]=========================*/

esp_err_t start_webserver( void )
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 9;

    /* Start the httpd server */
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if ( httpd_start( &server, &config ) != ESP_OK )
    {
        ESP_LOGE(TAG, "Failed to start file server!");
        return ESP_FAIL;
    }

    /* URI handler for web interface */
	httpd_uri_t uri_handler;

	uri_handler.uri = "/index.html";
	uri_handler.method    = HTTP_GET;
	uri_handler.handler   = download_get_handler;
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	uri_handler.user_ctx  = scratch;
	httpd_register_uri_handler( server, &uri_handler );

    /* URI handler for web interface */
	uri_handler.uri = "/favicon.ico";
	uri_handler.method    = HTTP_GET;
	uri_handler.handler   = download_get_handler;
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	uri_handler.user_ctx  = scratch;
	httpd_register_uri_handler( server, &uri_handler );

    /* URI handler for web interface */
	uri_handler.uri = "/pulses.txt";
	uri_handler.method    = HTTP_GET;
	uri_handler.handler   = download_get_handler;
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	uri_handler.user_ctx  = scratch;
	httpd_register_uri_handler( server, &uri_handler );

    /* URI handler for web interface */
	uri_handler.uri = "/jquerymobilecss.css";
	uri_handler.method    = HTTP_GET;
	uri_handler.handler   = download_get_handler;
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	uri_handler.user_ctx  = scratch;
	httpd_register_uri_handler( server, &uri_handler );

	/* URI handler for web interface */
	uri_handler.uri = "/jqueryjs.js";
	uri_handler.method    = HTTP_GET;
	uri_handler.handler   = download_get_handler;
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	uri_handler.user_ctx  = scratch;
	httpd_register_uri_handler( server, &uri_handler );

	/* URI handler for web interface */
	uri_handler.uri = "/jquerymobilejs.js";
	uri_handler.method    = HTTP_GET;
	uri_handler.handler   = download_get_handler;
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	uri_handler.user_ctx  = scratch;
	httpd_register_uri_handler( server, &uri_handler );

	/* URI handler for web interface */
	uri_handler.uri = "/jquerymobilemap.map";
	uri_handler.method    = HTTP_GET;
	uri_handler.handler   = download_get_handler;
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	uri_handler.user_ctx  = scratch;
	httpd_register_uri_handler( server, &uri_handler );

	/* URI handler for web interface */
	uri_handler.uri = "/ajax-loadergif.gif";
	uri_handler.method    = HTTP_GET;
	uri_handler.handler   = download_get_handler;
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	uri_handler.user_ctx  = scratch;
	httpd_register_uri_handler( server, &uri_handler );

	/* URI handler for web interface */
	uri_handler.uri = "/wifi";
	uri_handler.method    = HTTP_POST;
	uri_handler.handler   = post_handler;
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	uri_handler.user_ctx  = NULL;
	httpd_register_uri_handler( server, &uri_handler );

    return ESP_OK;
}

void web_interface_init( void )
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK( esp_netif_init() );
    ESP_ERROR_CHECK( esp_event_loop_create_default() );
	ESP_ERROR_CHECK( spiffs_init() );

	wifi_init();

	ESP_ERROR_CHECK( start_webserver() );
}


/*==================[internal functions definition]==========================*/

static esp_err_t spiffs_init( void )
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,   // This decides the maximum number of files that can be created on the storage
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        	ESP_LOGE(TAG, "Failed to mount or format filesystem");
        else if (ret == ESP_ERR_NOT_FOUND)
        	ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        else
        	ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));

        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info( NULL, &total, &used );
    if ( ret != ESP_OK )
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);

//	FILE* f = NULL;
//	f = fopen( "/spiffs/sta_data.txt", "w" );
//	fclose( f );

	return ESP_OK;
}

/* corregir para usar switch */
static esp_err_t set_content_type_from_file( httpd_req_t* req, const char* filename )
{
    if ( IS_FILE_EXT( filename, ".pdf" ) )
    	return httpd_resp_set_type( req, "application/pdf" );
    else if (IS_FILE_EXT( filename, ".html" ))
    {
    	httpd_resp_set_hdr( req, "Content-Encoding", "gzip" );
       	return httpd_resp_set_type( req, "text/html" );
    }
    else if (IS_FILE_EXT( filename, ".jpeg" ) )
    	return httpd_resp_set_type( req, "image/jpeg" );
    else if ( IS_FILE_EXT( filename, ".ico" ) )
    	return httpd_resp_set_type( req, "image/x-icon" );
    else if ( IS_FILE_EXT( filename, ".css" ) )
    {
		httpd_resp_set_hdr( req, "Content-Encoding", "gzip" );
		return httpd_resp_set_type( req, "text/css" );
    }
    else if ( IS_FILE_EXT( filename, ".js" ) )
    {
    	httpd_resp_set_hdr( req, "Content-Encoding", "gzip" );
    	return httpd_resp_set_type( req, "application/javascript" );
    }
    else if ( IS_FILE_EXT( filename, ".map" ) )
    {
    	httpd_resp_set_hdr( req, "Content-Encoding", "gzip" );
    	return httpd_resp_set_type( req, "application/octet-stream" );
    }

    else if ( IS_FILE_EXT( filename, ".gif" ) )
    {
    	httpd_resp_set_hdr( req, "Content-Encoding", "gzip" );
    	return httpd_resp_set_type( req, "application/octet-stream" );
    }

    /* This is a limited set only */
    /* For any other type always set as plain text */
    return httpd_resp_set_type( req, "text/plain" );
}

static const char* get_path_from_uri( char* dest, const char* base_path, const char* uri, size_t dest_size )
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

static esp_err_t download_get_handler( httpd_req_t* req )
{
	char filepath[ FILE_PATH_MAX ];
	FILE * fd = NULL;
	struct stat file_stat;

	const char * filename = get_path_from_uri( filepath, "/spiffs", req->uri, sizeof( filepath ) );

	char filepath_aux[ FILE_PATH_MAX ];
	strcpy( filepath_aux, filepath );

	char* extension = strchr( filename, '.' );

	if( !strcmp( extension, ".html" ) )
	{
		uint8_t i = 0;
		for( ; filepath_aux[ i ] != '.'; i++ ){}
		filepath_aux[ i + 1 ] = 'g';
		filepath_aux[ i + 2 ] = 'z';
		filepath_aux[ i + 3 ] = '\0';
	}
	else if( !strcmp( extension, ".js" ) )
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
	else if( !strcmp( extension, ".gif" ) )
	{
		uint8_t i = 0;
		for( ; filepath_aux[ i ] != '.'; i++ ){}
		filepath_aux[ i + 1 ] = 'g';
		filepath_aux[ i + 2 ] = 'z';
		filepath_aux[ i + 3 ] = '\0';
	}




	if( !filename )
	{
		ESP_LOGE( TAG, "Filename is too long" );

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

    fd = fopen( filepath_aux, "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath_aux);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_500( req );
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filename, file_stat.st_size);

    set_content_type_from_file( req, filename );
    httpd_resp_set_hdr( req, "Access-Control-Allow-Origin", "*" );


    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *chunk = ( char* )req->user_ctx;
    size_t chunksize;
    do {
        /* Read file in chunks into the scratch buffer */
        chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

        if (chunksize > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                fclose(fd);
                ESP_LOGE(TAG, "File sending failed!");

                /* Respond with 500 Internal Server Error */
                httpd_resp_send_500( req );
               return ESP_FAIL;
           }
        }

        /* Keep looping till the whole file is sent */
    } while (chunksize != 0);

    /* Close file after sending complete */
    fclose(fd);
    ESP_LOGI(TAG, "File sending complete");

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk( req, NULL, 0 );
    return ESP_OK;
}

static void wifi_init( void )
{
    tcpip_adapter_init();

    /* se inicializa wifi con la configuración por defecto */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init( &cfg ) );

    /* se registran los handler de los eventos de wifi e ip */
    ESP_ERROR_CHECK(esp_event_handler_register( WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL ) ); // sta
    ESP_ERROR_CHECK(esp_event_handler_register( IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, NULL ) ); // sta

    /* se busca la información del último ap al que se conectó */
	FILE * f = NULL;
	char buf[ 100 ];

//	f = fopen( "/spiffs/sta_data.txt", "w" );
//	memset(buf, 0, sizeof(buf));
//	sprintf(buf, "ssid=CASAwifi&pass=orcobebe");
//	fprintf(f, buf );
//	fclose( f );

	f = fopen( "/spiffs/sta_data.txt", "r" );
	if( f != 0 )
	{
		//char buf[ 100 ];
		fgets( buf, sizeof( buf ), f );
		fclose( f );

        sta_connect( buf, strlen(buf) );
	}
	else
		ap_connect();
}

static esp_err_t post_handler( httpd_req_t* req )
{
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0)
    {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        buf[ ret ] = '\0';

        sta_connect( buf, ret );

        httpd_resp_set_hdr( req, "Access-Control-Allow-Origin", "*" );
        /* Send back the same data */
        httpd_resp_send_chunk(req, buf, ret);
    }

    // End response
    httpd_resp_send_chunk(req, NULL, 0);

    return ESP_OK;
}

void sta_connect( char * buf, size_t len )
{
	wifi_event_group = xEventGroupCreate();

	uint8_t ssid[ 32 ];
	memset(ssid,0,32);
	uint8_t i = 5;
	for( ; buf[ i ] != '&'; i++ )
		ssid[ i - 5 ] = buf[ i ];
	ssid[ i ] = '\0';
	ESP_LOGI(TAG,"ssid:%s",ssid);

	uint8_t pass[ 32 ];
	memset(pass,0,32);
	uint8_t j = i + 6;
	for( ; j < len; j++ )
		pass[ j - (i + 6) ] = buf[ j ];
	pass[ j ] = '\0';
	ESP_LOGI(TAG,"pass:%s",pass);

	/* se definen los parámetros de configuración del modo STA */
	wifi_config_t wifi_config_sta = { 0 };
	strcpy( ( char * )wifi_config_sta.sta.ssid, ( const char *)ssid );
	strcpy( ( char * )wifi_config_sta.sta.password, ( const char *)pass );

	/* se define el modo de wifi en AP y STA */
	ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ) );

	/* se configuran los parámetros del modo STA */
	ESP_ERROR_CHECK( esp_wifi_set_config( ESP_IF_WIFI_STA, &wifi_config_sta ) );

	/* se inicializa wifi */
	ESP_ERROR_CHECK( esp_wifi_start() );

	EventBits_t bits = xEventGroupWaitBits( wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */
	if (bits & WIFI_CONNECTED_BIT)
	{
		FILE * f = NULL;
		f = fopen( "/spiffs/sta_data.txt", "w" );
		sprintf( buf, "ssid=%s&pass=%s", wifi_config_sta.sta.ssid, wifi_config_sta.sta.password );
		fprintf( f, buf );
		fclose( f );

		ESP_LOGI(TAG, "Connected to AP SSID:%s password:%s", ssid, pass);
	}
	else if ( bits & WIFI_FAIL_BIT )
	{
		ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", ssid, pass);

		ap_connect();
	}
	else
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
}

void ap_connect( void )
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
    ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_AP ) );

    /* se configuran los parámetros del modo AP */
	ESP_ERROR_CHECK( esp_wifi_set_config( ESP_IF_WIFI_AP, &wifi_config_ap ) );

    /* se inicializa wifi */
    ESP_ERROR_CHECK( esp_wifi_start() );

	ESP_LOGI(TAG, "SoftAP mode configured with SSID:%s password:%s", AP_WIFI_SSID, AP_WIFI_PASS );
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

			/* evento en modo softap */
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
				ip_event_got_ip_t * event = ( ip_event_got_ip_t * ) event_data;
				ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa( &event->ip_info.ip ) );
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
