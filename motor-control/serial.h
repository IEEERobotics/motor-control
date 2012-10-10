/**
 * @file
 * @author Ethan LaMaster <ealamast@ncsu.edu>
 * @version 0.1
 *
 * @section Description
 *
 * Functions to initialize the UART, and to parse commands sent over serial.
 */

#ifndef SERIAL_STDIO_H_
#define SERIAL_STDIO_H_

#include <stdio.h>

/**
 * Valid tokens that can be sent over serial. The values correspond to indices in
 * the tokens array in serial.c.
 */
typedef enum token {
	TOKEN_UNDEF		= -1,
	TOKEN_A 		= 0,
	TOKEN_B 		= 1,
	TOKEN_C 		= 2,
	TOKEN_D 		= 3,
	TOKEN_HEADING	= 4,
	TOKEN_HELP 		= 5,
	TOKEN_PWM		= 6,
	TOKEN_SET 		= 7,
	TOKEN_STATUS 	= 8,
} token_t;


void init_serial();
int uart_putchar(char c, FILE *f);
int uart_getchar(FILE *f);
void tolower_str(char *str);
token_t find_token(char *token);
void parse_command(void);
void test_serial_out(void);
void print_banner(void);
void get_command(void);
void print_status(motor_t *motor);
void run_pwm(motor_t *motor, int pwm);
void run_pid(motor_t *motor, int sp);

#endif /* SERIAL_STDIO_H_ */
