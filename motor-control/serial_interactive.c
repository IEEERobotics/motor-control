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
#include "json.h"
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
					   	 "compass_calibrate",
					   	 "compass_dump_eeprom",
					   	 "compass_flat",
					   	 "compass_ramp",
					   	 "compass_read_eeprom",
					   	 "compass_read_ram",
					   	 "compass_reset",
					   	 "compass_start_calibration",
					   	 "compass_stop_calibration",
					   	 "compass_write_eeprom",
					   	 "compass_write_ram",
					   	 "d",
					   	 "heading",
					   	 "heading_accuracy",
					   	 "heading_pid",
					   	 "help",
					   	 "interactive",
					   	 "left_close",
					   	 "left_down",
					   	 "left_drop",
					   	 "left_grab",
					   	 "left_open",
					   	 "left_up",
					   	 "motor_pid",
					   	 "motor_step_response",
					   	 "move",
					   	 "pwm",
					   	 "pwm_drive",
					   	 "ramp",
					   	 "reset",
					   	 "reset_servos",
					   	 "right_close",
					   	 "right_down",
					   	 "right_drop",
					   	 "right_grab",
					   	 "right_open",
					   	 "right_up",
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

bool interactive_mode = false;
int id_short, id_long;


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


static inline void exec_compass_calibrate(void)
{
	bool enabled = pid_is_enabled();

	if(enabled) pid_disable();

//	while(! compass_write_ram(COMPASS_RAM_OPMODE, COMPASS_OPMODE_STANDBY));
	while(! compass_enter_calibration_mode());

	for(ms_timer = 0; ms_timer < (20000/MS_TIMER_PER););	// Delay 20 s

	while(! compass_exit_calibration_mode());
	while(! compass_save_opmode());
	init_compass();

	if(enabled) pid_enable();
}


static inline void exec_compass_dump_eeprom(void)
{
	int i;
	uint8_t eeprom[9];
	uint8_t opmode;
	uint8_t outmode;

	for(i=0; i<9; i++)
	{
		while(! compass_read_eeprom(i, &eeprom[i]));
	}

	compass_read_ram(COMPASS_RAM_OPMODE, &opmode);
	compass_read_ram(COMPASS_RAM_OUTMODE, &outmode);

	json_start_response(true, empty_string, id_short);
	json_add_int("COMPASS_EEPROM_I2C_ADDRESS", eeprom[COMPASS_EEPROM_I2C_ADDRESS]);
	json_add_int("COMPASS_EEPROM_XOFFMSB", eeprom[COMPASS_EEPROM_XOFFMSB]);
	json_add_int("COMPASS_EEPROM_XOFFLSB", eeprom[COMPASS_EEPROM_XOFFLSB]);
	json_add_int("COMPASS_EEPROM_YOFFMSB", eeprom[COMPASS_EEPROM_YOFFMSB]);
	json_add_int("COMPASS_EEPROM_YOFFLSB", eeprom[COMPASS_EEPROM_YOFFLSB]);
	json_add_int("COMPASS_EEPROM_TIME_DELAY", eeprom[COMPASS_EEPROM_TIME_DELAY]);
	json_add_int("COMPASS_EEPROM_NUM_MEASUREMENTS", eeprom[COMPASS_EEPROM_NUM_MEASUREMENTS]);
	json_add_int("COMPASS_EEPROM_SOFTWARE_VER", eeprom[COMPASS_EEPROM_SOFTWARE_VER]);
	json_add_int("COMPASS_EEPROM_OPMODE", eeprom[COMPASS_EEPROM_OPMODE]);
	json_add_int("COMPASS_RAM_OPMODE", opmode);
	json_add_int("COMPASS_RAM_OUTMODE", outmode);
	json_end_response();
}


static inline void exec_compass_flat(void)
{
	compass_set(COMPASS_FLAT);
	json_respond_ok(empty_string, id_short);
}


static inline void exec_compass_ramp(void)
{
	compass_set(COMPASS_RAMP);
	json_respond_ok(empty_string, id_short);
}


static inline void exec_compass_read_eeprom(void)
{
	char *address_str = NEXT_STRING();
	uint8_t address;
	uint8_t data;

	if(address_str != NULL)
	{
		address = atoi(address_str);
		while(! compass_read_eeprom(address, &data));
		json_start_response(true, empty_string, id_short);
		json_add_int("address", address);
		json_add_int("data", data);
		json_end_response();
	}
	else
	{
		json_respond_error(argument_error, id_short);
	}
}


