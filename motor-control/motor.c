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
#include "motor.h"
#include "pid.h"

/**
 * Structs representing the four motors
 */
motor_t motor_a, motor_b, motor_c, motor_d;

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
	timer->CTRLA = TC_CLKSEL_DIV64_gc;
	timer->CTRLB = TC_WGMODE_NORMAL_gc | TC1_CCAEN_bm;
	timer->CTRLD = TC_EVACT_FRQ_gc | event_channel;
	timer->PERBUF = 0xffff;
}


/**
 * Initialize the millisecond timer.
 *
 * This will be used to generate an interrupt at regular intervals, in which the next
 * PID iteration will be computed and the motor output speeds updated.
 *
 * This doesn't actually have to occur every millisecond, and the period will likely
 * have to be tweaked depending on the response of the motors. The period is controlled
 * by the MS_TIMER_PER macro in motor.h.
 */
void init_ms_timer(void)
{
	MS_TIMER.CTRLA = TC_CLKSEL_DIV64_gc;			// Clock source is system clock
	MS_TIMER.CTRLB = TC_WGMODE_NORMAL_gc;		// Normal waveform generation mode
	MS_TIMER.INTCTRLA = TC_OVFINTLVL_MED_gc;	// Medium priority interrupt
	MS_TIMER.PER = 500 * MS_TIMER_PER;			// Timer period

	PMIC.CTRL |= PMIC_MEDLVLEN_bm;				// Enable medium level interrupts
	sei();
}


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
	init_controller(&(motor->controller));
	motor->sample_counter = 0;
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
	init_motor(&motor_a, &(PWM_TIMER0.CCA), &(TCD0.CCB), &(ENC_TIMER0.CCA));
	init_motor(&motor_b, &(PWM_TIMER0.CCC), &(TCD0.CCD), &(ENC_TIMER1.CCA));
	init_motor(&motor_c, &(PWM_TIMER1.CCA), &(TCF0.CCB), &(ENC_TIMER2.CCA));
	init_motor(&motor_d, &(PWM_TIMER1.CCC), &(TCF0.CCD), &(ENC_TIMER3.CCA));
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
	motor->response.dir = dir;
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
	motor->response.pwm = pwm;
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
	switch(motor->response.dir)
	{
	case DIR_BRAKE:
		motor->reg.pwma = 0;
		motor->reg.pwmb = 0;
		break;
	case DIR_FORWARD:
		*(motor->reg.pwma) = motor->response.pwm;
		motor->reg.pwmb = 0;
		break;
	case DIR_REVERSE:
		motor->reg.pwma = 0;
		*(motor->reg.pwmb) = motor->response.pwm;
		break;
	}
}


/**
 * MS_TIMER interrupt service routine
 */
ISR(TCE0_OVF_vect)
{
	if(motor_a.controller.enabled)
	{
		compute_pid(&motor_a);
		update_speed(&motor_a);
	}

	if(motor_b.controller.enabled)
	{
		compute_pid(&motor_b);
		update_speed(&motor_b);

	}

	if(motor_c.controller.enabled)
	{
		compute_pid(&motor_c);
		update_speed(&motor_c);
	}

	if(motor_d.controller.enabled)
	{
		compute_pid(&motor_d);
		update_speed(&motor_d);
	}
}
