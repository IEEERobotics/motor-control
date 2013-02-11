/*
 * accelerometer.c
 *
 *  Created on: Jan 11, 2013
 *      Author: eal
 */


#include <stdlib.h>
#include <stdbool.h>
#include "i2c.h"
#include "accelerometer.h"


/**
 * Sign extend 12 bit data to 16 bits
 */
static inline int16_t sext_12(uint16_t x)
{
	if(x > 0x7fff)
		x |= 0xf000;

	return x;
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
	return i2c_send_receive(ACCEL_TWI_ADDRESS,
							rx_bytes,
							tx_bytes,
							rx_data,
							tx_data);
}


/**
 * Initializes the accelerometer
 */
void init_accelerometer(void)
{
	uint8_t xyz_data_cfg;

	xyz_data_cfg = ACCEL_GSCALE;
	if(xyz_data_cfg > 8)
		xyz_data_cfg = 8;
	xyz_data_cfg >>= 2;

	while(! accelerometer_standby());
	while(! accelerometer_write_ram(ACCEL_XYZ_DATA_CFG, xyz_data_cfg));
	while(! accelerometer_write_ram(ACCEL_CTRL_REG2, 0x80));
	while(! accelerometer_active());
}


/**
 * Write one byte of data to the accelerometer ram
 *
 * @param address Memory address to write to
 * @param data Data to write
 * @return True if transmission successful, otherwise false
 */
bool accelerometer_write_ram(uint8_t address, uint8_t data)
{
	uint8_t tx_data[] = {address, data};
	return send_receive(0, sizeof(tx_data), NULL, tx_data);
}


/**
 * Read one byte of data from the accelerometer ram
 *
 * @param address Memory address to write to
 * @param rx_data Pointer to save received data
 * @return True if transmission successful, otherwise false
 */
bool accelerometer_read_ram(uint8_t address, uint8_t *rx_data)
{
	uint8_t tx_data[] = {address};
	return send_receive(sizeof(rx_data), sizeof(tx_data), rx_data, tx_data);
}


/**
 * Put accelerometer into standby mode
 */
bool accelerometer_standby(void)
{
	uint8_t data;

	while(! accelerometer_read_ram(ACCEL_CTRL_REG1, &data));
	data &= ~ACCEL_CTRL_REG1_ACTIVE_bm;

	return accelerometer_write_ram(ACCEL_CTRL_REG1, data);
}


/**
 * Put accelerometer into active mode
 */
bool accelerometer_active(void)
{
	uint8_t data;

	while(! accelerometer_read_ram(ACCEL_CTRL_REG1, &data));
	data |= ACCEL_CTRL_REG1_ACTIVE_bm;

	return accelerometer_write_ram(ACCEL_CTRL_REG1, data);
}


/**
 * Poll accelerometer for latest data
 *
 * @param a Pointer to an accelerometer_data_t struct
 * @return True if transaction successful, otherwise false
 */
bool accelerometer_get_data(accelerometer_data_t *a)
{
	uint8_t raw_data[6];
	uint8_t tx_data[] = {ACCEL_OUT_X_MSB};
	bool result_ok;

	result_ok = send_receive(sizeof(raw_data), sizeof(tx_data), raw_data, tx_data);

	if(result_ok)
	{
		a->x = sext_12((raw_data[0] << 4) | (raw_data[1] >> 4));
		a->y = sext_12((raw_data[2] << 4) | (raw_data[3] >> 4));
		a->z = sext_12((raw_data[4] << 4) | (raw_data[5] >> 4));
		return true;
	}

	return false;
}
