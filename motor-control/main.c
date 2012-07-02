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

#define UNIT_TEST

motor_t motor_a, motor_b, motor_c, motor_d;


/**
 * Does magic.
 *
 * @todo Implement me!
 */
#ifndef UNIT_TEST
int main()
{
	// Just a stub

	/* Make sure control never reaches the end of main() */
	for(;;)
	{
		__asm__ __volatile("nop");
	}
}

#else
/* This main() is used to execute unit tests */
int main(int argc, char **argv)
{

	return 0;
}
