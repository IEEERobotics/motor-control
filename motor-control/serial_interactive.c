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

#define NEXT_TOKEN()	(find_token(strtok(NULL, delimiters)))
#define NEXT_STRING()	(strtok(NULL, delimiters))
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
const char *not_implemented = "Not implemented.";


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


static inline void exec_heading(void)
{
	puts(not_implemented);
}


static inline void exec_help(void)
{
	puts(help);
}


static inline void exec_pwm(void)
{
	motor_t *motor = get_motor(NEXT_TOKEN());
	char *tok = strtok(NULL, delimiters);
	int pwm;

	if(motor != NULL && tok != NULL)
	{
		pwm = atoi(tok);

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
	else if(motor == NULL)
		puts(bad_motor);
	else
		puts(error);
}


static inline void exec_servo(void)
{
	char *channel = NEXT_STRING();
	char *ramp = NEXT_STRING();
	char *angle = NEXT_STRING();

	if(channel != NULL && ramp != NULL && angle != NULL)
	{
		parallax_set_angle(atoi(channel), atoi(angle), atoi(ramp));
	}
	else
	{
		puts(error);
	}
}


static inline void exec_set(void)
{
	motor_t *motor = get_motor(NEXT_TOKEN());
	char *tok = strtok(NULL, delimiters);
	int sp;

	if(motor != NULL && tok != NULL)
	{
		sp = atoi(tok);

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
	else if(motor == NULL)
		puts(bad_motor);
	else
		puts(error);
}


static inline void exec_status(void)
{
	int i;
	motor_t *motor = get_motor(NEXT_TOKEN());

	if(motor != NULL)
	{
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
	else
	{
		puts(bad_motor);
	}
}


/**
 * Read a command from the serial terminal and parse it.
 */
static inline void parse_command(void)
{
	char input[16];
	int i = 0;
	char c;

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
		exec_heading();
		break;
	case TOKEN_HELP:
		exec_help();
		break;
	case TOKEN_PWM:
		exec_pwm();
		break;
	case TOKEN_SERVO:
		exec_servo();
		break;
	case TOKEN_SET:
		exec_set();
		break;
	case TOKEN_STATUS:
		exec_status();
		break;
	default:
		puts(error);
		break;
	}
}


/**
 * Print a prompt to stdout, then parse and execute a command from stdin.
 */
void get_command_interactive(void)
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
