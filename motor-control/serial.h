/*
 * serial.h
 *
 *  Created on: Jun 15, 2012
 *      Author: eal
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#define SERIAL_UNITTEST_ENABLE

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


void init_serial();
int bfr_readln(volatile buffer_t *buffer, char *line, int size);
void bfr_putln(volatile buffer_t *buffer, const char *line, int print_prompt);
void parse_line(command_t *c);
void run_command(command_t *c);

#ifdef SERIAL_UNITTEST_ENABLE
void test_serial();
#endif


#endif /* SERIAL_H_ */
