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
#include "servo_parallax.h"
#include "ultrasonic.h"
#include "compass.h"
#include "timer.h"
#include "accelerometer.h"
#include "pid.h"
#include "uart.h"
#include "serial_interactive.h"

#define NEXT_TOKEN()	(find_token(strtok(NULL, delimiters)))
#define NEXT_STRING()	(strtok(NULL, delimiters))
//#define BACKSPACE		'\b'
#define BACKSPACE		0x7f

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
					   	 "compass_start_calibration",
					   	 "compass_stop_calibration",
					   	 "d",
					   	 "heading",
					   	 "heading_pid",
					   	 "help",
					   	 "interactive",
					   	 "motor_pid",
					   	 "motor_step_response",
					   	 "move",
					   	 "pwm",
					   	 "pwm_drive",
					   	 "reset",
					   	 "s",
					   	 "sensor",
					   	 "sensors",
					   	 "sensors_continuous",
					   	 "servo",
					   	 "set",
					   	 "sizeofs",
					   	 "status",
					   	 "stop",
					   	 "straight",
					   	 "turn_abs",
					   	 "turn_in_place",
					   	 "turn_rel"
};

const char *prompt = "> ";
const char *banner = "\x1b[2J\x1b[HNCSU IEEE 2012 Hardware Team Motor Controller\r\n"
					 "Type \"help\" for a list of available commands.\r\n";
const char *help = "heading\r\n"
				   "heading_pid [Kp] [Ki] [Kd]\r\n"
				   "help\r\n"
				   "motor_pid [Kp] [Ki] [Kd]\r\n"
				   "pwm [a|b|c|d] [0-10000]\r\n"
				   "pwm_drive [left] [right]\r\n"
				   "reset\r\n"
				   "sensors\r\n"
				   "servo [channel] [ramp] [angle]\r\n"
				   "set [heading] [speed] [distance]\r\n"
				   "sizeofs\r\n"
				   "status\r\n"
				   "stop\r\n"
				   "straight [pwm]\r\n"
				   "turn_in_place [pwm]\r\n";
//const char *error = "ERROR\r\n";
//const char *ok = "OK\r\n";
//const char *bad_motor = "Bad motor.\r\n";
//const char *not_implemented = "Not implemented.\r\n";
const char *lf = "\n";
const char *crlf = "\r\n";
const char *argument_error = "too few arguments";
const char *empty_string = "";
const char *json_true = "true";
const char *json_false = "false";
const char *json_null = "null";

bool interactive_mode = false;


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


static inline void json_start_response(bool result, const char *msg)
{
	const char *result_str = result ? json_true : json_false;
	printf("{\"result\":%s,\"msg\":\"%s\"", result_str, msg);
}


static inline void json_add_int(const char *key, int val)
{
	printf(",\"%s\":%d", key, val);
}


static inline void json_add_object(const char *key, json_kv_t *kv_pairs, uint8_t len)
{
	uint8_t i;


	printf(",\"%s\":{", key);
	for(i=0; i<len; i++)
	{
		if(i > 0)
			printf(",");
		printf("\"%s\":%d", kv_pairs[i].key, kv_pairs[i].value);
	}
	printf("}");
}


static inline void json_end_response(void)
{
	const char *newline = interactive_mode ? crlf : lf;

	printf("}%s", newline);
}


static inline void json_respond_ok(const char *msg)
{
	json_start_response(true, msg);
	json_end_response();
}


static inline void json_respond_error(const char *msg)
{
	json_start_response(false, msg);
	json_end_response();
}


static inline void exec_compass_start_calibration()
{
	while(! compass_enter_calibration_mode());
	json_respond_ok(empty_string);
}


static inline void exec_compass_stop_calibration()
{
	while(! compass_exit_calibration_mode());
	json_respond_ok(empty_string);
}


static inline void exec_heading(void)
{
	uint16_t heading;

	while(! compass_read(&heading));

	json_start_response(true, "deprecated, use 'sensors' instead");
	json_add_int("data", heading);
	json_end_response();
}


