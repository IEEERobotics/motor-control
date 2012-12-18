/*
 * compass.c
 *
 *  Created on: Dec 17, 2012
 *      Author: eal
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
#include "clock.h"
#include "twi_master_driver.h"
#include "compass.h"

TWI_Master_t twi;


/**
 * Initialize compass module.
 */
void init_compass(void)
{
	// Enable pullup resistors for I2C bus
	COMPASS_TWI_PORT.PIN0CTRL |= PORT_OPC_PULLUP_gc;
	COMPASS_TWI_PORT.PIN1CTRL |= PORT_OPC_PULLUP_gc;

	TWI_MasterInit(&twi,
				   &COMPASS_TWI,
				   TWI_MASTER_INTLVL_MED_gc,
				   TWI_BAUD(CPU_SPEED_HZ, COMPASS_TWI_FREQ));

	// Enable medium priority interrupts
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;
	sei();

	/* Initialize compass state. Consider adding a timeout so we don't get stuck here. */
	while(! compass_wakeup());
	// Continuous mode, 20Hz update frequency
	while(! compass_write_ram(COMPASS_RAM_OPMODE,
			COMPASS_OPMODE_FREQ_20HZ | COMPASS_OPMODE_CONTINUOUS));
}


/**
 * Helper function to send/receive data.
 *
 * @param rx_bytes Number of bytes to receive
 * @param tx_bytes Number of bytes to transmit
 * @param rx_data Pointer to receive data buffer
 * @param tx_data Pointer to data to send
 * @return True if transaction was successful, otherwise false
 */
inline bool send_receive(uint8_t rx_bytes,
						 uint8_t tx_bytes,
						 uint8_t *rx_data,
						 uint8_t *tx_data)
{
	int i;

	TWI_MasterWriteRead(&twi,
						COMPASS_TWI_ADDRESS,
						tx_data,
						rx_bytes,
						tx_bytes);

	while(twi.status != TWIM_STATUS_READY);

	for(i=0; i<rx_bytes; i++)
		rx_data[i] = twi.readData[i];

	return (twi.result == TWIM_RESULT_OK);
}


/**
 * Write a byte to the compass' EEPROM
 * @param address EEPROM address to write to
 * @param data Data to write
 * @return True if transaction successful, otherwise false
 */
bool compass_write_eeprom(uint8_t address, uint8_t data)
{
	uint8_t tx_data[] = {COMPASS_WRITE_EEPROM, address, data};
	return send_receive(0, sizeof(tx_data), NULL, tx_data);
}


/**
 * Read a byte from the compass' EEPROM
 * @param address EEPROM address to read
 * @param data Pointer to received data
 * @return True if transaction successful, otherwise false
 */
bool compass_read_eeprom(uint8_t address, uint8_t *data)
{
	uint8_t tx_data[] = {COMPASS_READ_EEPROM, address};
	return send_receive(0, sizeof(tx_data), data, tx_data);
}


/**
 * Write a byte to the compass' RAM
 * @param address Memory address to read
 * @param data Data to write
 * @return True if transaction successful, otherwise false
 */
bool compass_write_ram(uint8_t address, uint8_t data)
{
	uint8_t tx_data[] = {COMPASS_WRITE_RAM, address, data};
	return send_receive(0, sizeof(tx_data), NULL, tx_data);
}


/**
 * Read a byte from the compass' RAM
 * @param address Memory address to read
 * @param data Pointer to received data
 * @return True if transaction successful, otherwise false
 */
bool compass_read_ram(uint8_t address, uint8_t *data)
{
	uint8_t tx_data[] = {COMPASS_READ_RAM, address};
	return send_receive(0, sizeof(tx_data), data, tx_data);
}


/**
 * Enter compass sleep mode
 * @return True if transaction successful, otherwise false
 */
bool compass_sleep(void)
{
	uint8_t tx_data[] = {COMPASS_SLEEP};
	return send_receive(0, sizeof(tx_data), NULL, tx_data);
}


/**
 * Exit compass sleep mode
 * @return True if transaction successful, otherwise false
 */
bool compass_wakeup(void)
{
	uint8_t tx_data[] = {COMPASS_WAKEUP};
	return send_receive(0, sizeof(tx_data), NULL, tx_data);
}


/**
 * Update compass bridge offsets (not totally sure what this does)
 * @return True if transaction successful, otherwise false
 */
bool compass_update_bridge_offsets(void)
{
	uint8_t tx_data[] = {COMPASS_UPDATE_BRIDGE_OFFSETS};
	return send_receive(0, sizeof(tx_data), NULL, tx_data);
}


/**
 * Enter user calibration mode
 * @return True if transaction successful, otherwise false
 */
bool compass_enter_calibration_mode(void)
{
	uint8_t tx_data[] = {COMPASS_ENTER_CALIBRATION};
	return send_receive(0, sizeof(tx_data), NULL, tx_data);
}


/**
 * Exit user calibration mode
 * @return True if transaction successful, otherwise false
 */
bool compass_exit_calibration_mode(void)
{
	uint8_t tx_data[] = {COMPASS_EXIT_CALIBRATION};
	return send_receive(0, sizeof(tx_data), NULL, tx_data);
}


/**
 * Save the current operation mode to the compass EEPROM
 * @return True if transaction successful, otherwise false
 */
bool compass_save_opmode(void)
{
	uint8_t tx_data[] = {COMPASS_SAVE_OPMODE};
	return send_receive(0, sizeof(tx_data), NULL, tx_data);
}


/**
 * Get data from compass by sending an 'A' command.
 * @param data Pointer to received heading data
 * @return True if transaction successful, otherwise false
 */
bool compass_get_data(uint16_t *data)
{
	uint8_t rx_data[2];
	uint8_t tx_data[] = {COMPASS_GET_DATA};
	bool result = send_receive(2, sizeof(tx_data), rx_data, tx_data);
	*data = rx_data[1] | (rx_data[0] << 8);
	return result;
}


/**
 * Clock out two bytes to read data from compass in continuous and query modes
 * @param data Pointer to received heading data
 * @return True if transaction successful, otherwise false
 */
bool compass_read(uint16_t *data)
{
	uint8_t rx_data[2];
	bool result = send_receive(2, 0, rx_data, NULL);
	*data = rx_data[1] | (rx_data[0] << 8);
	return result;
}


ISR(COMPASS_TWI_VECT)
{
	TWI_MasterInterruptHandler(&twi);
}
