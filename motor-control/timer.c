/**
 * @file
 *
 * Functions for initializing the timers
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "motor.h"
#include "debug.h"
#include "timer.h"

volatile uint8_t ms_timer = 0;

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
	timer->INTCTRLB = TC_CCAINTLVL_HI_gc;
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
	MS_TIMER.CTRLA = TC_CLKSEL_DIV64_gc;		// Clock source is system clock
	MS_TIMER.CTRLB = TC_WGMODE_NORMAL_gc;		// Normal waveform generation mode
	MS_TIMER.INTCTRLA = TC_OVFINTLVL_LO_gc;		// Low priority interrupt
	MS_TIMER.PER = 500 * MS_TIMER_PER;			// Timer period

	PMIC.CTRL |= PMIC_LOLVLEN_bm;				// Enable low priority interrupts
	sei();
}


/**
 * MS_TIMER interrupt service routine
 */
ISR(TCC0_OVF_vect)
{
	DEBUG_ENTER_ISR(DEBUG_ISR_MSTIMER);

	ms_timer++;

	if(pid_enabled)
	{
		compute_next_pid_iteration();

		update_speed(&MOTOR_LEFT_FRONT);
		update_speed(&MOTOR_LEFT_BACK);
		update_speed(&MOTOR_RIGHT_FRONT);
		update_speed(&MOTOR_RIGHT_BACK);
	}

	DEBUG_EXIT_ISR(DEBUG_ISR_MSTIMER);
}


ISR(TCC1_CCA_vect)
{
	DEBUG_ENTER_ISR(DEBUG_ISR_ENCODER);
	motor_a.encoder_count++;
	DEBUG_EXIT_ISR(DEBUG_ISR_ENCODER);
}


ISR(TCD1_CCA_vect)
{
	DEBUG_ENTER_ISR(DEBUG_ISR_ENCODER);
	motor_b.encoder_count++;
	DEBUG_EXIT_ISR(DEBUG_ISR_ENCODER);
}


ISR(TCE1_CCA_vect)
{
	DEBUG_ENTER_ISR(DEBUG_ISR_ENCODER);
	motor_c.encoder_count++;
	DEBUG_EXIT_ISR(DEBUG_ISR_ENCODER);
}


ISR(TCF1_CCA_vect)
{
	DEBUG_ENTER_ISR(DEBUG_ISR_ENCODER);
	motor_d.encoder_count++;
	DEBUG_EXIT_ISR(DEBUG_ISR_ENCODER);
}
