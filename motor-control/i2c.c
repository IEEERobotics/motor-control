/*
 * i2c.c
 *
 *  Created on: Jan 12, 2013
 *      Author: eal
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "clock.h"
#include "twi_master_driver.h"
#include "i2c.h"


TWI_Master_t twi;


/**
 * Initialize the I2C bus
 */
void init_i2c(void)
{
	// Enable pullup resistors for I2C bus
//	I2C_TWI_PORT.PIN0CTRL |= PORT_OPC_PULLUP_gc;
//	I2C_TWI_PORT.PIN1CTRL |= PORT_OPC_PULLUP_gc;

	TWI_MasterInit(&twi,
				   &I2C_TWI,
				   TWI_MASTER_INTLVL_MED_gc,
				   TWI_BAUD(CPU_SPEED_HZ, I2C_TWI_FREQ));

	// Enable medium priority interrupts
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;
	sei();
}


/**
 * Function to send/receive data over the I2C bus.
 *
 * @param address I2C address to use, not including the send/receive bit
 * @param rx_bytes Number of bytes to receive
 * @param tx_bytes Number of bytes to transmit
 * @param rx_data Pointer to receive data buffer
 * @param tx_data Pointer to data to send
 * @return True if transaction was successful, otherwise false
 */
bool i2c_send_receive(uint8_t address,
				  	  uint8_t rx_bytes,
				  	  uint8_t tx_bytes,
				  	  uint8_t *rx_data,
				  	  uint8_t *tx_data)
{
	int i;

	if(TWI_MasterWriteRead(&twi,
						   address,
						   tx_data,
						   tx_bytes,
						   rx_bytes))
	{
		while(twi.status != TWIM_STATUS_READY);

		if(twi.result == TWIM_RESULT_OK)
		{
			for(i=0; i<rx_bytes; i++)
				rx_data[i] = twi.readData[i];

			return true;
		}
	}

	return false;
}


ISR(I2C_TWI_VECT)
{
	TWI_MasterInterruptHandler(&twi);
}
