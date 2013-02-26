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
#include <stdbool.h>
#include "motor.h"
#include "pid.h"
#include "servo_parallax.h"
#include "ultrasonic.h"
#include "compass.h"
#include "timer.h"
#include "accelerometer.h"
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
					   	 "heading_pid",
					   	 "help",
					   	 "motor_pid",
					   	 "pwm",
					   	 "reset",
					   	 "s",
					   	 "sensors",
					   	 "servo",
					   	 "set",
					   	 "sizeofs",
					   	 "status",
					   	 "stop",
					   	 "straight",
					   	 "turn_in_place"
};

const char *prompt = "> ";
const char *banner = "\x1b[2J\x1b[HNCSU IEEE 2012 Hardware Team Motor Controller\r\n"
					 "Type \"help\" for a list of available commands.\r\n";
const char *help = "heading [angle] [speed]\r\n"
				   "heading_pid [Kp] [Ki] [Kd]\r\n"
				   "help\r\n"
				   "motor_pid [Kp] [Ki] [Kd]\r\n"
				   "pwm [a|b|c|d] [0-10000]\r\n"
				   "reset"
				   "sensors\r\n"
				   "servo [channel] [ramp] [angle]\r\n"
				   "set [heading] [speed]\r\n"
				   "sizeofs\r\n"
				   "status\r\n"
				   "stop\r\n"
				   "straight [pwm]\r\n"
				   "turn_in_place [pwm]\r\n";
const char *error = "Bad command.\r\n";
const char *bad_motor = "Bad motor.\r\n";
const char *not_implemented = "Not implemented.\r\n";


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
	char *heading = NEXT_STRING();
	char *speed = NEXT_STRING();

	if(heading != NULL && speed != NULL)
	{
		change_setpoint(atoi(heading), atoi(speed));
	}
	else
	{
		puts(error);
	}
}


static inline void exec_heading_pid(void)
{
	char *p = NEXT_STRING();
	char *i = NEXT_STRING();
	char *d = NEXT_STRING();

	if(p != NULL && i != NULL && d != NULL)
	{
		change_heading_constants(atoi(p), atoi(i), atoi(d));
	}
	else
	{
		puts(error);
	}
}


static inline void exec_help(void)
{
	puts(help);
}


static inline void exec_motor_pid(void)
{
	char *p = NEXT_STRING();
	char *i = NEXT_STRING();
	char *d = NEXT_STRING();

	if(p != NULL && i != NULL && d != NULL)
	{
		change_motor_constants(atoi(p), atoi(i), atoi(d));
	}
	else
	{
		puts(error);
	}
}


