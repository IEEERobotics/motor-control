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
#include <avr/interrupt.h>
#include "buffer.h"
#include "serial.h"

#define NEXT_TOKEN		(find_token(strtok(NULL, delimiters)))
#define UART USARTC0
#define UART_DREINTLVL	USART_DREINTLVL_MED_gc	// Data Register Empty interrupt priority
#define USE_DOS_NEWLINES						// Use CRLF line endings
#define BACKSPACE	'\b'
#define BUFFER_SIZE		128						// UART read and write buffer size
//#define UART_ECHO_ON					// Echo received characters back to the terminal


/**
 * Serial read and write buffers
 */
buffer_t read_buffer, write_buffer;
volatile uint8_t read_buffer_data[BUFFER_SIZE], write_buffer_data[BUFFER_SIZE];


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
					   	 "servo",
					   	 "set",
					   	 "status"
};

const char *prompt = "> ";
const char *banner = "\fNCSU IEEE 2012 Hardware Team Sensors Controller\n"
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
	// Initialize buffers
	buffer_init(&read_buffer, read_buffer_data, BUFFER_SIZE);
	buffer_init(&write_buffer, write_buffer_data, BUFFER_SIZE);

	// Set up port pins
	PORTC.DIRSET = PIN3_bm;
	PORTC.DIRCLR = PIN2_bm;

	// Set interrupt priority levels
	UART.CTRLA = USART_RXCINTLVL_MED_gc
			   | USART_DREINTLVL_OFF_gc;

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

	// Enable medium-priority interrupts
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;
	sei();
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

	buffer_put(&write_buffer, c);

	// Enable Data Register Empty interrupt. This will be disabled once all of the data
	// in write_buffer has been sent.
	UART.CTRLA = (UART.CTRLA & ~USART_DREINTLVL_gm) | UART_DREINTLVL;

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
	uint8_t c;

	while(! buffer_get(&read_buffer, &c));	// Block while read_buffer is empty
#ifdef UART_ECHO_ON
	uart_putchar(c);
#endif
	return (int) c;
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
static inline token_t find_token(char *token)
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
 * Convert a string to lowercase.
 *
 * @param str String to convert
 */
static inline void tolower_str(char *str)
{
	while(*str)
	{
		*str = (char) tolower((int) *str);
		str++;
	}
}


/**
 * Returns a pointer to the motor referenced by token, or NULL if the token
 * is not a motor.
 */
static inline motor_t *get_motor(token_t token)
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
 * Returns the servo_t corresponding to a given token, or SERVO_INVALID if the token
 * can't be mapped to a servo.
 */
static inline servo_t get_servo(token_t token)
{
	switch(token)
	{
	case TOKEN_A:
		return SERVO_A;
		break;
	case TOKEN_B:
		return SERVO_B;
		break;
	case TOKEN_C:
		return SERVO_C;
		break;
	case TOKEN_D:
		return SERVO_D;
		break;
	default:
		break;
	}

	return SERVO_INVALID;
}


/**
 * Run a motor at a specified duty cycle.
 * Calling this function will disable the PID controller associated with the given motor.
 *
 * @param motor The motor to operate on
 * @param pwm The duty cycle, within the range 0 to PWM_PERIOD. A negative sign indicates
 * 			  that the motor should be ran in reverse, and 0 indicates a hard brake.
 */
static inline void run_pwm(motor_t *motor, int pwm)
{
	motor->controller.enabled = 0;

	if(pwm > 0)
	{
		change_pwm(motor, pwm);
		change_direction(motor, DIR_FORWARD);
	}
	else if(pwm < 0)
	{
		change_pwm(motor, -pwm);
		change_direction(motor, DIR_REVERSE);
	}
	else
	{
		change_pwm(motor, 0);
		change_direction(motor, DIR_BRAKE);
	}

	update_speed(motor);
}


/**
 * Change the PID setpoint of a motor.
 * Calling this function will enable the PID controller associated with the motor.
 *
 * @param motor The motor to operate on
 * @param sp The setpoint to change the motor to
 */
static inline void run_pid(motor_t *motor, int sp)
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


/**
 * Print the first NUM_SAMPLES after last changing the setpoint of motor
 */
static inline void print_status(motor_t *motor)
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

	puts("pwm speed");
	printf("[");

	for(i=0; i<NUM_SAMPLES; i++)
	{
		printf("%u %u\n", motor->samples[i].pwm, motor->samples[i].enc);
	}

	printf("]\n");
}


/**
 * Read a command from the serial terminal and parse it.
 */
static inline void parse_command(void)
{
	char input[16];
	char *tok;
	int i = 0;
	char c;
	motor_t *motor;
	servo_t servo;

	while((c = getchar()) != '\r')
	{
		if(i < 15 && isprint(c))
		{
			input[i++] = c;
			putchar(c);
		}
		else if(c == BACKSPACE && i > 0)
		{
			i--;
			putchar(BACKSPACE);
		}
	}

	printf("\n");
	if(i == 0)
		return;		// Empty string

	input[i] = '\0';
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
		motor = get_motor(NEXT_TOKEN);
		tok = strtok(NULL, delimiters);

		if(motor != NULL && tok != NULL)
			run_pwm(motor, atoi(tok));
		else if(motor == NULL)
			puts(bad_motor);
		else
			puts(error);
		break;
	case TOKEN_SERVO:
		servo = get_servo(NEXT_TOKEN);
		tok = strtok(NULL, delimiters);

		if(! set_angle(servo, atoi(tok)))
		{
			puts(error);
		}
		break;
	case TOKEN_SET:
		motor = get_motor(NEXT_TOKEN);
		tok = strtok(NULL, delimiters);

		if(motor != NULL && tok != NULL)
			run_pid(motor, atoi(tok));
		else if(motor == NULL)
			puts(bad_motor);
		else
			puts(error);
		break;
	case TOKEN_STATUS:
		motor = get_motor(NEXT_TOKEN);
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
 * Data Register Empty ISR
 *
 * This ISR is called whenever the UART is ready to transmit the next byte, and it has
 * been enabled in uart_putchar(). This interrupt is disabled as soon as write_buffer
 * becomes empty (or else it would be called continuously).
 */
ISR(USARTC0_DRE_vect)
{
	uint8_t c;

	if(buffer_get(&write_buffer, &c))
		UART.DATA = c;
	else
		UART.CTRLA = (UART.CTRLA & ~USART_DREINTLVL_gm) | USART_DREINTLVL_OFF_gc;
}


/**
 * Receive Complete ISR
 *
 * This ISR is called whenever the UART has received a byte.
 */
ISR(USARTC0_RXC_vect)
{
	buffer_put(&read_buffer, UART.DATA);
}
