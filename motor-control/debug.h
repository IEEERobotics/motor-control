/**
 * @file
 * @author Ethan LaMaster <ealamast@ncsu.edu>
 *
 * Macros to assist in debugging. Ports H and J are used to output debug info.
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <avr/io.h>

#define DEBUG_STATUS_PORT								PORTH
#define DEBUG_ISR_PORT									PORTJ

#define DEBUG_UNKNOWN									0b00000000
#define DEBUG_INIT_ACCELEROMETER						0b00000001
#define DEBUG_INIT_COMPASS								0b00000010
#define DEBUG_COMPUTE_PID								0b00000011

#define DEBUG_ISR_MSTIMER								PIN0_bm
#define DEBUG_ISR_I2C									PIN1_bm
#define DEBUG_ISR_DRE									PIN2_bm
#define DEBUG_ISR_RXC									PIN3_bm
#define DEBUG_ISR_US_TIMER								PIN4_bm
#define DEBUG_ISR_US_TIMER_OVF							PIN5_bm
#define DEBUG_ISR_ENCODER								PIN6_bm

#define DEBUG_ENTER_ISR(mask)							( DEBUG_ISR_PORT.OUTSET = mask )
#define DEBUG_EXIT_ISR(mask)							( DEBUG_ISR_PORT.OUTCLR = mask )
#define DEBUG_STATUS(code)								( DEBUG_STATUS_PORT.OUT = code )
#define DEBUG_CLEAR_STATUS()							( DEBUG_STATUS_PORT.OUT = DEBUG_UNKNOWN )


inline void init_debug(void)
{
	DEBUG_STATUS_PORT.OUT = DEBUG_UNKNOWN;
	DEBUG_STATUS_PORT.DIR = 0xff;
	DEBUG_ISR_PORT.OUT = 0x00;
	DEBUG_ISR_PORT.DIR = 0xff;
}


#endif /* DEBUG_H_ */
