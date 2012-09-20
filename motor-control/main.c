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

motor_t motor_a, motor_b, motor_c, motor_d;


/**
 * Does magic.
 *
 */
int main()
{
	init_clock();		// Set up the system clock
	init_serial();		// Set up the UART

	test_serial_out();	// Print "Hello, world!" in an infinite loop

	/* Make sure control never reaches the end of main() */
	for(;;)
	{
		__asm__ __volatile("nop");
	}
}
