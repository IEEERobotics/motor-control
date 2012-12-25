/*
 * servo_parallax.h
 *
 *  Created on: Dec 8, 2012
 *      Author: eal
 */

#ifndef SERVO_PARALLAX_H_
#define SERVO_PARALLAX_H_

void init_servo_parallax();
int parallax_set_angle(int channel, int angle, int ramp);

#endif /* SERVO_PARALLAX_H_ */
