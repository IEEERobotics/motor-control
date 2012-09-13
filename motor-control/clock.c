/*
 * clock.c
 *
 *  Created on: Sep 13, 2012
 *      Author: eal
 */

#include <avr/io.h>

void init_clock(void)
{
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;					// 32 MHz RC Oscillator
	CLK.PSCTRL = CLK_PSADIV_1_gc | CLK_PSBCDIV_1_1_gc;	// Don't prescale clock
	CLK.RTCCTRL = CLK_RTCSRC_ULP_gc;					// Use 1 kHz clock for RTC

	CLK.LOCK = CLK_LOCK_bm;								// Prevent further changes
														// to the system clock
}
