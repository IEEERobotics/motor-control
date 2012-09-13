/*
 * clock.c
 *
 *  Created on: Sep 13, 2012
 *      Author: eal
 */

#include <avr/io.h>
#include "clksys_driver.h"
#include "clock.h"

void init_clock(void)
{
	CLKSYS_Enable(OSC_RC32MEN_bm);
	while(CLKSYS_IsReady(OSC_RC32MRDY_bm) == 0);
	CLKSYS_Main_ClockSource_Select(CLK_SCLKSEL_RC32M_gc);
	CLKSYS_Disable(OSC_RC32MEN_bm | OSC_RC32KEN_bm);

	PORTC.DIRSET |= PIN7_bm;
	PORTCFG.CLKEVOUT = PORTCFG_CLKOUT_PC7_gc;			// Output clock on port C pin 7
}
