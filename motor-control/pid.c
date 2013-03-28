/**
 * @file
 * @author Ethan LaMaster <ealamast@ncsu.edu>
 * @version 0.1
 *
 * @section Description
 *
 * Functions implementing a software PID controller
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <util/atomic.h>
#include "debug.h"
#include "motor.h"
#include "compass.h"
#include "timer.h"
#include "pid.h"

#define LIMIT(x, min, max)	((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x))

static int heading_setpoint;
static int motor_setpoint;		// either speed or distance, depending on PID_CONTROL_SPEED
controller_t heading_pid;
static bool pid_enabled = false;
static unsigned long int time = 0;
static unsigned long distance = 0;
static bool turning = false;

static inline void reset_controller(controller_t *c)
{
	c->i_sum = 0;
	c->prev_input = 0;
	c->sample_counter = 0;
}


/**
 * More abstract implementation of a PID controller
 *
 * @param pid Pointer to a controller_t struct
 * @param error Difference between setpoint and plant output
 * @return Input to plant
 */
static inline int compute_pid(controller_t *pid, int error)
{
	int p, i, d, output;
	volatile sample_t *sample;

	p = pid->p_const * error;
	i = pid->i_sum * pid->i_const;
	d = pid->d_const * (error - pid->prev_input);

	pid->i_sum += error;
	pid->i_sum = LIMIT(pid->i_sum, pid->i_sum_min, pid->i_sum_max);

	pid->prev_input = error;
//	output = LIMIT(p + i + d, 0, PWM_PERIOD);
	output = p + i + d;

	if(pid->sample_counter < PID_NUM_SAMPLES)
	{
		sample = &(pid->samples[pid->sample_counter++]);
		sample->error = error;
		sample->output = output;
	}

	return output;
}


