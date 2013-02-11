/*
 * i2c.h
 *
 *  Created on: Jan 12, 2013
 *      Author: eal
 */

#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>
#include <stdbool.h>

#define I2C_TWI_PORT			PORTC
#define I2C_TWI					TWIC
#define I2C_TWI_FREQ			100000
#define I2C_TWI_VECT			TWIC_TWIM_vect

void init_i2c(void);
bool i2c_send_receive(uint8_t address,
				  	  uint8_t rx_bytes,
				  	  uint8_t tx_bytes,
				  	  uint8_t *rx_data,
				  	  uint8_t *tx_data);

#endif /* I2C_H_ */