static inline void exec_heading_pid(void)
{
	char *p = NEXT_STRING();
	char *i = NEXT_STRING();
	char *d = NEXT_STRING();

	if(p != NULL && i != NULL && d != NULL)
	{
		change_heading_constants(atoi(p), atoi(i), atoi(d));
		json_respond_ok(empty_string);
	}
	else
	{
		json_respond_error(argument_error);
	}
}


static inline void exec_help(void)
{
	puts(help);
}


static inline void exec_interactive(void)
{
	interactive_mode = !interactive_mode;

	if(interactive_mode)
	{
		json_respond_ok("interactive on");
	}
	else
	{
		json_respond_ok("interactive off");
	}
}


static inline void exec_motor_pid(void)
{
	char *p = NEXT_STRING();
	char *i = NEXT_STRING();
	char *d = NEXT_STRING();

	if(p != NULL && i != NULL && d != NULL)
	{
		change_motor_constants(atoi(p), atoi(i), atoi(d));
		json_respond_ok(empty_string);
	}
	else
	{
		json_respond_error(argument_error);
	}
}


static inline void exec_motor_step_response(void)
{
	motor_t *motor = get_motor(NEXT_TOKEN());
	unsigned long int count = 0, prev_count = 0;
	unsigned long speed;
	int i;

	if(motor != NULL)
	{
		pid_disable();
		clear_encoder_count();
		*(motor->reg.enc) = 0;
		change_pwm(motor, 10000);
		change_direction(motor, DIR_FORWARD);

		for(ms_timer=0; ms_timer<1;);	// wait for the next tick before starting the motor
		update_speed(motor);
		for(i=0; i<128; i++)
		{
			prev_count = count;
			count = motor->encoder_count;
//			speed = (count - prev_count)*(60000u/MS_TIMER_PER);
			speed = (ENC_SAMPLE_HZ / (unsigned short int)*(motor->reg.enc));
			if(speed > 10000) speed = 0;
			printf("%lu\r\n", speed);
			for(ms_timer=0; ms_timer<1;);
		}

		change_pwm(motor, 0);
		change_direction(motor, DIR_BRAKE);
		update_speed(motor);
	}
	else
	{
		json_respond_error(argument_error);
	}
}


static inline void exec_move(void)
{
	char *speed_str = NEXT_STRING();
	char *distance_str = NEXT_STRING();

	if(speed_str != NULL && distance_str != NULL)
	{
		change_setpoint(0, atoi(speed_str), atoi(distance_str), true);
		json_respond_ok("");
	}
	else
	{
		json_respond_error(argument_error);
	}
}


static inline void exec_pwm(void)
{
	motor_t *motor = get_motor(NEXT_TOKEN());
	char *tok = strtok(NULL, delimiters);
	int pwm;
	int i;

	if(motor != NULL && tok != NULL)
	{
		pwm = atoi(tok);

		pid_disable();

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
		clear_encoder_count();
		json_respond_ok(empty_string);

//		for(i=0; i<256; i++)
//		{
//			for(ms_timer=0; ms_timer<1;);
//			printf("%lu\r\n", motor->encoder_count);
//		}
	}
	else
	{
		json_respond_error(argument_error);
	}
}


