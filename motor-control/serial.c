/*
 * serial.c
 *
 *  Created on: Jul 1, 2012
 *      Author: eal
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <avr/io.h>
#include "serial.h"

#define UART USARTC0
#define USE_DOS_NEWLINES
//#define UART_ECHO_ON

/**
 * Delimiter string to pass to strtok for parsing commands
 */
const char *delimiters = " \r\n";

/**
 * Array of valid tokens.
 *
 * The tokens are in strcmp-sorted order, and are all lowercase as the input
 * string will be converted to all lowercase with tolower_str() before parsing.
 *
 * The indices corresponding to each token are defined in enum token. Be sure to
 * keep this up to date as more tokens are added.
 */
const char *tokens[] = { "a",
					   	 "b",
					   	 "c",
					   	 "d",
					   	 "help",
					   	 "run",
					   	 "status"
};

const char *prompt = "> ";
const char *banner = "NCSU IEEE 2012 Motor Controller\n"
					 "Type \"help\" for a list of available commands.";
const char *help = "Not implemented.";
const char *error = "Bad command.";


/**
 * These are the FILE streams that become stdin, stdout, and stderr.
 */
FILE uart_in, uart_out;
//FILE uart_io;


/**
 * Write a character to the UART. This function is connected to the uart_out FILE stream,
 * which is set to stdout and stderr.
 */
int uart_putchar(char c, FILE *f)
{
#ifdef USE_DOS_NEWLINES
	if(c == '\n')
		uart_putchar('\r', stdout);
#endif

	while((UART.STATUS & USART_DREIF_bm) == 0);
	UART.DATA = c;

	return c;
}


/**
 * Read a character from the UART. This function is connected to the uart_in FILE stream,
 * which is set to stdin.
 */
int uart_getchar(FILE *f)
{
#ifdef UART_ECHO_ON
	char c;
#endif
	while((UART.STATUS & USART_RXCIF_bm) == 0);
#ifdef UART_ECHO_ON
	c = UART.DATA;
	uart_putchar(c);
	return c;
#else
	return UART.DATA;
#endif
}


/**
 * Initializes the UART. stdio.h won't work until this function is called.
 */
void init_serial()
{
	//TODO set up port pins
	PORTC.DIRSET = PIN3_bm;
	PORTC.DIRCLR = PIN2_bm;

	UART.CTRLB = USART_RXEN_bm
			   | USART_TXEN_bm;
	UART.CTRLC = USART_CMODE_ASYNCHRONOUS_gc
			   | USART_PMODE_DISABLED_gc
			   | USART_CHSIZE_8BIT_gc;

	// BSEL=2094, BSCALE=-7
	//UART.BAUDCTRLA = 0x2e;
	//UART.BAUDCTRLB = 0x98;

	// BSEL=1047, BSCALE=-6
	//UART.BAUDCTRLA = 0x17;
	//UART.BAUDCTRLB = 0xa4;

	UART.BAUDCTRLA  = 12;
	UART.BAUDCTRLB = 0;

	fdev_setup_stream(&uart_in, NULL, uart_getchar, _FDEV_SETUP_READ);
	fdev_setup_stream(&uart_out, uart_putchar, NULL, _FDEV_SETUP_WRITE);
//	fdev_setup_stream(&uart_io, uart_putchar, uart_getchar, _FDEV_SETUP_RW);

	stdin = &uart_in;
	stdout = &uart_out;
	stderr = &uart_out;
}


/**
 * Convert a string to lowercase.
 *
 * @param str String to convert
 */
void tolower_str(char *str)
{
	while(*str)
	{
		*str = (char) tolower((int) *str);
		str++;
	}
}


/**
 * Compare function used by bsearch in find_token.
 */
static int comp_token(const void *a, const void *b)
{
	return strcmp(*(char * const *)a, *(char * const *)b);
}


/**
 * Find the given token in the tokens array. Returns the index into the tokens array
 * if it was found, or -1 if no match was found.
 *
 * @param token The string to search for
 *
 * @return Index into the tokens array. See enum token, in serial.h.
 */
token_t find_token(char *token)
{
	char *match = (char *) bsearch(&token,
								   tokens,
								   sizeof(tokens)/sizeof(*tokens),
								   sizeof(*tokens),
								   comp_token);

	if(match == NULL)
		return -1;
	else
		return ((int)match - (int)tokens) / sizeof(*tokens);
}


/**
 * Read a line from stdin, and tokenize it into a command_t struct.
 *
 * @param cmd Pointer to the command_t struct
 *
 * @return 0 on success, or nonzero if an EOF is received.
 */
int parse_command(command_t *cmd)
{
	char input[16];
	char *tok;

	if(fgets(input, 16, stdin) == NULL)
		return -1;

	tolower_str(input);

	if((tok = strtok(input, delimiters)) != NULL)
		cmd->instruction = find_token(tok);
	else
		cmd->instruction = TOKEN_UNDEF;

	if((tok = strtok(NULL, delimiters)) != NULL)
		cmd->motor = find_token(tok);
	else
		cmd->motor = TOKEN_UNDEF;

	if((tok = strtok(NULL, delimiters)) != NULL)
		cmd->parameter = atoi(tok);
	else
		cmd->parameter = 0;

	return 0;
}


void execute_command(command_t *cmd)
{
	switch(cmd->instruction)
	{
	case TOKEN_HELP:
		puts(help);
		break;

	case TOKEN_RUN:
		switch(cmd->motor)
		{
		case TOKEN_A:
			break;
		case TOKEN_B:
			break;
		case TOKEN_C:
			break;
		case TOKEN_D:
			break;
		default:
			puts(error);
			break;
		}
		break;

	case TOKEN_STATUS:
		break;

	default:
		puts(error);
		break;
	}
}


void test_serial_out(void)
{
	PORTA.DIRSET = 0xff;

	for(;;)
	{
		PORTA.OUTTGL = 0xff;
		//puts("Hello, world!");
		uart_putchar('a', NULL);
	}
}
