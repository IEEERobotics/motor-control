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

motor_t motor_a, motor_b, motor_c, motor_d;

/**
 * Does magic.
 *
 * @todo Implement me!
 */
int main()
{
	// Just a stub

	/* Make sure control never reaches the end of main() */
	for(;;)
	{
		__asm__ __volatile("nop");
	}
}
