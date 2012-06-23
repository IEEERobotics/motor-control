/*
 * serial.h
 *
 *  Created on: Jun 15, 2012
 *      Author: eal
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#define BUFSIZE 64			// Must be a power of 2
//#define USE_UNIX_NEWLINES		// Uncomment to use UNIX line endings

typedef enum command_type {
	COMMAND_RUN,
	COMMAND_STATUS,
	COMMAND_HELP,
	COMMAND_UNKNOWN
} command_type_t;

typedef enum motor_type {
	MOTOR_A,
	MOTOR_B,
	MOTOR_C,
	MOTOR_D,
	MOTOR_INVALID
} motor_type_t;

typedef struct buffer {
	volatile unsigned char head;
	volatile unsigned char tail;
	char *bfr;
	short int size;
} buffer_t;

typedef struct command {
	int token0;
	int token1;
	int setpoint;
} command_t;


#endif /* SERIAL_H_ */
