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

#include <stdbool.h>
#include "motor.h"

//#define PID_IGNORE_HEADING
//#define PID_CONTROL_SPEED

#define PID_MOTOR_KP			15
#define PID_MOTOR_KI			7		// Ki/0.005; Ki=0.01
#define PID_MOTOR_KD			0		// Kd*0.005; Kd=1000
#define PID_MOTOR_ISUM_MIN		-10000
#define PID_MOTOR_ISUM_MAX		10000

#define PID_HEADING_KP			10
#define PID_HEADING_KI			0
#define PID_HEADING_KD			0
#define PID_HEADING_ISUM_MIN	-10000000
#define PID_HEADING_ISUM_MAX	10000000
#define PID_HEADING_TOLERANCE	200		// Robot will stop and turn in place to correct
										// heading if error is greater than this
#define PID_NUM_SAMPLES 		128		// Number of samples to save in memory after changing the setpoint


typedef struct sample {
	volatile short unsigned int error;
	volatile short unsigned int output;
} sample_t;


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
	volatile sample_t samples[PID_NUM_SAMPLES];
	volatile unsigned short int sample_counter;
	bool enabled;
} controller_t;

void compute_next_pid_iteration(void);
void init_controller(controller_t *controller,
					 int p_const,
					 int i_const,
					 int d_const,
					 long i_sum_min,
					 long i_sum_max);
void change_setpoint(int heading_sp,
					 int motor_sp,
					 unsigned long new_distance,
					 bool heading_is_relative,
					 bool reset);
void init_heading_controller(void);
void change_heading_constants(int p, int i, int d);
void change_motor_constants(int p, int i, int d);
void pid_enable(void);
void pid_disable(void);
bool pid_is_enabled(void);

extern controller_t heading_pid;

#endif /* PID_H_ */
