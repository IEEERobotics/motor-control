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

#define PID_MOTOR_KP			20
#define PID_MOTOR_KI			2
#define PID_MOTOR_KD			5
#define PID_MOTOR_ISUM_MIN		-10000000
#define PID_MOTOR_ISUM_MAX		10000000

/**
 * @struct controller
 *
 * Contains the state of a PID controller
 */
typedef struct controller {
	int p_const;		//!< P constant
	int i_const;		//!< I constant
	int d_const;		//!< D constant
	long i_sum;			//!< Sum of all previous errors (basically a Riemann sum)
	long i_sum_min;		//!< Max and min values for i_sum. This prevents "integral
	long i_sum_max;		//   windup".
	int prev_input;		//!< The last input, used for computing the derivative term
	int setpoint;		//!< The value at which the controller will attempt to converge
	char enabled;		//!< True if PID is enabled, otherwise false
} controller_t;

void compute_motor_pid(motor_t *motor);
void init_controller(controller_t *controller,
					 int p_const,
					 int i_const,
					 int d_const,
					 long i_sum_min,
					 long i_sum_max);
void change_setpoint(motor_t *motor, int sp);

#endif /* PID_H_ */
