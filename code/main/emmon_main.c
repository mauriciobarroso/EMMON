/*
 * user_main.c
 *
 *  Created on: Nov 1, 2019
 *      Author: Mauricio Barroso
 */

/*==================[inclusions]=============================================*/
#include <sys/param.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <esp_http_server.h>

#include "driver/gpio.h"

#include "dataLogger.h"

/*==================[macros]=================================================*/
/* definicion de pines de interrupcion */
#define GPIO_METER_PULSES	4
#define GPIO_RTC_ALARM    	5
#define GPIO_INTERRUPT_PINS	( ( 1ULL << GPIO_METER_PULSES ) | ( 1ULL << GPIO_RTC_ALARM ) )

/*==================[typedef]================================================*/

/*==================[data declaration]================================================*/

const char *TAG = "EMMON";

DataLogger_t xData;
static httpd_handle_t server = NULL;

/*==================[function declaration ]================================================*/

void vAliveTask( void *pvParameters );
void vPulseCountHandler( void *pvParameters );
void vSendDataToLoraHandler( void *pvParameters );
void IRAM_ATTR xRTCAlarmISRHandler( void *pvParameters );
void IRAM_ATTR xMeterPulsesISRHandler( void *pvParameters );
void vGpioInterrupPinsConfig( void *pvParameters );

esp_err_t hello_get_handler(httpd_req_t *req);
esp_err_t echo_post_handler(httpd_req_t *req);
esp_err_t ctrl_put_handler(httpd_req_t *req);
httpd_handle_t start_webserver(void);
void stop_webserver(httpd_handle_t server);
static void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

/*========================[main]=============================================*/

void app_main()
{
	vGpioInterrupPinsConfig( ( void * )GPIO_INTERRUPT_PINS );
	vDataLoggerInit( &xData );

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect());

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
    server = start_webserver();

    //xTaskCreate( vAliveTask, "Alive", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 2, NULL );
	xTaskCreate( vPulseCountHandler, "Pulse Count Task", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 2, NULL );
	xTaskCreate( vSendDataToLoraHandler, "Send Task", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 2, NULL );
	/* el scheduler es iniciado en las tareas de inicialización del esp8266 */
}

/*==================[function definition ]================================================*/

void vAliveTask( void *pvParameters )
{
	for( ;; )
	{
		xData.usCount++;
		vDataLoggerSetCount( xData.usIndex, xData.usCount );

		ESP_LOGI( TAG, "EEPROM[0x%X] = %d", xData.usIndex, xData.usCount );

		vTaskDelay( pdMS_TO_TICKS( 1000 ) );
	}
}

void vPulseCountHandler( void *pvParameters )
{
	uint32_t ulEventToProcess;

	for( ;; )
	{
		ulEventToProcess = ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

		if( ulEventToProcess != 0 )
		{
			vDataLoggerSetCount( xData.usIndex, ++xData.usCount );

			ESP_LOGI( TAG, "Pulse count data = %d", xData.usCount );
		}
	}
}

void vSendDataToLoraHandler( void *pvParameters )
{
	uint32_t ulEventToProcess;
	char ucBuffer[] = "PULSES=XXXXX,DATE=XXXXXX,ID=XXXXXXXXCF=XXXX";

	for( ;; )
	{
		ulEventToProcess = ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
		if( ulEventToProcess != 0 )
		{
			sprintf( ucBuffer, "PULSES=%05d,DATE=XXXXXX,ID=0x%08X,CF=XXXX", xData.usCount, xData.ulID );

			//vDataLoggerSetCurrentCountIndex( xData.usCountIndex += 2 );
			//vDataLoggerSetDateIndex( xData.usDateIndex += 3 );
			xData.usCount = 0;
			// xData.usDate = usDataLoggerGetDate();
			// enviar ucBuffer en una cola a la tarea que envia por LoraWAN
		}
	}
}

void IRAM_ATTR xRTCAlarmISRHandler( void *pvParameters )
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR( vSendDataToLoraHandler, &xHigherPriorityTaskWoken );
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

void IRAM_ATTR xMeterPulsesISRHandler( void *pvParameters )
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR( vPulseCountHandler, &xHigherPriorityTaskWoken );
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

void vGpioInterrupPinsConfig( void *pvParameters )
{
	gpio_config_t xConfig;
	uint32_t ulInterruptPins = ( uint32_t )pvParameters;

	xConfig.pin_bit_mask = ulInterruptPins;
	xConfig.mode = GPIO_MODE_INPUT;
	xConfig.pull_up_en = GPIO_PULLUP_ENABLE;
	xConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
	xConfig.intr_type = GPIO_INTR_NEGEDGE;
	gpio_config( &xConfig );

	gpio_install_isr_service( 0 );
	gpio_isr_handler_add( GPIO_METER_PULSES, xMeterPulsesISRHandler, NULL );
	gpio_isr_handler_add( GPIO_RTC_ALARM, xRTCAlarmISRHandler, NULL );
}

/* An HTTP GET handler */
esp_err_t hello_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-2: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
        }
        free(buf);
    }

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
            }
            if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
            }
            if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
            }
        }
        free(buf);
    }

    /* Set some custom headers */
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

httpd_uri_t hello = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "Hello World!"
};

/* An HTTP POST handler */
esp_err_t echo_post_handler(httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        /* Log data received */
        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        ESP_LOGI(TAG, "%.*s", ret, buf);
        ESP_LOGI(TAG, "====================================");
    }

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

httpd_uri_t echo = {
    .uri       = "/echo",
    .method    = HTTP_POST,
    .handler   = echo_post_handler,
    .user_ctx  = NULL
};

/* An HTTP PUT handler. This demonstrates realtime
 * registration and deregistration of URI handlers
 */
esp_err_t ctrl_put_handler(httpd_req_t *req)
{
    char buf;
    int ret;

    if ((ret = httpd_req_recv(req, &buf, 1)) <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    if (buf == '0') {
        /* Handler can be unregistered using the uri string */
        ESP_LOGI(TAG, "Unregistering /hello and /echo URIs");
        httpd_unregister_uri(req->handle, "/hello");
        httpd_unregister_uri(req->handle, "/echo");
    }
    else {
        ESP_LOGI(TAG, "Registering /hello and /echo URIs");
        httpd_register_uri_handler(req->handle, &hello);
        httpd_register_uri_handler(req->handle, &echo);
    }

    /* Respond with empty body */
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

httpd_uri_t ctrl = {
    .uri       = "/ctrl",
    .method    = HTTP_PUT,
    .handler   = ctrl_put_handler,
    .user_ctx  = NULL
};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &echo);
        httpd_register_uri_handler(server, &ctrl);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}