static inline void exec_pwm_drive(void)
{
	char *left_str = NEXT_STRING();
	char *right_str = NEXT_STRING();
	int left, right;

	if(left_str != NULL && right_str != NULL)
	{
		left = atoi(left_str);
		right = atoi(right_str);
		pid_disable();

		if(left > 0)
		{
#if NUM_MOTORS == 4
			change_direction(&MOTOR_LEFT_FRONT, DIR_FORWARD);
			change_direction(&MOTOR_LEFT_BACK, DIR_FORWARD);
#elif NUM_MOTORS == 2
			change_direction(&MOTOR_LEFT, DIR_FORWARD);
#endif
		}
		else
		{
#if NUM_MOTORS == 4
			change_direction(&MOTOR_LEFT_FRONT, DIR_REVERSE);
			change_direction(&MOTOR_LEFT_BACK, DIR_REVERSE);
#elif NUM_MOTORS == 2
			change_direction(&MOTOR_LEFT, DIR_REVERSE);
#endif
			left = -left;
		}

		if(right > 0)
		{
#if NUM_MOTORS == 4
			change_direction(&MOTOR_RIGHT_FRONT, DIR_FORWARD);
			change_direction(&MOTOR_RIGHT_BACK, DIR_FORWARD);
#elif NUM_MOTORS == 2
			change_direction(&MOTOR_RIGHT, DIR_FORWARD);
#endif
		}
		else
		{
#if NUM_MOTORS == 4
			change_direction(&MOTOR_RIGHT_FRONT, DIR_REVERSE);
			change_direction(&MOTOR_RIGHT_BACK, DIR_REVERSE);
#elif NUM_MOTORS == 2
			change_direction(&MOTOR_RIGHT, DIR_REVERSE);
#endif
			right = -right;
		}

#if NUM_MOTORS == 4
		change_pwm(&MOTOR_LEFT_FRONT, left);
		change_pwm(&MOTOR_LEFT_BACK, left);
		change_pwm(&MOTOR_RIGHT_FRONT, right);
		change_pwm(&MOTOR_RIGHT_BACK, right);
		update_speed(&MOTOR_LEFT_FRONT);
		update_speed(&MOTOR_LEFT_BACK);
		update_speed(&MOTOR_RIGHT_FRONT);
		update_speed(&MOTOR_RIGHT_BACK);
#elif NUM_MOTORS == 2
		change_pwm(&MOTOR_LEFT, left);
		change_pwm(&MOTOR_RIGHT, right);
		update_speed(&MOTOR_LEFT);
		update_speed(&MOTOR_RIGHT);
#endif
		json_respond_ok(empty_string);
	}
	else
	{
		json_respond_error(argument_error);
	}
}


static inline void exec_reset(void)
{
	CCP = CCP_IOREG_gc;
	RST.CTRL = RST_SWRST_bm;
}


static inline void exec_sensor(void)
{
	char *id_str = NEXT_STRING();
	int data;
	accelerometer_data_t a;

	if(id_str != NULL)
	{
		switch(atoi(id_str))
		{
		case SENSOR_COMPASS:
			while(! compass_read((uint16_t *) &data));
			break;
		case SENSOR_ACCEL_X:
			accelerometer_get_data(&a);
			data = a.x;
			break;
		case SENSOR_ACCEL_Y:
			accelerometer_get_data(&a);
			data = a.y;
			break;
		case SENSOR_ACCEL_Z:
			accelerometer_get_data(&a);
			data = a.z;
			break;
		case SENSOR_US_LEFT:
			data = get_ultrasonic_distance(ULTRASONIC_LEFT);
			break;
		case SENSOR_US_FRONT:
			data = get_ultrasonic_distance(ULTRASONIC_FRONT);
			break;
		case SENSOR_US_RIGHT:
			data = get_ultrasonic_distance(ULTRASONIC_RIGHT);
			break;
		case SENSOR_US_BACK:
			data = get_ultrasonic_distance(ULTRASONIC_BACK);
			break;
		default:
			json_respond_error("unrecognized sensor id");
			return;
			break;
		}

		json_start_response(true, "");
		json_add_int("data", data);
		json_end_response();
	}
	else
	{
		json_respond_error(argument_error);
	}
}


