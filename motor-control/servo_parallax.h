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
#define SERVO_LEFT_GRIPPER_CHANNEL	1
#define SERVO_RIGHT_GRIPPER_CHANNEL	3

#define SERVO_LEFT_ARM_UP			670
#define SERVO_RIGHT_ARM_UP			340
#define SERVO_RAMP					10

void init_servo_parallax();
int parallax_set_angle(int channel, int angle, int ramp);

#endif /* SERVO_PARALLAX_H_ */
