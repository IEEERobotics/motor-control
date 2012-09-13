/*
 * serial.h
 *
 *  Created on: Jul 2, 2012
 *      Author: eal
 */

#ifndef SERIAL_STDIO_H_
#define SERIAL_STDIO_H_

#include <stdio.h>

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

typedef struct command {
	token_t instruction;
	token_t motor;
	int parameter;
} command_t;

void init_serial();
int uart_putchar(char c, FILE *f);
int uart_getchar(FILE *f);
void tolower_str(char *str);
token_t find_token(char *token);
int parse_command(command_t *cmd);
void execute_command(command_t *cmd);
void test_serial_out(void);

#endif /* SERIAL_STDIO_H_ */