static inline void exec_sensors(void)
{
	uint16_t heading;
	accelerometer_data_t a;
	json_kv_t us_array[4];
	json_kv_t accel_array[3];

	accel_array[0].key = "x";
	accel_array[0].value = a.x;
	accel_array[1].key = "y";
	accel_array[1].value = a.y;
	accel_array[2].key = "z";
	accel_array[2].value = a.z;

	us_array[0].key = "left";
	us_array[0].value = get_ultrasonic_distance(ULTRASONIC_LEFT);
	us_array[1].key = "right";
	us_array[1].value = get_ultrasonic_distance(ULTRASONIC_RIGHT);
	us_array[2].key = "front";
	us_array[2].value = get_ultrasonic_distance(ULTRASONIC_FRONT);
	us_array[3].key = "back";
	us_array[3].value = get_ultrasonic_distance(ULTRASONIC_BACK);

	while(! compass_read(&heading));
	while(! accelerometer_get_data(&a));

	json_start_response(true, empty_string);
	json_add_int("heading", heading);
	json_add_object("accel", accel_array, sizeof(accel_array)/sizeof(json_kv_t));
	json_add_object("ultrasonic", us_array, sizeof(us_array)/sizeof(json_kv_t));
	json_end_response();
}


static inline void exec_sensors_continuous(void)
{
	for(;;)
	{
		exec_sensors();
		if(! buffer_empty(&debug_uart.read_buffer))		// Quit if any key is pressed
			return;

		for(ms_timer=0; ms_timer<100;);
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
		json_respond_ok(empty_string);
	}
	else
	{
		json_respond_error(argument_error);
	}
}


static inline void exec_set(void)
{
	char *heading_str = NEXT_STRING();
	char *speed_str = NEXT_STRING();
	char *distance_str = NEXT_STRING();

	if(heading_str != NULL && speed_str != NULL && distance_str != NULL)
	{
		change_setpoint(atoi(heading_str), atoi(speed_str), atoi(distance_str), true);
		json_respond_ok("TODO: block here until movement is complete");
	}
	else
	{
		json_respond_error(argument_error);
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

	puts("a_error a_out b_error b_out c_error c_out d_error d_out heading_error heading_out [\r\n");
	for(ms_timer=0; ms_timer<10;);

	for(i=0; i<PID_NUM_SAMPLES; i++)
	{
//		printf("%d %d %d %d %d %d %d %d %d %d\r\n",
//				motor_a.controller.samples[i].error,
//				motor_a.controller.samples[i].output,
//				motor_b.controller.samples[i].error,
//				motor_b.controller.samples[i].output,
//				motor_c.controller.samples[i].error,
//				motor_c.controller.samples[i].output,
//				motor_d.controller.samples[i].error,
//				motor_d.controller.samples[i].output,
//				heading_pid.samples[i].error,
//				heading_pid.samples[i].output);
		printf("%d %d %d %d %d %d\r\n",
				MOTOR_LEFT.controller.samples[i].error,
				MOTOR_LEFT.controller.samples[i].output,
				MOTOR_RIGHT.controller.samples[i].error,
				MOTOR_RIGHT.controller.samples[i].output,
				heading_pid.samples[i].error,
				heading_pid.samples[i].output);
		for(ms_timer=0; ms_timer<10;);
	}

	puts("]\n");
}


static inline void exec_stop(void)
{
	pid_disable();

	change_direction(&motor_a, DIR_BRAKE);
	change_direction(&motor_b, DIR_BRAKE);
	change_direction(&motor_c, DIR_BRAKE);
	change_direction(&motor_d, DIR_BRAKE);
	update_speed(&motor_a);
	update_speed(&motor_b);
	update_speed(&motor_c);
	update_speed(&motor_d);

	json_respond_ok(empty_string);
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

		pid_disable();
#if NUM_MOTORS == 4
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
#elif NUM_MOTORS == 2
		change_pwm(&MOTOR_LEFT, pwm);
		change_pwm(&MOTOR_RIGHT, pwm);
		change_direction(&MOTOR_LEFT, dir);
		change_direction(&MOTOR_RIGHT, dir);
		update_speed(&MOTOR_LEFT);
		update_speed(&MOTOR_RIGHT);
#endif
		json_respond_ok(empty_string);
	}
	else
	{
		json_respond_error(argument_error);
	}
}


