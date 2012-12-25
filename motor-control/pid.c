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
#include "motor.h"

#define LIMIT(x, min, max)	((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x))


/**
 * More abstract implementation of a PID controller
 *
 * @param pid Pointer to a controller_t struct
 * @param current_value Output from plant
 * @return Input to plant
 */
static inline int compute_pid(controller_t *pid, int current_value)
{
	int p, i, d;
	int error = pid->setpoint - current_value;

	pid->i_sum += error;
	pid->i_sum = LIMIT(pid->i_sum, pid->i_sum_min, pid->i_sum_max);

	p = pid->p_const * error;
	i = pid->i_const * pid->i_sum;
	d = pid->d_const * (error - pid->prev_input);

	pid->prev_input = error;

	return p + i + d;
}


/**
 * Updates the motor response using a PID algorithm
 *
 * @param motor Motor to update
 */
void compute_motor_pid(motor_t *motor)
{
	controller_t *pid = &(motor->controller);
	int current_speed = *(motor->reg.enc) ? (ENC_SAMPLE_HZ / (unsigned short int)*(motor->reg.enc)) : 0;
	int plant = compute_pid(pid, error);

	motor->response.pwm = LIMIT(plant, 0, PWM_PERIOD);

	if(motor->sample_counter < NUM_SAMPLES)
	{
		motor->samples[motor->sample_counter].enc = current_speed;
		motor->samples[motor->sample_counter].pwm = motor->response.pwm;
		motor->sample_counter++;
	}
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
	controller->enabled = 0;
}


/**
 * Change the PID setpoint of a motor_t
 *
 * @param motor Pointer to the motor_t struct
 * @param sp The new setpoint
 *
 */
void change_setpoint(motor_t *motor, int sp)
{
	motor->controller.setpoint = sp;
	motor->sample_counter = 0;
}
