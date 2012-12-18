/*
 * clock.c
 *
 *  Created on: Sep 13, 2012
 *      Author: eal
 */

#include <avr/io.h>
#include "clksys_driver.h"
#include "clock.h"

/**
 * Initializes the system clock. Switches to the builtin 32 MHz ring oscillator.
 */
void init_clock(void)
{
	// Enable 32 MHz oscillator
	CLKSYS_Enable(OSC_RC32MEN_bm);

	// Wait for oscillator to stabilize
	while(CLKSYS_IsReady(OSC_RC32MRDY_bm) == 0);

	// Switch clock source to 32 MHz oscillator
	CLKSYS_Main_ClockSource_Select(CLK_SCLKSEL_RC32M_gc);

	// Turn off unused clock sources
	CLKSYS_Disable(OSC_RC32MEN_bm | OSC_RC32KEN_bm);

//	PORTC.DIRSET = PIN7_bm;
//	PORTCFG.CLKEVOUT = PORTCFG_CLKOUT_PC7_gc;	// Output clock on Port C, pin 7
}
