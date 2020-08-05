/*
 * web_interface.c
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */

/*==================[inlcusions]============================================*/

#include <web_interface.h>

/*==================[macros]=================================================*/

/* Max length a file path can have on storage */
#define FILE_PATH_MAX 					( ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN )

#define SCRATCH_BUF_SIZE				8192

#define IS_FILE_EXT( filename, ext )	( strcasecmp( &filename[ strlen( filename ) - sizeof( ext ) + 1 ], ext ) == 0 )

/*==================[typedef]================================================*/



/*==================[internal data declaration]==============================*/

static const char * TAG = "web_server";

//static char base_path[ ESP_VFS_PATH_MAX + 1 ];	/**< base path of tile storage */
static char scratch[ SCRATCH_BUF_SIZE ];	/**< scratch buffer for temporary storage during file transfer */

data_logger_t * aux;

/*==================[external data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static esp_err_t start_webserver( void );
static void register_uri_handlers( httpd_handle_t server );

static esp_err_t set_content_type_from_file( httpd_req_t* req, const char* filename );
static const char * get_path_from_uri( char* dest, const char* base_path, const char* uri, size_t dest_size );
static esp_err_t download_get_handler( httpd_req_t* req );

static esp_err_t set_wifi_data( httpd_req_t * req );
static esp_err_t get_pulses_data( httpd_req_t * req );


/*==================[external functions definition]=========================*/

void web_interface_init( data_logger_t * const me )
{
	ESP_ERROR_CHECK( nvs_flash_init() );
	wifi_init( me->wifi_data );
	ESP_ERROR_CHECK( start_webserver() );

	aux = me;
}


/*==================[internal functions definition]==========================*/

esp_err_t start_webserver( void )
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 15;

    /* Start the httpd server */
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if ( httpd_start( &server, &config ) != ESP_OK )
    {
        ESP_LOGE(TAG, "Failed to start file server!");
        return ESP_FAIL;
    }

    register_uri_handlers( server );
    ESP_LOGI( TAG, "URI handlers registered!" );

    return ESP_OK;
}

static void register_uri_handlers( httpd_handle_t server )
{
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
	uri_handler.uri = "/highchartsjs.js";
	uri_handler.method    = HTTP_GET;
	uri_handler.handler   = download_get_handler;
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	uri_handler.user_ctx  = scratch;
	httpd_register_uri_handler( server, &uri_handler );

	/* URI handler for web interface */
	uri_handler.uri = "/highchartsmap.map";
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
	uri_handler.uri = "/pulses.csv";
	uri_handler.method    = HTTP_GET;
	uri_handler.handler   = get_pulses_data;
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	uri_handler.user_ctx  = scratch;
	httpd_register_uri_handler( server, &uri_handler );

	/* URI handler for web interface */
	uri_handler.uri = "/config.txt";
	uri_handler.method    = HTTP_GET;
	uri_handler.handler   = download_get_handler;
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	uri_handler.user_ctx  = scratch;
	httpd_register_uri_handler( server, &uri_handler );

	/* URI handler for web interface */
	uri_handler.uri = "/wifi_data";
	uri_handler.method    = HTTP_POST;
	uri_handler.handler   = set_wifi_data;
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	uri_handler.user_ctx  = NULL;
	httpd_register_uri_handler( server, &uri_handler );
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

        wifi_sta_mode( buf, ret );
    }

    return ESP_OK;
}

static esp_err_t get_pulses_data( httpd_req_t * req )
{
	data_logger_get_history( aux );

	download_get_handler( req );

	return ESP_OK;
}

/*==================[end of file]============================================*/
