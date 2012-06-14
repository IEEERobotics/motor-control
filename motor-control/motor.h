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

#include <avr/io.h>
#include "pid.h"

#define PWM_PERIOD 10000

#define PWM_TIMER0 TCC0
#define PWM_TIMER1 TCD0
#define ENC_TIMER0 TCC1
#define ENC_TIMER1 TCD1
#define ENC_TIMER2 TCE1
#define ENC_TIMER3 TCF1
#define MS_TIMER   TCE0


typedef short int pwm_t;
typedef unsigned short int speed_t;


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
  DIR_BACKWARD,
  DIR_BRAKE_H,
  DIR_BRAKE_L
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
	short int pwm;
} motor_response_t;


/**
 * @struct motor
 *
 * Container for the three structs that define a motor.
 */
typedef struct motor {
	motor_reg_t reg;
	motor_response_t response;
	controller_t controller;
} motor_t;


void init_motor(motor_t *motor);
void init_pwm_timer(TC0_t *timer);
void init_enc_timer(TC1_t *timer, TC_EVSEL_t event_channel);
void init_enc_port(PORT_t *port);
void change_setpoint(motor_t *motor, int sp);
void change_direction(motor_t *motor, direction_t dir);
void update_speed(motor_t *motor);


#endif /* MOTOR_H_ */