static inline int get_motor_pv(motor_t *motor)
{
#ifndef PID_CONTROL_SPEED
	return *(motor->reg.enc) ? (ENC_SAMPLE_HZ / (unsigned short int)*(motor->reg.enc)) : 0;

	// units are ticks/min
//	unsigned long int pv = (motor->encoder_count - motor->prev_encoder_count) * (60000u/MS_TIMER_PER);
//	motor->prev_encoder_count = motor->encoder_count;
//	return pv;
#else
	return motor->encoder_count;
#endif
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

	if((motor->encoder_count < distance) || turning)
	{
		if(setpoint > 0)
		{
			motor->response.dir = DIR_FORWARD;
		}
		else if(setpoint < 0)
		{
			motor->response.dir = DIR_REVERSE;
			setpoint = -setpoint;
		}

		error = setpoint - get_motor_pv(motor);
		mv = compute_pid(&(motor->controller), error);

		motor->response.pwm = LIMIT(mv, 0, PWM_PERIOD);
	}
	else	// We've reached the target distance
	{
		motor->response.pwm = 0;
	}
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


static inline int get_motor_setpoint(motor_t *m)
{
	return motor_setpoint;
}


/**
 * Top-level function to calculate the next heading and speed PID iteration
 */
void compute_next_pid_iteration(void)
{
	uint16_t current_heading;	// Current absolute heading
	int heading_error;			// Error in heading
	int heading_mv;				// Heading manipulated variable (output of heading PID)
	int left_setpoint;
	int right_setpoint;

#ifdef PID_IGNORE_HEADING
	left_setpoint = get_motor_setpoint(&MOTOR_LEFT);
	right_setpoint = get_motor_setpoint(&MOTOR_RIGHT);
#else
	while(! compass_read(&current_heading));	// Get current heading, run again if error

	heading_error = normalize_heading(heading_setpoint - current_heading);

	if(abs(heading_error) < PID_HEADING_TOLERANCE)
	{
		if(turning)
		{
			turning = false;
			clear_encoder_count();
		}
		left_setpoint = motor_setpoint;
		right_setpoint = motor_setpoint;
	}
	else	// execute a turn
	{
		turning = true;
		left_setpoint = 0;
		right_setpoint = 0;
	}

	heading_mv = compute_pid(&heading_pid, heading_error) / 10;
	right_setpoint -= heading_mv;
	left_setpoint += heading_mv;
#endif

#if NUM_MOTORS == 4
	compute_motor_pid(&MOTOR_LEFT_FRONT, left_setpoint);
	compute_motor_pid(&MOTOR_LEFT_BACK, left_setpoint);
	compute_motor_pid(&MOTOR_RIGHT_FRONT, right_setpoint);
	compute_motor_pid(&MOTOR_RIGHT_BACK, right_setpoint);
#elif NUM_MOTORS == 2
	compute_motor_pid(&MOTOR_LEFT, left_setpoint);
	compute_motor_pid(&MOTOR_RIGHT, right_setpoint);
#endif

//	printf("%d %d\r\n",
//			heading_error,
//			heading_mv);
	time++;
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
	controller->sample_counter = 0;
}


/**
 * Change the controller setpoint
 *
 * @param heading_sp New heading setpoint
 * @param motor_sp New motor speed setpoint
 * @param new_distance New target distance
 * @param heading_is_relative True if heading is relative to the current position of the robot,
 * 		  or false if it is absolute.
 * @param reset Reset the controllers after setting the constants
 */
void change_setpoint(int heading_sp,
					 int motor_sp,
					 unsigned long new_distance,
					 bool heading_is_relative,
					 bool reset)
{
	pid_enabled = false;

	uint16_t current_heading;
	int new_heading_setpoint;

	/* Calculate absolute heading, add or subtract 360 degrees if necessary */
	if(heading_is_relative)
	{
		while(! compass_read(&current_heading));	// Get current heading, run again if error
		new_heading_setpoint = normalize_heading(heading_sp + current_heading);
	}
	else
		new_heading_setpoint = heading_sp;

	if(reset)
	{
		reset_controller(&heading_pid);
		reset_controller(&(motor_a.controller));
		reset_controller(&(motor_b.controller));
		reset_controller(&(motor_c.controller));
		reset_controller(&(motor_d.controller));
		clear_encoder_count();
		time = 0;
	}

	heading_setpoint = new_heading_setpoint;
	motor_setpoint = motor_sp;
	distance = new_distance;

	pid_enabled = true;
}


void init_heading_controller(void)
{
	init_controller(&heading_pid,
					PID_HEADING_KP,
					PID_HEADING_KI,
					PID_HEADING_KD,
					PID_HEADING_ISUM_MIN,
					PID_HEADING_ISUM_MAX);
}


void change_heading_constants(int p, int i, int d)
{
	pid_enabled = false;

	init_controller(&heading_pid,
					p,
					i,
					d,
					PID_HEADING_ISUM_MIN,
					PID_HEADING_ISUM_MAX);
}


void change_motor_constants(int p, int i, int d)
{
	pid_enabled = false;

	init_controller(&(motor_a.controller),
					p,
					i,
					d,
					PID_MOTOR_ISUM_MIN,
					PID_MOTOR_ISUM_MAX);
	init_controller(&(motor_b.controller),
					p,
					i,
					d,
					PID_MOTOR_ISUM_MIN,
					PID_MOTOR_ISUM_MAX);
	init_controller(&(motor_c.controller),
					p,
					i,
					d,
					PID_MOTOR_ISUM_MIN,
					PID_MOTOR_ISUM_MAX);
	init_controller(&(motor_d.controller),
					p,
					i,
					d,
					PID_MOTOR_ISUM_MIN,
					PID_MOTOR_ISUM_MAX);
}


void pid_enable(void)
{
	pid_enabled = true;
}


void pid_disable(void)
{
	pid_enabled = false;
}


bool pid_is_enabled(void)
{
	return pid_enabled;
}
