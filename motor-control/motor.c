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
 * @section Motor Structures
 *
 * The state of a motor in software is described using these three structs:
 * <b>motor_reg_t:</b> Contains pointers that point directly to hardware registers.
 * These registers are used to set the PWM duty cycle and read the period of the
 * motor's quadrature encoder. These registers are the lowest level of control available
 * in software, and should only be written to through update_speed().
 *
 * <b>motor_response_t:</b> This struct defines the PWM duty cycle (or, average power)
 * delivered to the motor, and the direction of rotation. The update_speed() function
 * reads the values in this struct, and writes to the special function registers pointed
 * to by the motor_reg_t.
 *
 * <b>controller_t:</b> This struct contains the internal state of a PID controller. More
 * information on the software PID controller is (will be) in pid.c
 *
 * By instantiating each of these structs, one motor can be represented in software.
 * Instead of manually keeping up with three separate struct instances for each of the four
 * motors, a "super struct", motor_t, is used as a container. Most of these functions
 * simply accept a motor_t as an argument.
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "motor.h"
#include "pid.h"
#include "debug.h"
#include "timer.h"

/**
 * Structs representing the four motors
 */
motor_t motor_a, motor_b, motor_c, motor_d;


/**
 * Initializes a motor port
 *
 * @param port Pointer to the port to initialize
 *
 * @todo Implement this function.
 */
void init_motor_port(PORT_t *port)
{
	/* Set pins 0-3 as outputs (PWM signal) and pins 4-7 as inputs (quadrature encoders) */
	port->DIR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;
	port->INTCTRL = PORT_INT1LVL_HI_gc | PORT_INT0LVL_HI_gc;
	port->INT0MASK = PIN4_bm | PIN6_bm;
	port->INT1MASK = PIN5_bm | PIN7_bm;
	PMIC.CTRL |= PMIC_HILVLEN_bm;
}


/**
 * Initializes a motor_response_t struct
 *
 * @param resp Pointer to the motor_response_t struct to initialize
 *
 * @todo Implement this function.
 */
void init_motor_response(motor_response_t *resp)
{
	resp->dir = DIR_BRAKE;
	resp->pwm = 0;
}


/**
 * Initializes a motor_reg_t struct
 *
 * @param reg Pointer to the motor_t struct to initialize
 * @param pwma First timer compare register for PWM signal generation
 * @param pwmb Second timer compare register for PWM signal generation
 * @param enc Encoder counter register
 *
 * @todo Implement this function.
 */
void init_motor_reg(motor_reg_t *reg,
					register16_t *pwma,
					register16_t *pwmb,
					register16_t *enc)
{
	reg->pwma = pwma;
	reg->pwmb = pwmb;
	reg->enc = enc;
}


/**
 * Initializes a motor_t struct
 *
 * @param motor Pointer to the motor_t struct to initialize
 *
 * @todo Implement this function.
 */
void init_motor(motor_t *motor,
				register16_t *pwma,
				register16_t *pwmb,
				register16_t *enc)
{
	init_motor_reg(&(motor->reg), pwma, pwmb, enc);
	init_motor_response(&(motor->response));
	init_controller(&(motor->controller),
					PID_MOTOR_KP,
					PID_MOTOR_KI,
					PID_MOTOR_KD,
					PID_MOTOR_ISUM_MIN,
					PID_MOTOR_ISUM_MAX);
	motor->encoder_count = 0;
	motor->prev_encoder_count = 0;
}


/**
 * Top-level function to initialize the motors
 */
void init_motors(void)
{
	/* Connect the first 4 event channels to the quadrature encoder inputs.
	 * We have to use the event system because two timers are connected to port E,
	 * which does not have a header on our development board (that port is connected
	 * to the board's 8 LEDs)
	 */
	EVSYS.CH0MUX = EVSYS_CHMUX_PORTD_PIN4_gc;
	EVSYS.CH1MUX = EVSYS_CHMUX_PORTD_PIN5_gc;
	EVSYS.CH2MUX = EVSYS_CHMUX_PORTF_PIN4_gc;
	EVSYS.CH3MUX = EVSYS_CHMUX_PORTF_PIN5_gc;

	/* Properly set the direction register for the two motor ports */
	init_motor_port(&PWM_PORT0);
	init_motor_port(&PWM_PORT1);

	/* Initialize the timers responsible for generating PWM signals */
	init_pwm_timer(&PWM_TIMER0);
	init_pwm_timer(&PWM_TIMER1);

	/* Initialize the timers responsible for measuring the quadrature encoder period. */
	init_enc_timer(&ENC_TIMER0, TC_EVSEL_CH0_gc);
	init_enc_timer(&ENC_TIMER1, TC_EVSEL_CH1_gc);
	init_enc_timer(&ENC_TIMER2, TC_EVSEL_CH2_gc);
	init_enc_timer(&ENC_TIMER3, TC_EVSEL_CH3_gc);

	/* Initialize the 4 motor_t structs */
	init_motor(&motor_a, &(PWM_TIMER0.CCA), &(TCD0.CCB), &(ENC_TIMER0.CCABUF));
	init_motor(&motor_b, &(PWM_TIMER0.CCC), &(TCD0.CCD), &(ENC_TIMER1.CCABUF));
	init_motor(&motor_c, &(PWM_TIMER1.CCA), &(TCF0.CCC), &(ENC_TIMER2.CCABUF));
	init_motor(&motor_d, &(PWM_TIMER1.CCB), &(TCF0.CCD), &(ENC_TIMER3.CCABUF));
}