static inline void exec_turn_abs(void)
{
	json_respond_error("TODO: implement support for this in pid.c");
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

		pid_disable();
#if NUM_MOTORS == 4
		change_pwm(&MOTOR_LEFT_BACK, pwm);
		change_pwm(&MOTOR_LEFT_FRONT, pwm);
		change_pwm(&MOTOR_RIGHT_BACK, pwm);
		change_pwm(&MOTOR_RIGHT_FRONT, pwm);
		change_direction(&MOTOR_LEFT_BACK, left_dir);
		change_direction(&MOTOR_LEFT_FRONT, left_dir);
		change_direction(&MOTOR_RIGHT_BACK, right_dir);
		change_direction(&MOTOR_RIGHT_FRONT, right_dir);
		update_speed(&MOTOR_LEFT_BACK);
		update_speed(&MOTOR_LEFT_FRONT);
		update_speed(&MOTOR_RIGHT_BACK);
		update_speed(&MOTOR_RIGHT_FRONT);
#elif NUM_MOTORS == 2
		change_pwm(&MOTOR_LEFT, pwm);
		change_pwm(&MOTOR_RIGHT, pwm);
		change_direction(&MOTOR_LEFT, left_dir);
		change_direction(&MOTOR_RIGHT, right_dir);
		update_speed(&MOTOR_LEFT);
		update_speed(&MOTOR_RIGHT);
#endif
		json_respond_ok(empty_string);
	}
	else
	{
		json_respond_error(argument_error);
	}
}


static inline void exec_turn_rel(void)
{
	char *heading_str = NEXT_STRING();

	if(heading_str != NULL)
	{
		change_setpoint(atoi(heading_str), 0, 0, true);
		json_respond_ok("TODO: This probably won't work because distance=0");
	}
	else
	{
		json_respond_error(argument_error);
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
			if(interactive_mode)
				putchar(c);
		}
		else if(c == BACKSPACE && i > 0)
		{
			i--;
			if(interactive_mode)
				putchar(BACKSPACE);
		}
	}

	if(interactive_mode)
		printf("%s", crlf);
	if(i == 0)
		return;		// Empty string

	input[i] = '\0';
	tolower_str(input);

	switch(find_token(strtok(input, delimiters)))
	{
	case TOKEN_COMPASS_START_CALIBRATION:
		exec_compass_start_calibration();
		break;
	case TOKEN_COMPASS_STOP_CALIBRATION:
		exec_compass_stop_calibration();
		break;
	case TOKEN_HEADING:
		exec_heading();
		break;
	case TOKEN_HEADING_PID:
		exec_heading_pid();
		break;
	case TOKEN_HELP:
		exec_help();
		break;
	case TOKEN_INTERACTIVE:
		exec_interactive();
		break;
	case TOKEN_MOTOR_PID:
		exec_motor_pid();
		break;
	case TOKEN_MOTOR_STEP_RESPONSE:
		exec_motor_step_response();
		break;
	case TOKEN_MOVE:
		exec_move();
		break;
	case TOKEN_PWM:
		exec_pwm();
		break;
	case TOKEN_PWM_DRIVE:
		exec_pwm_drive();
		break;
	case TOKEN_RESET:
		exec_reset();
		break;
	case TOKEN_S:
		exec_sensors();
		break;
	case TOKEN_SENSOR:
		exec_sensor();
		break;
	case TOKEN_SENSORS:
		exec_sensors();
		break;
	case TOKEN_SENSORS_CONTINUOUS:
		exec_sensors_continuous();
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
	case TOKEN_TURN_ABS:
		exec_turn_abs();
		break;
	case TOKEN_TURN_IN_PLACE:
		exec_turn_in_place();
		break;
	case TOKEN_TURN_REL:
		exec_turn_rel();
		break;
	default:
		json_respond_error("unrecognized command");
		break;
	}
}


/**
 * Print a prompt to stdout, then parse and execute a command from stdin.
 */
void get_command_interactive(void)
{
	if(interactive_mode)
		printf(prompt);
	parse_command();
}


/**
 * Prints a welcome message to stdout.
 */
void print_banner(void)
{
	if(interactive_mode)
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
