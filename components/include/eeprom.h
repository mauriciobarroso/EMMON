/*
 * eeprom.h
 *
 * Created on: Nov 1, 2019
 * Author: Mauricio Barroso
 */


#ifndef _eeprom_H_
#define _eeprom_H_

/*==================[inclusions]=============================================*/

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "esp_err.h"
#include "rom/ets_sys.h"

#include "esp_system.h"
#include "esp_log.h"

#include "driver/i2c.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/* eeprom settings */
#define EEPROM_ADDR					0x57	/*!< device address for eepromx */
#define EEPROM_SIZE					0x1E2	/*!< EEPROM size in bytes (32Kbits) */
#define EEPROM_INITIAL_ADDRESS		0x0		/*!< EEPROM initial addres */

/* i2c settings */
#ifndef I2C_MASTER_SCL_IO
#define I2C_MASTER_SCL_IO			2	 			/*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO           0				/*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0		/*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE   0				/*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0				/*!< I2C master do not need buffer */
#endif

#define WRITE_BIT                   I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                    I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS               0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                     0x0              /*!< I2C ack value */
#define NACK_VAL                    0x1              /*!< I2C nack value */
#define LAST_NACK_VAL               0x2              /*!< I2C last_nack value */

#define MS							1000
#define DELAY			  			( 10 * MS ) // >1.53ms according to datasheet
#define write_cycle_delay()			do { ets_delay_us( DELAY ); } while ( 0 )

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Function to init EEPROM device
 *
 * @param address[in] EEPROM address to be read
 * @param data[out] Pointer to varible that stores the 8-bit value to be read
 * 					of the EEPROM
 *
 * @return
 * 		- true Parameters whithin range
 * 		- false Parameters out of range
 */
void eeprom_init( void );


/**
 * @brief Function to read a 8-bit value of an EEPROM address
 *
 * @param address[in] EEPROM address to be read
 * @param data[out] Pointer to varible that stores the 8-bit value to be read
 * 					of the EEPROM
 *
 * @return
 * 		- true Parameters whithin range
 * 		- false Parameters out of range
 */
uint8_t eeprom_read8( uint16_t address, uint8_t * data );

/**
 * @brief Function to read a 16-bit value of an EEPROM address
 *
 * @param address[in] EEPROM address to be read
 * @param data[out] Pointer to varible that stores the 16-bit value to be read
 * 					of the EEPROM
 *
 * @return
 * 		- true Parameters whithin range
 * 		- false Parameters out of range
 */
uint8_t eeprom_read16( uint16_t address, uint16_t * data );

/**
 * @brief Function to read a 32-bit value of an EEPROM address
 *
 * @param address[in] EEPROM address to be read
 * @param data[out] Pointer to varible that stores the 32-bit value to be read
 * 					of the EEPROM
 *
 * @return
 * 		- true Parameters whithin range
 * 		- false Parameters out of range
 */
uint8_t eeprom_read32( uint16_t address, uint32_t * data );

/**
 * @brief Function to write a 8-bit value to an EEPROM address
 *
 * @param address[in] EEPROM address to be written
 * @param data[in] Pointer to varible that stores the 8-bit value to be written
 * 				   in the EEPROM
 *
 * @return
 * 		- true Parameters whithin range
 * 		- false Parameters out of range
 */
uint8_t eeprom_write8( uint16_t address, uint8_t * data );

/**
 * @brief Function to write a 16-bit value to an EEPROM address
 *
 * @param address[in] EEPROM address to be written
 * @param data[in] Pointer to varible that stores the 16-bit value to be written
 * 				   in the EEPROM
 *
 * @return
 * 		- true Parameters whithin range
 * 		- false Parameters out of range
 */
uint8_t eeprom_write16( uint16_t address, uint16_t * data );

/**
 * @brief Function to write a 32-bit value to an EEPROM address
 *
 * @param address[in] EEPROM address to be written
 * @param data[in] Pointer to varible that stores the 32-bit value to be written
 * 				   in the EEPROM
 *
 * @return
 * 		- true Parameters whithin range
 * 		- false Parameters out of range
 */
uint8_t eeprom_write32( uint16_t address, uint32_t * data );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* #ifndef _eeprom_H_ */