/**
 * Change the direction of a motor
 *
 * The change in direction doesn't take place until update_speed() is called.
 *
 * @param motor Pointer to the motor_t struct to modify
 * @param dir New direction
 */
void change_direction(motor_t *motor, direction_t dir)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		motor->response.dir = dir;
	}
}


/**
 * Change the PWM duty cycle of a motor
 *
 * The change in direction doesn't take place until update_speed() is called.
 *
 * @param motor Pointer to the motor_t struct to modify
 * @param pwm New PWM value
 */
void change_pwm(motor_t *motor, int pwm)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		motor->response.pwm = pwm;
	}
}


/**
 * Updates the motor speed based on the motor's motor_response_t
 *
 * Updates the appropriate hardware registers to change the PWM duty cycle
 *
 * @param motor Motor to update
 *
 */
void update_speed(motor_t *motor)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		switch(motor->response.dir)
		{
		case DIR_BRAKE:
			*(motor->reg.pwma) = 0;
			*(motor->reg.pwmb) = 0;
			break;
		case DIR_FORWARD:
			*(motor->reg.pwma) = motor->response.pwm;
			*(motor->reg.pwmb) = 0;
			break;
		case DIR_REVERSE:
			*(motor->reg.pwma) = 0;
			*(motor->reg.pwmb) = motor->response.pwm;
			break;
		}
	}
}


void clear_encoder_count(void)
{
#if NUM_MOTORS == 4
	MOTOR_LEFT_FRONT.encoder_count = 0;
	MOTOR_LEFT_FRONT.prev_encoder_count = 0;
	MOTOR_LEFT_FRONT.reg.enc = 0;

	MOTOR_LEFT_BACK.encoder_count = 0;
	MOTOR_LEFT_BACK.prev_encoder_count = 0;
	MOTOR_LEFT_BACK.reg.enc = 0;

	MOTOR_RIGHT_FRONT.encoder_count = 0;
	MOTOR_RIGHT_FRONT.prev_encoder_count = 0;
	MOTOR_RIGHT_FRONT.reg.enc = 0;

	MOTOR_RIGHT_BACK.encoder_count = 0;
	MOTOR_RIGHT_BACK.prev_encoder_count = 0;
	MOTOR_RIGHT_BACK.reg.enc = 0;
#elif NUM_MOTORS == 2
	MOTOR_LEFT.encoder_count = 0;
	MOTOR_LEFT.prev_encoder_count = 0;
	MOTOR_LEFT.reg.enc = 0;

	MOTOR_RIGHT.encoder_count = 0;
	MOTOR_RIGHT.prev_encoder_count = 0;
	MOTOR_RIGHT.reg.enc = 0;
#endif
}


ISR(PORTD_INT0_vect)
{
	DEBUG_ENTER_ISR(DEBUG_ISR_ENCODER);
	motor_a.encoder_count++;
	DEBUG_EXIT_ISR(DEBUG_ISR_ENCODER);
}


ISR(PORTD_INT1_vect)
{
	DEBUG_ENTER_ISR(DEBUG_ISR_ENCODER);
	motor_b.encoder_count++;
	DEBUG_EXIT_ISR(DEBUG_ISR_ENCODER);
}


ISR(PORTF_INT0_vect)
{
	DEBUG_ENTER_ISR(DEBUG_ISR_ENCODER);
	motor_c.encoder_count++;
	DEBUG_EXIT_ISR(DEBUG_ISR_ENCODER);
}


ISR(PORTF_INT1_vect)
{
	DEBUG_ENTER_ISR(DEBUG_ISR_ENCODER);
	motor_d.encoder_count++;
	DEBUG_EXIT_ISR(DEBUG_ISR_ENCODER);
}