static inline void exec_compass_read_ram(void)
{
	char *address_str = NEXT_STRING();
	uint8_t address;
	uint8_t data;

	if(address_str != NULL)
	{
		address = atoi(address_str);
		while(! compass_read_ram(address, &data));
		json_start_response(true, empty_string, id_short);
		json_add_int("address", address);
		json_add_int("data", data);
		json_end_response();
	}
	else
	{
		json_respond_error(argument_error, id_short);
	}
}


static inline void exec_compass_reset(void)
{
	init_compass();
	json_respond_ok(empty_string, id_short);
}


static inline void exec_compass_start_calibration(void)
{
	while(! compass_enter_calibration_mode());
	json_respond_ok(empty_string, id_short);
}


static inline void exec_compass_stop_calibration(void)
{
	while(! compass_exit_calibration_mode());
	json_respond_ok(empty_string, id_short);
}


static inline void exec_compass_write_eeprom(void)
{
	char *address_str = NEXT_STRING();
	char *data_str = NEXT_STRING();
	uint8_t address;
	uint8_t data;

	if(address_str != NULL && data_str != NULL)
	{
		address = atoi(address_str);
		data = atoi(data_str);
		while(! compass_write_eeprom(address, data));
		json_start_response(true, empty_string, id_short);
		json_add_int("address", address);
		json_add_int("data", data);
		json_end_response();
	}
	else
	{
		json_respond_error(argument_error, id_short);
	}
}


static inline void exec_compass_write_ram(void)
{
	char *address_str = NEXT_STRING();
	char *data_str = NEXT_STRING();
	uint8_t address;
	uint8_t data;

	if(address_str != NULL && data_str != NULL)
	{
		address = atoi(address_str);
		data = atoi(data_str);
		while(! compass_write_ram(address, data));
		json_start_response(true, empty_string, id_short);
		json_add_int("address", address);
		json_add_int("data", data);
		json_end_response();
	}
	else
	{
		json_respond_error(argument_error, id_short);
	}
}


static inline void exec_heading(void)
{
	uint16_t heading = compass_get_bearing();

	json_start_response(true, "deprecated, use 'sensors' instead", id_short);
	json_add_int("data", heading);
	json_end_response();
}


static inline void exec_heading_accuracy(void)
{
	char *deadband = NEXT_STRING();

	if(deadband != NULL)
	{
		set_heading_deadband(atoi(deadband));
		json_respond_ok(empty_string, id_short);
	}
	else
	{
		json_respond_error(argument_error, id_short);
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
		json_respond_ok(empty_string, id_short);
	}
	else
	{
		json_respond_error(argument_error, id_short);
	}
}


static inline void exec_help(void)
{
	puts(help);
}


static inline void exec_left_close(void)
{
	parallax_set_angle(SERVO_LEFT_GRIP_CHANNEL, SERVO_LEFT_GRIP_CLOSE, SERVO_GRIP_RAMP);
	json_respond_ok(empty_string, id_short);
}


static inline void exec_left_down(void)
{
	parallax_set_angle(SERVO_LEFT_ARM_CHANNEL, SERVO_LEFT_ARM_DOWN, SERVO_ARM_RAMP);
	json_respond_ok(empty_string, id_short);
}


static inline void exec_left_drop(void)
{
	parallax_set_angle(SERVO_LEFT_ARM_CHANNEL, SERVO_LEFT_ARM_DOWN, SERVO_ARM_RAMP);
	parallax_set_angle(SERVO_LEFT_GRIP_CHANNEL, SERVO_LEFT_GRIP_OPEN, SERVO_GRIP_RAMP);
	json_respond_ok(empty_string, id_short);
}


static inline void exec_left_grab(void)
{
	parallax_set_angle(SERVO_LEFT_GRIP_CHANNEL, SERVO_LEFT_GRIP_CLOSE, SERVO_GRIP_RAMP);
	for(ms_timer=0; ms_timer<SERVO_CLOSE_TIME;);	// wait for arm to close
	parallax_set_angle(SERVO_LEFT_ARM_CHANNEL, SERVO_LEFT_ARM_UP, SERVO_ARM_RAMP);
	json_respond_ok(empty_string, id_short);
}


static inline void exec_left_open(void)
{
	parallax_set_angle(SERVO_LEFT_GRIP_CHANNEL, SERVO_LEFT_GRIP_OPEN, SERVO_GRIP_RAMP);
	json_respond_ok(empty_string, id_short);
}


static inline void exec_left_up(void)
{
	parallax_set_angle(SERVO_LEFT_ARM_CHANNEL, SERVO_LEFT_ARM_UP, SERVO_ARM_RAMP);
	json_respond_ok(empty_string, id_short);
}


