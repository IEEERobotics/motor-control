/*
 * servo_parallax.h
 *
 *  Created on: Dec 8, 2012
 *      Author: eal
 */


#ifndef SERVO_PARALLAX_H_
#define SERVO_PARALLAX_H_

#define SERVO_LEFT_ARM_CHANNEL		0
#define SERVO_RIGHT_ARM_CHANNEL		2
#define SERVO_LEFT_GRIP_CHANNEL		1
#define SERVO_RIGHT_GRIP_CHANNEL	3

#define SERVO_LEFT_ARM_UP			680
#define SERVO_LEFT_ARM_DOWN			310
#define SERVO_LEFT_GRIP_OPEN		900
#define SERVO_LEFT_GRIP_CLOSE		450

#define SERVO_RIGHT_ARM_UP			330
#define SERVO_RIGHT_ARM_DOWN		710
#define SERVO_RIGHT_GRIP_OPEN		0
#define SERVO_RIGHT_GRIP_CLOSE		350

#define SERVO_ARM_RAMP				10
#define SERVO_GRIP_RAMP				5

#define SERVO_CLOSE_TIME			(100/MS_TIMER_PER)	// 100 ms

void init_servo_parallax();
int parallax_set_angle(int channel, int angle, int ramp);

#endif /* SERVO_PARALLAX_H_ */
