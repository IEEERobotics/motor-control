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

#include "motor.h"

#define LIMIT(x, min, max)	((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x))

/**
 * Updates the motor response using a PID algorithm
 *
 * @param motor Motor to update
 *
 * @todo Implement this function.
 * @todo Verify that this code meets real-time processing constraints when
 * 		 using floating-point arithmetic
 */
void compute_pid(motor_t *motor)
{
	controller_t *pid = &(motor->controller);
	int current_speed = (int) *(motor->reg.enc);
	int error = pid->setpoint - current_speed;
	float p, i, d;

	pid->i_sum += error;
	pid->i_sum = LIMIT(pid->i_sum, pid->i_sum_min, pid->i_sum_max);

	p = pid->p_const * error;
	i = pid->i_const * pid->i_sum;
	d = pid->d_const * (current_speed - pid->prev_input);

	pid->prev_input = current_speed;

	motor->response.pwm = (int) p + i - d;

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
void init_controller(controller_t *controller)
{
	controller->d_const = 0;
	controller->i_const = 0;
	controller->p_const = 0;
	controller->i_sum_min = -10000;
	controller->i_sum_max = 10000;

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
