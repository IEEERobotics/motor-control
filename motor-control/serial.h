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
	TOKEN_HELP 		= 4,
	TOKEN_RUN 		= 5,
	TOKEN_STATUS 	= 6,
} token_t;


/**
 * This represents a single serial command.
 */
typedef struct command {
	token_t instruction;	// Instruction identifier
	token_t motor;			// Motor to operate on. May not apply for some instructions, i.e. 'help'
	int parameter;			// Integer parameter, i.e. target speed
} command_t;

void init_serial();
int uart_putchar(char c, FILE *f);
int uart_getchar(FILE *f);
void tolower_str(char *str);
token_t find_token(char *token);
int parse_command(command_t *cmd);
void execute_command(command_t *cmd);
void test_serial_out(void);
void print_banner(void);
void get_command(void);

#endif /* SERIAL_STDIO_H_ */
