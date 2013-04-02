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
#include "i2c.h"
#include "debug.h"
#include "compass.h"

uint8_t current_compass_addr = COMPASS_FLAT_TWI_ADDRESS;


static inline void init_single_compass(void)
{
	/* Initialize compass state. Consider adding a timeout so we don't get stuck here. */
	while(! compass_wakeup());
	// Continuous mode, 20Hz update frequency
	while(! compass_write_ram(COMPASS_RAM_OPMODE,
			COMPASS_OPMODE_FREQ_20HZ | COMPASS_OPMODE_CONTINUOUS));
	while(! compass_update_bridge_offsets());
	while(! compass_write_eeprom(COMPASS_EEPROM_NUM_MEASUREMENTS, 4));
}


/**
 * Initialize compass module.
 */
void init_compass(void)
{
	DEBUG_STATUS(DEBUG_INIT_COMPASS);

	compass_set(COMPASS_RAMP);
	init_single_compass();
	compass_set(COMPASS_FLAT);
	init_single_compass();

	DEBUG_CLEAR_STATUS();
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
static inline bool send_receive(uint8_t rx_bytes,
						 	 	uint8_t tx_bytes,
						 	 	uint8_t *rx_data,
						 	 	uint8_t *tx_data)
{
	return i2c_send_receive(current_compass_addr,
							rx_bytes,
							tx_bytes,
							rx_data,
							tx_data);
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
	bool send, receive;

	send = send_receive(0, sizeof(tx_data), NULL, tx_data);
	receive = send_receive(1, 0, data, NULL);

	return send && receive;
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
	bool send, receive;

	send = send_receive(0, sizeof(tx_data), NULL, tx_data);
	receive = send_receive(1, 0, data, NULL);

	return send && receive;
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


void compass_set(compass_t compass)
{
	switch(compass)
	{
	case COMPASS_FLAT:
		current_compass_addr = COMPASS_FLAT_TWI_ADDRESS;
		break;
	case COMPASS_RAMP:
		current_compass_addr = COMPASS_RAMP_TWI_ADDRESS;
		break;
	}
}
