/**
 * @file
 * @author Ethan LaMaster <ealamast@ncsu.edu>
 * @version 0.1
 *
 * @section Description
 *
 * Does magic.
 *
 */

#include <avr/io.h>
#include "motor.h"
#include "pid.h"
#include "clock.h"
#include "serial.h"


/**
 * Does magic.
 *
 */
int main()
{
	init_clock();		// Set up the system clock
	init_serial();		// Set up the UART

	print_banner();		// Print welcome message to the serial port

	for(;;)
	{
		get_command();	// Get the next serial command and run it

		//__asm__ __volatile("nop");
	}
}
