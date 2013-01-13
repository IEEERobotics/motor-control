/*
 * compass.h
 *
 *  Created on: Dec 17, 2012
 *      Author: eal
 */

#ifndef COMPASS_H_
#define COMPASS_H_

#include <avr/io.h>
#include <stdbool.h>

#define COMPASS_TWI_ADDRESS				(0x42 >> 1)

// Compass command bytes
#define COMPASS_WRITE_EEPROM			'w'
#define COMPASS_READ_EEPROM				'r'
#define COMPASS_WRITE_RAM				'G'
#define COMPASS_READ_RAM				'g'
#define COMPASS_SLEEP					'S'
#define COMPASS_WAKEUP					'W'
#define COMPASS_UPDATE_BRIDGE_OFFSETS	'O'
#define COMPASS_ENTER_CALIBRATION		'C'
#define COMPASS_EXIT_CALIBRATION		'E'
#define COMPASS_SAVE_OPMODE				'L'
#define COMPASS_GET_DATA				'A'

// RAM addresses
#define COMPASS_RAM_OPMODE				0x74	// Operation mode byte (shadowed in EEPROM)
#define COMPASS_RAM_OUTMODE				0x4e	// Output mode byte

// EEPROM addresses
#define COMPASS_EEPROM_I2C_ADDRESS		0x00	// I2C slave address, default is 0x42
#define COMPASS_EEPROM_XOFFMSB			0x01	// X offset MSB (set at factory)
#define COMPASS_EEPROM_XOFFLSB			0x02	// X offset LSB
#define COMPASS_EEPROM_YOFFMSB			0x03	// Y offset MSB
#define COMPASS_EEPROM_YOFFLSB			0x04	// Y offset LSB
#define COMPASS_EEPROM_TIME_DELAY		0x05	// See datasheet (default is no delay)
#define COMPASS_EEPROM_NUM_MEASUREMENTS	0x06	// See datasheet (default is 4)
#define COMPASS_EEPROM_SOFTWARE_VER		0x07	// Software version
#define COMPASS_EEPROM_OPMODE			0x08	// Operation mode byte

// Operation mode bitmasks
#define COMPASS_OPMODE_FREQ_1HZ			(0<<5)	// Continuous mode measurement rate
#define COMPASS_OPMODE_FREQ_5HZ			(1<<5)
#define COMPASS_OPMODE_FREQ_10HZ		(2<<5)
#define COMPASS_OPMODE_FREQ_20HZ		(3<<5)
#define COMPASS_OPMODE_PER_SR_			(1<<4)	// Enable periodic set/reset
#define COMPASS_OPMODE_STANDBY			(0<<0)	// Wait for 'A' command (default)
#define COMPASS_OPMODE_QUERY			(1<<0)	// Perform measurement after each read
#define COMPASS_OPMODE_CONTINUOUS		(2<<0)	// Perform measurement at regular intervals

// Output mode bitmasks
#define COMPASS_OUTMODE_HEADING			0		// Default
#define COMPASS_OUTMODE_RAWX			1
#define COMPASS_OUTMODE_RAWY			2
#define COMPASS_OUTMODE_X				3
#define COMPASS_OUTMODE_Y				4

void init_compass(void);
bool compass_write_eeprom(uint8_t address, uint8_t data);
bool compass_read_eeprom(uint8_t address, uint8_t *data);
bool compass_write_ram(uint8_t address, uint8_t data);
bool compass_read_ram(uint8_t address, uint8_t *data);
bool compass_sleep(void);
bool compass_wakeup(void);
bool compass_update_bridge_offsets(void);
bool compass_enter_calibration_mode(void);
bool compass_exit_calibration_mode(void);
bool compass_save_opmode(void);
bool compass_get_data(uint16_t *data);
bool compass_read(uint16_t *data);


#endif /* COMPASS_H_ */
