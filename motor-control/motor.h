/**
 * @file
 * @author Ethan LaMaster <ealamast@ncsu.edu>
 * @version 0.1
 *
 * @section Description
 *
 * Functions to control the motors, including controlling the power delivered (PWM
 * duty cycle) and initializing peripheral registers.
 *
 */

#ifndef MOTOR_H_
#define MOTOR_H_

/* Predeclaring this struct is necessary because of circular references in pid.h */
struct motor;
typedef struct motor motor_t;

#include <avr/io.h>
#include "pid.h"

#define PWM_PERIOD 		10000
#define ENC_SAMPLE_HZ	(32000000/64)
#define PWM_PORT0		PORTD
#define PWM_PORT1		PORTF
#define PWM_TIMER0  	TCD0
#define PWM_TIMER1  	TCF0
#define ENC_TIMER0  	TCC1
#define ENC_TIMER1  	TCD1
#define ENC_TIMER2  	TCE1
#define ENC_TIMER3  	TCF1
#define NUM_SAMPLES 	256		// Number of samples to save in memory after changing the setpoint


/**
 * @enum direction
 *
 * Direction for the motor to spin.
 *
 * DIR_BRAKE_H and DIR_BRAKE_L are equivalent for all practical purposes
 * (and both cause the motor to brake regardless of the PWM value in the
 * motor response).
 */
typedef enum direction {
  DIR_FORWARD,
  DIR_REVERSE,
  DIR_BRAKE
} direction_t;


/**
 * @struct motor_reg
 *
 * Contains pointers to the hardware registers that interface with the motors
 *
 */
typedef struct motor_reg {
	register16_t *pwma;		///< PWM output register A
	register16_t *pwmb;		///< PWM output register B
	register16_t *enc;		///< Quadrature encoder input
} motor_reg_t;


/**
 * @struct motor_response
 *
 * Contains the PWM duty cycle and direction of rotation.
 *
 * DIR_BRAKE_H and DIR_BRAKE_L override the pwm field.
 */
typedef struct motor_response {
	direction_t dir;
	int pwm;
} motor_response_t;


typedef struct sample {
	short unsigned int pwm;
	short unsigned int enc;
} sample_t;


/**
 * @struct motor
 *
 * Container for the three structs that define a motor.
 */
struct motor {
	motor_reg_t reg;
	motor_response_t response;
	controller_t controller;
	sample_t samples[NUM_SAMPLES];
	short int sample_counter;
	int encoder_count;
};

extern motor_t motor_a, motor_b, motor_c, motor_d;

void init_motor(motor_t *motor, register16_t *pwma, register16_t *pwmb, register16_t *enc);
void init_motor_reg(motor_reg_t *reg, register16_t *pwma, register16_t *pwmb, register16_t *enc);
void init_motor_port(PORT_t *port);
void change_setpoint(motor_t *motor, int sp);
void change_direction(motor_t *motor, direction_t dir);
void change_pwm(motor_t *motor, int pwm);
void update_speed(motor_t *motor);
void init_motors(void);


#endif /* MOTOR_H_ */
