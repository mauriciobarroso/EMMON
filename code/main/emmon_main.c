/*
 * user_main.c
 *
 *  Created on: Nov 1, 2019
 *      Author: Mauricio Barroso
 */

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/unistd.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_log.h"

#include "driver/gpio.h"

#include "dataLogger.h"
#include "webServer.h"
#include "lcd20x4.h"

/*==================[macros]=================================================*/

/* definicion de pines de interrupcion */
#define GPIO_METER_PULSES	4
#define GPIO_RTC_ALARM    	5
#define GPIO_INTERRUPT_PINS	( ( 1ULL << GPIO_METER_PULSES ) | ( 1ULL << GPIO_RTC_ALARM ) )

/*==================[typedef]================================================*/

/*==================[data declaration]================================================*/

static const char *TAG = "EMMON";
DataLogger_t xData;

/* handler tasks */
TaskHandle_t xPulseCounter;
TaskHandle_t xSendToLora;

/*==================[function declaration ]================================================*/

void vAliveTask( void *pvParameters );
void vPulseCountHandler( void *pvParameters );
void vSendDataToLoraHandler( void *pvParameters );
void IRAM_ATTR vRTCAlarmISRHandler( void *pvParameters );
void IRAM_ATTR vMeterPulsesISRHandler( void *pvParameters );
void vGpioInterrupPinsConfig( void *pvParameters );

/*========================[main]=============================================*/

void app_main()
{
	vGpioInterrupPinsConfig( ( void * )GPIO_INTERRUPT_PINS );
	vDataLoggerInit( &xData );
	lcd_init();

	vWebServerInit();

    xTaskCreate( vAliveTask, "Alive", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 2, NULL );
	xTaskCreate( vPulseCountHandler, "Pulse Count Task", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 2, &xPulseCounter );
	xTaskCreate( vSendDataToLoraHandler, "Send Task", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 2, &xSendToLora );

	/* el scheduler es iniciado en las tareas de inicialización del esp8266 */
}

/*==================[function definition ]================================================*/

void vAliveTask( void *pvParameters )
{
	TickType_t xLastWakeTime;
	char buffer[ 20 ];

	xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		vDataLoggerSetCount( xData.usIndex, ++xData.usCount );

		ds3231_get_date( &xData.xRtc );
		ds3231_get_time( &xData.xRtc );

		sprintf( buffer, "%02x/%02x/%02x    %02x:%02x:%02x", xData.xRtc.date.date, xData.xRtc.date.month, xData.xRtc.date.year, xData.xRtc.time.hours, xData.xRtc.time.minutes, xData.xRtc.time.seconds );
		lcd_gotoxy( 1, 1 );
		lcd_puts( buffer );

		sprintf( buffer, "DAYS=%02d", xData.usDaysCount );
		lcd_gotoxy( 1, 2 );
		lcd_puts( buffer );

		sprintf( buffer, "E[0x%02X]=%04d", xData.usIndex, xData.usCount );
		lcd_gotoxy( 1, 3 );
		lcd_puts( buffer );

		if( get_alarm1_flag() )
		{
			clear_alarm1_flag();
			xTaskNotifyGive( xSendToLora );
		}

		vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 1000 ) );
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
			/* borra el flag de la alarma del RTC */
			clear_alarm1_flag();
			/* aumenta en 1 el conteo de pulsos */
			vDataLoggerSetCount( xData.usIndex, ++xData.usCount );

			ESP_LOGI( TAG, "Pulse count data = %d", xData.usCount );
		}
	}
}

void vSendDataToLoraHandler( void *pvParameters )
{
	uint32_t ulEventToProcess;
	char buffer[ 20 ];

	for( ;; )
	{
		ulEventToProcess = ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
		if( ulEventToProcess != 0 )
		{
			/* sección crítica? */
			ds3231_get_date( &xData.xRtc );
			sprintf( buffer, "%04d,%02x%02x%02x,%08X", xData.usCount, xData.xRtc.date.date, xData.xRtc.date.month, xData.xRtc.date.year, xData.ulID );
			lcd_gotoxy( 1, 4 );
			lcd_puts( buffer );

			vDataLoggerSetCurrentDate();
			vDataLoggerSetDaysCount( ++xData.usDaysCount );
			vDataLoggerSetIndex( xData.usIndex += 5 );
			xData.usCount = 0;
			// enviar ucBuffer en una cola a la tarea que envia por LoraWAN
		}
	}
}

void IRAM_ATTR vRTCAlarmISRHandler( void *pvParameters )
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR( vSendDataToLoraHandler, &xHigherPriorityTaskWoken );
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

void IRAM_ATTR vMeterPulsesISRHandler( void *pvParameters )
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
	gpio_isr_handler_add( GPIO_METER_PULSES, vMeterPulsesISRHandler, NULL );
	gpio_isr_handler_add( GPIO_RTC_ALARM, vRTCAlarmISRHandler, NULL );
}

