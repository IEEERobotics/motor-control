/*
 * serial_interactive.h
 *
 *  Created on: Dec 22, 2012
 *      Author: eal
 */

#ifndef SERIAL_INTERACTIVE_H_
#define SERIAL_INTERACTIVE_H_

/**
 * Valid tokens that can be sent over serial. The values correspond to indices in
 * the tokens array in serial.c.
 */
typedef enum token {
	TOKEN_UNDEF			= -1,
	TOKEN_A 			= 0,
	TOKEN_B 			= 1,
	TOKEN_C 			= 2,
	TOKEN_D 			= 3,
	TOKEN_HEADING		= 4,
	TOKEN_HEADING_PID	= 5,
	TOKEN_HELP 			= 6,
	TOKEN_MOTOR_PID		= 7,
	TOKEN_PWM			= 8,
	TOKEN_S				= 9,
	TOKEN_SENSORS		= 10,
	TOKEN_SERVO			= 11,
	TOKEN_SET 			= 12,
	TOKEN_STATUS 		= 13,
} token_t;

void test_serial_out(void);
void print_banner(void);
void get_command_interactive(void);

#endif /* SERIAL_INTERACTIVE_H_ */