static inline void exec_pwm(void)
{
	motor_t *motor = get_motor(NEXT_TOKEN());
	char *tok = strtok(NULL, delimiters);
	int pwm;

	if(motor != NULL && tok != NULL)
	{
		pwm = atoi(tok);

		pid_enabled = false;

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


static inline void exec_reset(void)
{
	RST.CTRL = RST_SWRST_bm;
}


static inline void exec_sensors(void)
{
	uint16_t heading;
	int us_left;
	int us_front;
	int us_bottom;
	int us_right;
	int us_back;
	accelerometer_data_t a;
	const char *fmt_string = "heading: %d\r\n"
							 "us_left: %d\r\n"
							 "us_right: %d\r\n"
							 "us_front: %d\r\n"
							 "us_back: %d\r\n"
							 "us_bottom: %d\r\n"
							 "accel_x: %d\r\n"
							 "accel_y: %d\r\n"
							 "accel_z: %d\r\n";

	for(;;)
	{
		while(! compass_read(&heading));
		while(! accelerometer_get_data(&a));

		us_left = get_ultrasonic_distance(ULTRASONIC_LEFT);
		us_front = get_ultrasonic_distance(ULTRASONIC_FRONT);
		us_bottom = get_ultrasonic_distance(ULTRASONIC_BOTTOM);
		us_right = get_ultrasonic_distance(ULTRASONIC_RIGHT);
		us_back = get_ultrasonic_distance(ULTRASONIC_BACK);

		printf(fmt_string, heading, us_left, us_right, us_front, us_back, us_bottom, a.x, a.y, a.z);

		for(ms_timer=0; ms_timer<10;);
	}
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
	char *heading_str = NEXT_STRING();
	char *speed_str = NEXT_STRING();

	if(heading_str != NULL && speed_str != NULL)
	{
		change_setpoint(atoi(heading_str), atoi(speed_str));
	}
	else
	{
		puts(error);
	}
}


static inline void exec_sizeofs(void)
{
	const char *fmt_string = "char: %d\n"
							 "short int: %d\n"
							 "int: %d\n"
							 "long int: %d\n";

	printf(fmt_string, sizeof(char), sizeof(short int), sizeof(int), sizeof(long int));
}


static inline void exec_status(void)
{
	int i;

	puts("a_error a_out b_error b_out c_error c_out d_error d_out heading_error heading_out\r\n[");

	for(i=0; i<PID_NUM_SAMPLES; i++)
	{
		printf("%d %d %d %d %d %d %d %d %d %d\r\n",
				motor_a.controller.samples[i].error,
				motor_a.controller.samples[i].output,
				motor_b.controller.samples[i].error,
				motor_b.controller.samples[i].output,
				motor_c.controller.samples[i].error,
				motor_c.controller.samples[i].output,
				motor_d.controller.samples[i].error,
				motor_d.controller.samples[i].output,
				heading_pid.samples[i].error,
				heading_pid.samples[i].output);
	}

	puts("]\n");
}


static inline void exec_stop(void)
{
	pid_enabled = false;

	change_direction(&motor_a, DIR_BRAKE);
	change_direction(&motor_b, DIR_BRAKE);
	change_direction(&motor_c, DIR_BRAKE);
	change_direction(&motor_d, DIR_BRAKE);
	update_speed(&motor_a);
	update_speed(&motor_b);
	update_speed(&motor_c);
	update_speed(&motor_d);
}


static inline void exec_straight(void)
{
	char *pwm_str = NEXT_STRING();
	int pwm;
	direction_t dir;

	if(pwm_str != NULL)
	{
		pwm = atoi(pwm_str);

		if(pwm < 0)
		{
			dir = DIR_REVERSE;
			pwm = -pwm;
		}
		else
		{
			dir = DIR_FORWARD;
		}

		pid_enabled = false;
		change_pwm(&motor_a, pwm);
		change_pwm(&motor_b, pwm);
		change_pwm(&motor_c, pwm);
		change_pwm(&motor_d, pwm);
		change_direction(&motor_a, dir);
		change_direction(&motor_b, dir);
		change_direction(&motor_c, dir);
		change_direction(&motor_d, dir);
		update_speed(&motor_a);
		update_speed(&motor_b);
		update_speed(&motor_c);
		update_speed(&motor_d);
	}
	else
	{
		puts(error);
	}
}


static inline void exec_turn_in_place(void)
{
	char *pwm_str = NEXT_STRING();
	int pwm;
	direction_t left_dir, right_dir;

	if(pwm_str != NULL)
	{
		pwm = atoi(pwm_str);

		if(pwm < 0)
		{
			left_dir = DIR_REVERSE;
			right_dir = DIR_FORWARD;
			pwm = -pwm;
		}
		else
		{
			left_dir = DIR_FORWARD;
			right_dir = DIR_REVERSE;
		}

		pid_enabled = false;
		change_pwm(&motor_a, pwm);
		change_pwm(&motor_b, pwm);
		change_pwm(&motor_c, pwm);
		change_pwm(&motor_d, pwm);
		change_direction(&motor_a, left_dir);
		change_direction(&motor_b, left_dir);
		change_direction(&motor_c, right_dir);
		change_direction(&motor_d, right_dir);
		update_speed(&motor_a);
		update_speed(&motor_b);
		update_speed(&motor_c);
		update_speed(&motor_d);
	}
	else
	{
		puts(error);
	}
}


/**
 * Read a command from the serial terminal and parse it.
 */
static inline void parse_command(void)
{
	char input[32];
	int i = 0;
	char c;

	while((c = getchar()) != '\r')
	{
		if(i < 31 && isprint(c))
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

	printf("\r\n");
	if(i == 0)
		return;		// Empty string

	input[i] = '\0';
	tolower_str(input);

	switch(find_token(strtok(input, delimiters)))
	{
	case TOKEN_HEADING:
		exec_heading();
		break;
	case TOKEN_HEADING_PID:
		exec_heading_pid();
		break;
	case TOKEN_HELP:
		exec_help();
		break;
	case TOKEN_MOTOR_PID:
		exec_motor_pid();
		break;
	case TOKEN_PWM:
		exec_pwm();
		break;
	case TOKEN_RESET:
		exec_reset();
		break;
	case TOKEN_S:
	case TOKEN_SENSORS:
		exec_sensors();
		break;
	case TOKEN_SERVO:
		exec_servo();
		break;
	case TOKEN_SET:
		exec_set();
		break;
	case TOKEN_SIZEOFS:
		exec_sizeofs();
		break;
	case TOKEN_STATUS:
		exec_status();
		break;
	case TOKEN_STOP:
		exec_stop();
		break;
	case TOKEN_STRAIGHT:
		exec_straight();
		break;
	case TOKEN_TURN_IN_PLACE:
		exec_turn_in_place();
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
