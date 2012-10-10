/**
 * @file
 * @author Ethan LaMaster <ealamast@ncsu.edu>
 * @version 0.1
 *
 * @section Description
 *
 * Functions to initialize the UART, and to parse commands sent over serial.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <avr/io.h>
#include "motor.h"
#include "serial.h"

#define UART USARTC0
#define USE_DOS_NEWLINES		// Use CRLF line endings
#define UART_ECHO_ON			// Echo received characters back to the terminal


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
					   	 "heading",
					   	 "help",
					   	 "pwm",
					   	 "set",
					   	 "status"
};

const char *prompt = "> ";
const char *banner = "NCSU IEEE 2012 Motor Controller\n"
					 "Type \"help\" for a list of available commands.";
const char *help = "Not implemented.";
const char *error = "Bad command.";
const char *bad_motor = "Bad motor.";


/**
 * These are the FILE streams that become stdin, stdout, and stderr.
 */
FILE uart_in, uart_out;


/**
 * Initializes the UART. stdio.h won't work until this function is called.
 */
void init_serial()
{
	// Set up port pins
	PORTC.DIRSET = PIN3_bm;
	PORTC.DIRCLR = PIN2_bm;

	// Enable transmit and receive on the UART
	UART.CTRLB = USART_RXEN_bm
			   | USART_TXEN_bm;

	// Asynchronous mode, 8 bits, no parity
	UART.CTRLC = USART_CMODE_ASYNCHRONOUS_gc
			   | USART_PMODE_DISABLED_gc
			   | USART_CHSIZE_8BIT_gc;

	// BSEL=2094, BSCALE=-7
	// Baudrate is 115200 at 32 MHz system clock
	UART.BAUDCTRLA = 0x2e;
	UART.BAUDCTRLB = 0x98;

	// Attach the uart_getchar and uart_putchar functions to the UART_IN and UART_OUT
	// file streams
	fdev_setup_stream(&uart_in, NULL, uart_getchar, _FDEV_SETUP_READ);
	fdev_setup_stream(&uart_out, uart_putchar, NULL, _FDEV_SETUP_WRITE);

	// Connect stdin, stdout, and stderr to the UART
	stdin = &uart_in;
	stdout = &uart_out;
	stderr = &uart_out;
}


/**
 * Write a character to the UART. This function is connected to the uart_out FILE stream,
 * which is set to stdout and stderr.
 *
 * @param c Character to transmit
 * @param f FILE pointer to operate on. This parameter is ignored, but is required by
 * 			stdio.
 *
 * @return Character that was transmitted, casted to an int.
 */
int uart_putchar(char c, FILE *f)
{
#ifdef USE_DOS_NEWLINES
	if(c == '\n')
		uart_putchar('\r', stdout);
#endif

	while((UART.STATUS & USART_DREIF_bm) == 0);		// Block until ready to transmit
	UART.DATA = c;

	return c;
}


/**
 * Read a character from the UART. This function is connected to the uart_in FILE stream,
 * which is set to stdin.
 *
 * @param f FILE pointer to operate on. This parameter is ignored, but is required by
 * 			stdio.
 *
 * @return Received character, casted to an int.
 */
int uart_getchar(FILE *f)
{
#ifdef UART_ECHO_ON
	char c;
#endif
	while((UART.STATUS & USART_RXCIF_bm) == 0);		// Block until character is received
#ifdef UART_ECHO_ON
	c = UART.DATA;
	uart_putchar(c, NULL);
	return (int) c;
#else
	return (int) UART.DATA;
#endif
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
	if(token == NULL)
		return TOKEN_UNDEF;

	char *match = (char *) bsearch(&token,
								   tokens,
								   sizeof(tokens)/sizeof(*tokens),
								   sizeof(*tokens),
								   comp_token);

	if(match == NULL)
		return TOKEN_UNDEF;
	else
		return ((int)match - (int)tokens) / sizeof(*tokens);
}


/**
 * Returns a pointer to the motor referenced by token, or NULL if the token
 * is not a motor.
 */
