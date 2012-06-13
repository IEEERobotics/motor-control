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

#include <avr/io.h>
#include "motor.h"
#include "error.h"

/**
 * Initializes a PWM timer
 *
 * Configures the timer to generate a single-slope PWM waveform, sets the period
 * register to PWM_PERIOD, enables all four capture/compare registers, and sets
 * each capture/compare register to 0.
 *
 * Note that this function only accepts a TC0_t. Pins 0-3 of the timer's corresponding
 * port will act as PWM outputs. Each H-bridge requires two PWM inputs, hence one timer
 * can drive two motors.
 *
 * @param timer Pointer to a TC0_t to initialize
 *
 * @return void
 */
void init_pwm_timer(TC0_t *timer)
{
	timer->CTRLA = TC_CLKSEL_DIV1_gc;
	timer->CTRLB = TC_WGMODE_SS_gc
				 | TC0_CCAEN_bm
				 | TC0_CCBEN_bm
				 | TC0_CCCEN_bm
				 | TC0_CCDEN_bm;
	timer->CTRLD = TC_EVACT_OFF_gc | TC_EVSEL_OFF_gc;
	timer->PERBUF = PWM_PERIOD;
	timer->CCABUF = 0;
	timer->CCBBUF = 0;
	timer->CCCBUF = 0;
	timer->CCDBUF = 0;
}


/**
 * Initializes an encoder timer
 *
 * Configures a timer to capture waveform frequency from an event channel.
 *
 * @param timer Pointer to a TC1_t to initialize
 * @param event_channel Event channel to use for frequency capture.
 *
 */
void init_enc_timer(TC1_t *timer, TC_EVSEL_t event_channel)
{
	timer->CTRLA = TC_CLKSEL_DIV1_gc;
	timer->CTRLB = TC_WGMODE_NORMAL_gc | TC1_CCAEN_bm;
	timer->CTRLD = TC_EVACT_FRQ_gc | event_channel;
	timer->PERBUF = 0xffff;
}


/**
 * Initializes an encoder port
 *
 * @param port Pointer to the port to initialize
 *
 * @todo Implement this function.
 */
void init_enc_port(PORT_t *port)
{
	// Just a stub
}


/**
 * Initializes a motor_respobse_t struct
 *
 * @param resp Pointer to the motor_response_t struct to initialize
 *
 * @todo Implement this function.
 */
void init_motor_response(motor_response_t *resp)
{
	// Just a stub
}


/**
 * Initializes a motor_reg_t struct
 *
 * @param reg Pointer to the motor_t struct to initialize
 *
 * @todo Implement this function.
 */
void init_motor_reg(motor_reg_t *reg)
{
	// Just a stub
}


/**
 * Initializes a motor_t struct
 *
 * @param motor Pointer to the motor_t struct to initialize
 *
 * @todo Implement this function.
 */
void init_motor(motor_t *motor)
{
	init_motor_reg(&(motor->reg));
	init_motor_response(&(motor->response));
	init_controller(&(motor->controller));
}


/**
 * Change the PID setpoint of a motor_t
 *
 * @param motor Pointer to the motor_t struct
 * @param sp The new setpoint
 *
 * @todo Implement this function.
 */
void change_setpoint(motor_t *motor, int sp)
{
	// Just a stub
}


/**
 * Change the direction of a motor
 *
 * The change in direction doesn't take place until update_speed() is called.
 *
 * @param motor Pointer to the motor_t struct to initialize
 * @param dir New direction
 *
 * @todo Implement this function.
 */
void change_direction(motor_t *motor, direction_t dir)
{
	// Just a stub
}


/**
 * Updates the motor speed based on the motor's motor_response_t
 *
 * Updates the appropriate hardware registers to change the PWM duty cycle
 *
 * @param motor Motor to update
 *
 * @todo Implement this function.
 */
void update_speed(motor_t *motor)
{
	// Just a stub
}



