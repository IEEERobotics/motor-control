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

#ifndef PID_H_
#define PID_H_

#include "motor.h"

/**
 * @struct controller
 *
 * Contains the state of a PID controller
 */
typedef struct controller {
	float p_const;		//!< P constant
	float i_const;		//!< I constant
	float d_const;		//!< D constant
	int i_sum;			//!< Sum of all previous errors (basically a Riemann sum)
	int i_sum_min;		//!< Max and min values for i_sum. This prevents "integral
	int i_sum_max;		//   windup".
	int prev_input;		//!< The last input, used for computing the derivative term
	int setpoint;		//!< The value at which the controller will attempt to converge
} controller_t;

void compute_pid(motor_t *motor);
void init_controller(controller_t *controller);
void change_setpoint(motor_t *motor, int sp);

#endif /* PID_H_ */
