/**
 * @file
 * @author Ethan LaMaster <ealamast@ncsu.edu>
 * @version 0.1
 *
 * @section Description
 *
 * Functions implementing a software PID controller
 *
 */

#include "motor.h"

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

	motor->response.pwm = (pwm_t) p + i - d;
}


/**
 * Initializes a controller_t struct
 *
 * @param controller Controller to initialize
 *
 * @todo Implement this function.
 */
void init_controller(controller_t *controller)
{
	// Just a stub
}
