/**
 * @file
 *
 * Functions for initializing the timers
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "compass.h"
#include "timer.h"


/**
 * Initialize the millisecond timer.
 *
 * This will be used to generate an interrupt at regular intervals.
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


void init_servo_timer(void)
{
	PORTA.DIRSET = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;

	SERVO_TIMER.CTRLA = TC_CLKSEL_DIV64_gc;
	SERVO_TIMER.CTRLB = TC_WGMODE_NORMAL_gc
					  | TC0_CCAEN_bm
					  | TC0_CCBEN_bm
					  | TC0_CCCEN_bm
					  | TC0_CCDEN_bm;
	SERVO_TIMER.CTRLD = TC_EVACT_OFF_gc | TC_EVSEL_OFF_gc;
	SERVO_TIMER.PERBUF = 500 * SERVO_TIMER_PER;
	SERVO_TIMER.CCABUF = 0;
	SERVO_TIMER.CCBBUF = 0;
	SERVO_TIMER.CCCBUF = 0;
	SERVO_TIMER.CCDBUF = 0;
	SERVO_TIMER.INTCTRLA = TC_OVFINTLVL_HI_gc;
	SERVO_TIMER.INTCTRLB = TC_CCAINTLVL_HI_gc
						 | TC_CCBINTLVL_HI_gc
						 | TC_CCCINTLVL_HI_gc
						 | TC_CCDINTLVL_HI_gc;

	PMIC.CTRL |= PMIC_HILVLEN_bm;
	sei();
}


/**
 * MS_TIMER interrupt service routine
 */
ISR(TCC0_OVF_vect)
{

}