static inline void exec_interactive(void)
{
	interactive_mode = !interactive_mode;

	if(interactive_mode)
	{
		json_respond_ok("interactive on", id_short);
	}
	else
	{
		json_respond_ok("interactive off", id_short);
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
		json_respond_ok(empty_string, id_short);
	}
	else
	{
		json_respond_error(argument_error, id_short);
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
		json_respond_error(argument_error, id_short);
	}
}


static inline void exec_move(void)
{
	char *speed_str = NEXT_STRING();
	char *distance_str = NEXT_STRING();

	if(speed_str != NULL && distance_str != NULL)
	{
		change_setpoint(0, atoi(speed_str), atoi(distance_str), true, true);
//		json_respond_ok("");
	}
	else
	{
//		json_respond_error(argument_error);
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
		json_respond_ok(empty_string, id_short);

//		for(i=0; i<256; i++)
//		{
//			for(ms_timer=0; ms_timer<1;);
//			printf("%lu\r\n", motor->encoder_count);
//		}
	}
	else
	{
		json_respond_error(argument_error, id_short);
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
		json_respond_ok(empty_string, id_short);
	}
	else
	{
		json_respond_error(argument_error, id_short);
	}
}


static inline void exec_ramp(void)
{
	char *ramp_str = NEXT_STRING();

	if(ramp_str != NULL)
	{
		set_ramp(atoi(ramp_str));
		json_respond_ok(empty_string, id_short);
	}
	else
	{
		json_respond_error(argument_error, id_short);
	}
}


static inline void exec_reset(void)
{
	CCP = CCP_IOREG_gc;
	RST.CTRL = RST_SWRST_bm;
}


static inline void exec_reset_servos(void)
{
	init_servo_parallax();
	json_respond_ok(empty_string, id_short);
}


static inline void exec_right_close(void)
{
	parallax_set_angle(SERVO_RIGHT_GRIP_CHANNEL, SERVO_RIGHT_GRIP_CLOSE, SERVO_GRIP_RAMP);
	json_respond_ok(empty_string, id_short);
}


static inline void exec_right_down(void)
{
	parallax_set_angle(SERVO_RIGHT_ARM_CHANNEL, SERVO_RIGHT_ARM_DOWN, SERVO_ARM_RAMP);
	json_respond_ok(empty_string, id_short);
}


static inline void exec_right_drop(void)
{
	parallax_set_angle(SERVO_RIGHT_ARM_CHANNEL, SERVO_RIGHT_ARM_DOWN, SERVO_ARM_RAMP);
	parallax_set_angle(SERVO_RIGHT_GRIP_CHANNEL, SERVO_RIGHT_GRIP_OPEN, SERVO_GRIP_RAMP);
	json_respond_ok(empty_string, id_short);
}


static inline void exec_right_grab(void)
{
	parallax_set_angle(SERVO_RIGHT_GRIP_CHANNEL, SERVO_RIGHT_GRIP_CLOSE, SERVO_GRIP_RAMP);
	for(ms_timer=0; ms_timer<SERVO_CLOSE_TIME;);	// wait for arm to close
	parallax_set_angle(SERVO_RIGHT_ARM_CHANNEL, SERVO_RIGHT_ARM_UP, SERVO_ARM_RAMP);
	json_respond_ok(empty_string, id_short);
}


static inline void exec_right_open(void)
{
	parallax_set_angle(SERVO_RIGHT_GRIP_CHANNEL, SERVO_RIGHT_GRIP_OPEN, SERVO_GRIP_RAMP);
	json_respond_ok(empty_string, id_short);
}


static inline void exec_right_up(void)
{
	parallax_set_angle(SERVO_RIGHT_ARM_CHANNEL, SERVO_RIGHT_ARM_UP, SERVO_ARM_RAMP);
	json_respond_ok(empty_string, id_short);
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
			data = compass_get_bearing();
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
			json_respond_error("unrecognized sensor id", id_short);
			return;
			break;
		}

		json_start_response(true, "", id_short);
		json_add_int("data", data);
		json_end_response();
	}
	else
	{
		json_respond_error(argument_error, id_short);
	}
}


static inline void exec_sensors(void)
{
	int heading;
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

	heading = compass_get_bearing();
	while(! accelerometer_get_data(&a));

	json_start_response(true, empty_string, id_short);
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
		json_respond_ok(empty_string, id_short);
	}
	else
	{
		json_respond_error(argument_error, id_short);
	}
}