motor_t *get_motor(token_t token)
{
	switch(token)
	{
	case TOKEN_A:
		return &motor_a;
		break;
	case TOKEN_B:
		return &motor_b;
		break;
	case TOKEN_C:
		return &motor_c;
		break;
	case TOKEN_D:
		return &motor_d;
		break;
	default:
		break;
	}

	return NULL;
}


/**
 * Read a command from the serial terminal and parse it.
 */
void parse_command(void)
{
	char input[16];
	char *tok;
	int i;
	motor_t *motor;

	for(i=0; i<15; i++)
	{
		input[i] = getchar();

		if(input[i] == '\n' || input[i] == '\r')
			break;
	}

	input[i] = '\0';
	printf("\n");
	tolower_str(input);

	switch(find_token(strtok(input, delimiters)))
	{
	case TOKEN_HEADING:
		puts("Not implemented");
		break;
	case TOKEN_HELP:
		puts(help);
		break;
	case TOKEN_PWM:
		motor = get_motor(find_token(strtok(NULL, delimiters)));
		tok = strtok(NULL, delimiters);

		if(motor != NULL && tok != NULL)
			run_pwm(motor, atoi(tok));
		else if(motor == NULL)
			puts(bad_motor);
		else
			puts(error);
		break;
	case TOKEN_SET:
		motor = get_motor(find_token(strtok(NULL, delimiters)));
		tok = strtok(NULL, delimiters);

		if(motor != NULL && tok != NULL)
			run_pid(motor, atoi(tok));
		else if(motor == NULL)
			puts(bad_motor);
		else
			puts(error);
		break;
	case TOKEN_STATUS:
		motor = get_motor(find_token(strtok(NULL, delimiters)));
		if(motor != NULL)
			print_status(motor);
		else
			puts(bad_motor);
		break;
	default:
		puts(error);
		break;
	}
}


/**
 * Print a prompt to stdout, then parse and execute a command from stdin.
 */
void get_command(void)
{
	printf(prompt);
	parse_command();
}


/**
 * Prints a welcome message to stdout.
 */
void print_banner(void)
{
	puts(banner);
}


/**
 * Simple function to test the serial port. Outputs "Hello, world!" in an infinite loop.
 */
void test_serial_out(void)
{
	for(;;)
	{
		puts("Hello, world!");
	}
}


/**
 * Print the first NUM_SAMPLES after last changing the setpoint of motor
 */
void print_status(motor_t *motor)
{
	int i;

	if(motor == NULL)
	{
		puts(bad_motor);
		return;
	}

	if(motor->sample_counter < NUM_SAMPLES)
	{
		printf("The sample buffer is not yet full. Only %d out of %d samples have been "
			   "collected so far.\n", motor->sample_counter+1, NUM_SAMPLES);
		return;
	}

	puts("t,pwm,speed");

	for(i=0; i<NUM_SAMPLES; i++)
	{
		printf("%d,%u,%u\n", i, motor->samples->pwm, motor->samples->enc);
	}
}


void run_pwm(motor_t *motor, int pwm)
{
	motor->controller.enabled = 0;

	if(pwm > 0)
	{
		change_pwm(motor, pwm);
		change_direction(motor, DIR_FORWARD);
	}
	else if(pwm < 0)
	{
		change_pwm(motor, pwm);
		change_direction(motor, DIR_REVERSE);
	}
	else
	{
		change_pwm(motor, pwm);
		change_direction(motor, DIR_BRAKE);
	}

	update_speed(motor);
}


void run_pid(motor_t *motor, int sp)
{
	if(sp > 0)
	{
		change_setpoint(motor, sp);
		change_direction(motor, DIR_FORWARD);
	}
	else if(sp < 0)
	{
		change_setpoint(motor, -sp);
		change_direction(motor, DIR_REVERSE);
	}
	else
	{
		change_setpoint(motor, 0);
		change_direction(motor, DIR_BRAKE);
	}

	motor->controller.enabled = 1;
}
