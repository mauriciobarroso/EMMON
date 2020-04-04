/* Copyright 2020, Mauricio Barroso
 * All rights reserved.
 *
 * This file is part of EMMON.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* Date: 04/02/20 */

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


/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

#ifndef I2C_MASTER_SCL_IO
#define I2C_MASTER_SCL_IO				2                /* gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO           	14               /* gpio number for I2C master data  */
#define I2C_MASTER_NUM              	I2C_NUM_0        /* I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE   	0                /* I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   	0                /* I2C master do not need buffer */
#endif

#define SX127X_REG_FIFO                 0x00
#define SX127X_REG_OP_MODE              0x01
#define SX127X_REG_FRF_MSB              0x06
#define SX127X_REG_FRF_MID              0x07
#define SX127X_REG_FRF_LSB              0x08
#define SX127X_REG_PA_CONFIG            0x09
#define SX127X_REG_LNA                  0x0C
#define SX127X_REG_FIFO_ADDR_PTR        0x0D
#define SX127X_REG_FIFO_TX_BASE_ADDR    0x0E
#define SX127X_REG_FIFO_RX_BASE_ADDR	0x0F
#define SX127X_REG_FIFO_RX_CURRENT_ADDR	0x10
#define SX127X_REG_IRQ_FLAGS            0x12
#define SX127X_REG_RX_NB_BYTES          0x13
#define SX127X_REG_PKT_SNR_VALUE        0x19
#define SX127X_REG_PKT_RSSI_VALUE       0x1A
#define SX127X_REG_MODEM_CONFIG_1       0x1D
#define SX127X_REG_MODEM_CONFIG_2       0x1E
#define SX127X_REG_PREAMBLE_MSB         0x20
#define SX127X_REG_PREAMBLE_LSB         0x21
#define SX127X_REG_PAYLOAD_LENGTH       0x22
#define SX127X_REG_MODEM_CONFIG_3       0x26
#define SX127X_REG_RSSI_WIDEBAND		0x2C
#define SX127X_REG_DETECTION_OPTIMIZE   0x31
#define SX127X_REG_DETECTION_THRESHOLD  0x37
#define SX127X_REG_SYNC_WORD            0x39
#define SX127X_REG_DIO_MAPPING_1        0x40
#define SX127X_REG_VERSION              0x42

#define LONG_RANGE_MODE           		0x80
#define SLEEP                     		0x00
#define STDBY                     		0x01
#define TX                        		0x03
#define RX_CONTINUOUS             		0x05
#define RX_SINGLE                 		0x06
/*
 * PA configuration
 */
#define PA_BOOST                       	0x80

/*
 * IRQ masks
 */
#define TX_DONE_MASK               		0x08
#define PAYLOAD_CRC_ERROR_MASK     		0x20
#define RX_DONE_MASK               		0x40

#define PA_OUTPUT_RFO_PIN              	0
#define PA_OUTPUT_PA_BOOST_PIN         	1

#define TIMEOUT_RESET                  	100

#define SX127X_RESET_PIN				3
#define SX127X_CS_PIN					15
#define SX127X_MOSI_PIN					13
#define SX127X_MISO_PIN					12

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

extern void lora_reset( void );
extern void lora_explicit_header_mode( void );
extern void lora_implicit_header_mode( int size );
extern void lora_idle( void );
extern void lora_sleep( void );
extern void lora_receive( void );
extern void lora_set_tx_power( int level );
extern void lora_set_frequency( long frequency );
extern void lora_set_spreading_factor( int sf );
extern void lora_set_bandwidth( long sbw );
extern void lora_set_coding_rate( int denominator );
extern void lora_set_preamble_length( long length );
extern void lora_set_sync_word( int sw );
extern void lora_enable_crc( void );
extern void lora_disable_crc( void );
extern esp_err_t lora_init( void );
extern void lora_send_packet( uint8_t *buf, int size );
extern int lora_receive_packet( uint8_t *buf, int size );
extern int lora_received( void );
extern int lora_packet_rssi( void );
extern float lora_packet_snr( void );
extern void lora_close( void );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _SX127X_H_ */
