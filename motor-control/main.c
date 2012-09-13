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
 * @todo Implement me!
 */
int main()
{
	init_clock();
	init_serial();

	PORTE.DIRSET = 0xff;
	PORTE.OUT = 0xaa;

	test_serial_out();

	/* Make sure control never reaches the end of main() */
	for(;;)
	{
		__asm__ __volatile("nop");
	}
}