static inline void exec_set(void)
{
	char *heading_str = NEXT_STRING();
	char *speed_str = NEXT_STRING();
	char *distance_str = NEXT_STRING();

	if(heading_str != NULL && speed_str != NULL && distance_str != NULL)
	{
		change_setpoint(atoi(heading_str), atoi(speed_str), atoi(distance_str), true, true);
//		json_respond_ok("TODO: block here until movement is complete");
	}
	else
	{
//		json_respond_error(argument_error);
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

	json_respond_ok(empty_string, id_short);
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
		json_respond_ok(empty_string, id_short);
	}
	else
	{
		json_respond_error(argument_error, id_short);
	}
}


static inline void exec_turn_abs(void)
{
	char *heading_str = NEXT_STRING();

	if(heading_str != NULL)
	{
		change_setpoint(atoi(heading_str), 0, 0, false, true);
//		json_respond_ok("TODO: This probably won't work because distance=0");
	}
	else
	{
//		json_respond_error(argument_error);
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
		json_respond_ok(empty_string, id_short);
	}
	else
	{
		json_respond_error(argument_error, id_short);
	}
}


static inline void exec_turn_rel(void)
{
	char *heading_str = NEXT_STRING();

	if(heading_str != NULL)
	{
		change_setpoint(atoi(heading_str), 0, 0, true, true);
//		json_respond_ok("TODO: This probably won't work because distance=0");
	}
	else
	{
//		json_respond_error(argument_error);
	}
}


static inline bool is_long_command(token_t cmd)
{
	switch(cmd)
	{
	case TOKEN_MOVE:
	case TOKEN_SET:
	case TOKEN_TURN_ABS:
	case TOKEN_TURN_REL:
		return true;
		break;
	default:
		return false;
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
	int id;
	token_t command;

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

	if(interactive_mode)
	{
		id_short = -1;
		id_long = -1;
		command = find_token(strtok(input, delimiters));
	}
	else
	{
		id = atoi(strtok(input, delimiters));
		command = NEXT_TOKEN();
		if(is_long_command(command))
			id_long = id;
		else
			id_short = id;
	}

	switch(command)
	{
	case TOKEN_COMPASS_CALIBRATE:
		exec_compass_calibrate();
		break;
	case TOKEN_COMPASS_DUMP_EEPROM:
		exec_compass_dump_eeprom();
		break;
	case TOKEN_COMPASS_FLAT:
		exec_compass_flat();
		break;
	case TOKEN_COMPASS_RAMP:
		exec_compass_ramp();
		break;
	case TOKEN_COMPASS_READ_EEPROM:
		exec_compass_read_eeprom();
		break;
	case TOKEN_COMPASS_READ_RAM:
		exec_compass_read_ram();
		break;
	case TOKEN_COMPASS_RESET:
		exec_compass_reset();
		break;
	case TOKEN_COMPASS_START_CALIBRATION:
		exec_compass_start_calibration();
		break;
	case TOKEN_COMPASS_STOP_CALIBRATION:
		exec_compass_stop_calibration();
		break;
	case TOKEN_COMPASS_WRITE_EEPROM:
		exec_compass_write_eeprom();
		break;
	case TOKEN_COMPASS_WRITE_RAM:
		exec_compass_write_ram();
		break;
	case TOKEN_HEADING:
		exec_heading();
		break;
	case TOKEN_HEADING_ACCURACY:
		exec_heading_accuracy();
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
	case TOKEN_LEFT_CLOSE:
		exec_left_close();
		break;
	case TOKEN_LEFT_DOWN:
		exec_left_down();
		break;
	case TOKEN_LEFT_DROP:
		exec_left_drop();
		break;
	case TOKEN_LEFT_GRAB:
		exec_left_grab();
		break;
	case TOKEN_LEFT_OPEN:
		exec_left_open();
		break;
	case TOKEN_LEFT_UP:
		exec_left_up();
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
	case TOKEN_RESET_SERVOS:
		exec_reset_servos();
		break;
	case TOKEN_RAMP:
		exec_ramp();
		break;
	case TOKEN_RIGHT_CLOSE:
		exec_right_close();
		break;
	case TOKEN_RIGHT_DOWN:
		exec_right_down();
		break;
	case TOKEN_RIGHT_DROP:
		exec_right_drop();
		break;
	case TOKEN_RIGHT_GRAB:
		exec_right_grab();
		break;
	case TOKEN_RIGHT_OPEN:
		exec_right_open();
		break;
	case TOKEN_RIGHT_UP:
		exec_right_up();
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
		json_respond_error("unrecognized command", id);
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
