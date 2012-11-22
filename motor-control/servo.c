/**
 * @file
 *
 * Functions for controlling servos
 */


#include <avr/io.h>
#include <stdio.h>
#include "timer.h"
#include "servo.h"


/**
 * Set the angle of a servo
 *
 * @param servo servo_t to operate on
 * @param angle An angle between 0 and 180
 * @return True on success, or false if bad input
 */
int set_angle(servo_t servo, int angle)
{
	double tmp;
	register16_t cc_register;

	if(angle < 0 || angle > 180)
		return 0;

	/* This looks goofy, but is necessary to avoid processor register overflow.
	 * cc_register = (angle*505)/180 + 505 is the most obvious solution, but the
	 * intermediate value (angle*505) has a maximum value of 90900, which overflows
	 * the processor's 16 bit registers.
	 */
	tmp = (double)angle / (double)180;
	cc_register = (tmp * 505) + 505;

	switch(servo)
	{
	case SERVO_A:
		SERVO_TIMER.CCABUF = cc_register;
		break;
	case SERVO_B:
		SERVO_TIMER.CCBBUF = cc_register;
		break;
	case SERVO_C:
		SERVO_TIMER.CCCBUF = cc_register;
		break;
	case SERVO_D:
		SERVO_TIMER.CCDBUF = cc_register;
		break;
	default:
		return 0;
	}

	return 1;
}
