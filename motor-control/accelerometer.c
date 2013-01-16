/*
 * accelerometer.c
 *
 *  Created on: Jan 11, 2013
 *      Author: eal
 */


#include <stdbool.h>
#include "i2c.h"
#include "accelerometer.h"


/**
 * Helper function to send/receive data.
 *
 * @param rx_bytes Number of bytes to receive
 * @param tx_bytes Number of bytes to transmit
 * @param rx_data Pointer to receive data buffer
 * @param tx_data Pointer to data to send
 * @return True if transaction was successful, otherwise false
 */
static inline bool send_receive(uint8_t rx_bytes,
						 	 	uint8_t tx_bytes,
						 	 	uint8_t *rx_data,
						 	 	uint8_t *tx_data)
{
	return i2c_send_receive(ACCEL_TWI_ADDRESS,
							rx_bytes,
							tx_bytes,
							rx_data,
							tx_data);
}


void init_accelerometer(void)
{

}
