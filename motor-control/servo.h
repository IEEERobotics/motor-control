/*
 * servo.h
 *
 *  Created on: Oct 29, 2012
 *      Author: eal
 */

#ifndef SERVO_H_
#define SERVO_H_

typedef enum servo {
	SERVO_A,
	SERVO_B,
	SERVO_C,
	SERVO_D,
	SERVO_INVALID
} servo_t;

int set_angle(servo_t servo, int angle);

#endif /* SERVO_H_ */
