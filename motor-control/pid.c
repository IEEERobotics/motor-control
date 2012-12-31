/**
 * @file
 * @author Ethan LaMaster <ealamast@ncsu.edu>
 * @version 0.1
 *
 * @section Description
 *
 * Functions implementing a software PID controller
 *
 * @todo Set up timer interrupt to run compute_pid()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <util/atomic.h>
#include "motor.h"
#include "compass.h"

#define LIMIT(x, min, max)	((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x))

int heading_setpoint, speed_setpoint;
controller_t heading_pid;
bool pid_enabled = false;

/**
 * More abstract implementation of a PID controller
 *
 * @param pid Pointer to a controller_t struct
 * @param error Difference between setpoint and plant output
 * @return Input to plant
 */
static inline int compute_pid(controller_t *pid, int error)
{
	int p, i, d;

	pid->i_sum += error;
	pid->i_sum = LIMIT(pid->i_sum, pid->i_sum_min, pid->i_sum_max);

	p = pid->p_const * error;
	i = pid->i_const * pid->i_sum;
	d = pid->d_const * (error - pid->prev_input);

	pid->prev_input = error;

	return p + i + d;
}


static inline int get_motor_speed(motor_t *motor)
{
	return *(motor->reg.enc) ? (ENC_SAMPLE_HZ / (unsigned short int)*(motor->reg.enc)) : 0;
}


/**
 * Compute the next PID iteration for a motor, and update the motor_response struct.
 *
 * @param motor Motor to operate on
 * @param setpoint Signed target setpoint
 */
static inline void compute_motor_pid(motor_t *motor, int setpoint)
{
	int error;
	int mv;

	if(setpoint > 0)
	{
		motor->response.dir = DIR_FORWARD;
	}
	else if(setpoint < 0)
	{
		motor->response.dir = DIR_REVERSE;
	}

	error = abs(setpoint - get_motor_speed(motor));
	mv = compute_pid(&(motor->controller), error);

	motor->response.pwm = LIMIT(mv, 0, PWM_PERIOD);
}


/**
 * Normalize a heading in the range -180 to 180 degrees
 */
static inline int normalize_heading(int heading)
{
	if(heading > 1800)
		heading -= 3600;
	else if(heading < -1800)
		heading += 3600;

	return heading;
}


/**
 * Top-level function to calculate the next heading and speed PID iteration
 */
void compute_next_pid_iteration(void)
{
	uint16_t current_heading;	// Current absolute heading
	int heading_error;			// Error in heading
	int heading_mv;				// Heading manipulated variable (output of heading PID)
	int left_speed_setpoint;
	int right_speed_setpoint;

	while(! compass_read(&current_heading));	// Get current heading, run again if error

	heading_error = normalize_heading(heading_setpoint - current_heading);

	if(heading_error < PID_HEADING_TOLERANCE)
	{
		left_speed_setpoint = speed_setpoint;
		right_speed_setpoint = speed_setpoint;
	}
	else
	{
		left_speed_setpoint = 0;
		right_speed_setpoint = 0;
	}

	heading_mv = compute_pid(&heading_pid, heading_error);
	right_speed_setpoint += heading_mv;
	left_speed_setpoint -= heading_mv;

	compute_motor_pid(&MOTOR_LEFT_FRONT, left_speed_setpoint);
	compute_motor_pid(&MOTOR_LEFT_BACK, left_speed_setpoint);
	compute_motor_pid(&MOTOR_RIGHT_FRONT, right_speed_setpoint);
	compute_motor_pid(&MOTOR_RIGHT_BACK, right_speed_setpoint);
}


/**
 * Initializes a controller_t struct
 *
 * @param controller Controller to initialize
 *
 */
void init_controller(controller_t *controller,
					 int p_const,
					 int i_const,
					 int d_const,
					 long i_sum_min,
					 long i_sum_max)
{
	controller->p_const = p_const;
	controller->i_const = i_const;
	controller->d_const = d_const;
	controller->i_sum_min = i_sum_min;
	controller->i_sum_max = i_sum_max;

	controller->i_sum = 0;
	controller->prev_input = 0;
	controller->setpoint = 0;
}


/**
 * Change the controller setpoint
 *
 * @param relative_heading New signed heading, relative to the front of the robot
 * @param speed New target speed
 */
void change_setpoint(int relative_heading, int speed)
{
	uint16_t current_heading;
	int new_heading_setpoint;

	while(! compass_read(&current_heading));	// Get current heading, run again if error

	/* Calculate absolute heading, add or subtract 360 degrees if necessary */
	new_heading_setpoint = normalize_heading(relative_heading + current_heading);

	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		heading_setpoint = new_heading_setpoint;
		speed_setpoint = speed;
		pid_enabled = true;
	}
}


void init_heading_controller(void)
{
	heading_pid.p_const = PID_HEADING_KP;
	heading_pid.i_const = PID_HEADING_KI;
	heading_pid.d_const = PID_HEADING_KD;
	heading_pid.i_sum_min = PID_HEADING_ISUM_MIN;
	heading_pid.i_sum_max = PID_HEADING_ISUM_MAX;

	heading_pid.i_sum = 0;
	heading_pid.prev_input = 0;
	heading_pid.setpoint = 0;
}
