/*
 * timer.h
 *
 *  Created on: Oct 29, 2012
 *      Author: eal
 */

#ifndef TIMER_H_
#define TIMER_H_

#define SERVO_TIMER		TCE0
#define MS_TIMER    	TCC0
#define SERVO_TIMER_PER	20		// Period of SERVO_TIMER in milliseconds
#define MS_TIMER_PER	5		// Period of MS_TIMER in milliseconds

void init_ms_timer(void);

#endif /* TIMER_H_ */
