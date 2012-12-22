/*
 * serial_interactive.c
 *
 *  Created on: Dec 22, 2012
 *      Author: eal
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "motor.h"
#include "pid.h"
#include "serial_interactive.h"

#define NEXT_TOKEN		(find_token(strtok(NULL, delimiters)))
#define BACKSPACE		'\b'

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
const char *banner = "\fNCSU IEEE 2012 Hardware Team Motor Controller\n"
					 "Type \"help\" for a list of available commands.";
const char *help = "Not implemented.";
const char *error = "Bad command.";
const char *bad_motor = "Bad motor.";


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
		puts("Not implemented.");
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
