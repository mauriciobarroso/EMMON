/* Mauricio Barroso
 * Date: 04/02/20
 */

#ifndef _SX127X_H_
#define _SX127X_H_

/*==================[inclusions]=============================================*/

#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "driver/spi.h"
#include "driver/gpio.h"

#include "esp_system.h"
#include "esp_log.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

#define PA_OUTPUT_RFO_PIN       0
#define PA_OUTPUT_PA_BOOST_PIN	1

#define SX127X_RESET_PIN		16
#define SX127X_RESET_PIN_MASK	( 1ULL << SX127X_RESET_PIN )
//#define SX127X_DIO0_PIN			4
//#define SX127X_DIO0_PIN_MASK	( 1ULL << SX127X_DIO0_PIN )

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

int lora_begin( long frequency );
void lora_end( void );
int lora_begin_packet( int implicitHeader );	//false
int lora_end_packet( bool async );	//false
int lora_parse_packet( int size );	// 0
int lora_packet_rssi( void );
float lora_packet_snr( void );
//long lora_packetFrequencyError();
size_t lora_write(const uint8_t *buffer, size_t size);
int lora_available( void );
int lora_read( void );
int lora_peek( void );
void lora_flush( void );
void lora_receive(int size);	// 0
void lora_idle( void );
void lora_sleep( void );
void lora_set_tx_power( int level, int output_pin ); // PA_OUTPUT_PA_BOOST_PIN
void lora_set_frequency( long frequency );
void lora_set_spreading_factor( int sf );
void lora_set_signal_bandwidth( long sbw );
void lora_set_coding_rate4( int denominator );
void lora_set_preamble_length( long length );
void lora_set_sync_word( int sw );
void lora_enable_crc( void );
void lora_disable_crc( void );
void lora_enable_invert_iq( void );
void lora_disable_invert_iq( void );
void lora_set_ocp( uint8_t current ); // Over Current Protection control
uint8_t lora_random( void );

void lora_send_packet( char * buf, int size );
int lora_receive_packet( char * buf, int size );
int lora_received( void );
void lora_recv( void );
/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _SX127X_H_ */
