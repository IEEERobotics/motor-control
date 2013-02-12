/**
 * @file
 * @author Ethan LaMaster <ealamast@ncsu.edu>
 * @version 0.1
 *
 * @section Description
 *
 * Does magic.
 *
 * @section Ports
 * Port A (Header J2):
 * Pin 0: Unassigned
 * Pin 1: Unassigned
 * Pin 2: Unassigned
 * Pin 3: Unassigned
 * Pin 4: Unassigned
 * Pin 5: Unassigned
 * Pin 6: Unassigned
 * Pin 7: Unassigned
 *
 * Port C (Header J4):
 * Pin 0: Unassigned
 * Pin 1: Unassigned
 * Pin 2: UART Receive (from CV board)
 * Pin 3: UART Transmit (to CV board)
 * Pin 4: Unassigned
 * Pin 5: Unassigned
 * Pin 6: Unassigned
 * Pin 7: System clock output
 *
 * Port D (Header J3):
 * Pin 0: Motor A0
 * Pin 1: Motor A1
 * Pin 2: Motor B0
 * Pin 3: Motor B1
 * Pin 4: Encoder 0
 * Pin 5: Encoder 2
 * Pin 6: Encoder 1
 * Pin 7: Encoder 3
 *
 * Port F (Header J1):
 * Pin 0: Motor C0
 * Pin 1: Motor D0
 * Pin 2: Motor C1
 * Pin 3: Motor D1
 * Pin 4: Encoder 4
 * Pin 5: Encoder 6
 * Pin 6: Encoder 5
 * Pin 7: Encoder 7
 */

#include <avr/io.h>
#include <stdio.h>
#include "motor.h"
#include "pid.h"
#include "clock.h"
#include "uart.h"
#include "serial_interactive.h"
#include "serial_pandaboard.h"
#include "timer.h"
#include "compass.h"
#include "ultrasonic.h"
#include "i2c.h"
#include "servo_parallax.h"
#include "accelerometer.h"


/**
 * Does magic.
 *
 */
int main()
{
	uint16_t i;
	for(i=0; i<0xffff; i++) __asm__ __volatile("nop");

	init_clock();						// Set up the system clock
	init_motors();						// Set up everything to do with motor control
	init_ms_timer();					// Initialize timer interrupt
	init_ultrasonic();
	init_uarts();						// Set up the UART
	init_i2c();
	init_compass();
	init_accelerometer();
	init_servo_parallax();
	print_banner();						// Print welcome message to the serial port

	for(;;)
	{
		get_command_interactive();	// Get the next serial command and run it
//		get_command_pandaboard();

		//__asm__ __volatile("nop");
	}
}
