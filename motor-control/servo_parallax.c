/*
 * servo_parallax.c
 *
 *  Created on: Dec 8, 2012
 *      Author: eal
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "uart.h"
#include "servo_parallax.h"


static inline void servo_putchar(uint8_t c)
{
	uart_putchar(c, &(servo_uart.f_out));
}


static inline void servo_putchar_n(uint8_t *buffer, uint8_t num_bytes)
{
	while(num_bytes > 0)
	{
		servo_putchar(*buffer);
		buffer++;
		num_bytes--;
	}
}


/**
 * Initialize the Parallax servo controller.
 */
void init_servo_parallax()
{
	parallax_set_angle(SERVO_LEFT_ARM_CHANNEL, SERVO_LEFT_ARM_UP, SERVO_RAMP);
	parallax_set_angle(SERVO_RIGHT_ARM_CHANNEL, SERVO_RIGHT_ARM_UP, SERVO_RAMP);
}


/**
 * Set the angle of a servo.
 *
 * @param channel Servo channel (0-15)
 * @param angle Angle (between 0 and 1000)
 * @param ramp Rate at which to ramp the servo (between 0 and 63). 63 corresponds to
 * 			   maximum ramping, and 0 disables this feature. This is "roughly linear,
 * 			   but no equation exists to describe it" (see the Parallax servo controller
 * 			   datasheet for details).
 * 	@return True if successful, or false if bad input
 */
int parallax_set_angle(int channel, int angle, int ramp)
{
	if(angle < 0 || angle > 1000)
		return 0;
	if(channel < 0 || channel > 15)
		return 0;
	if(ramp < 0 || ramp > 63)
		return 0;

	angle += 250;
	uint8_t data[] = {'!',
					  'S',
					  'C',
					  (char) channel,
					  (char) ramp,
					  (char) angle & 0xff,
					  (char) (angle >> 8) & 0xff,
					  '\r',
	};

	servo_putchar_n(data, sizeof(data));

	return 1;
}
