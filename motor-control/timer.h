/*
 * timer.h
 *
 *  Created on: Oct 29, 2012
 *      Author: eal
 */

#ifndef TIMER_H_
#define TIMER_H_

#define MS_TIMER    	TCC0
#define MS_TIMER_PER	5		// Period of MS_TIMER in milliseconds

extern volatile uint8_t ms_timer;

void init_pwm_timer(TC0_t *timer);
void init_enc_timer(TC1_t *timer, TC_EVSEL_t event_channel);
void init_ms_timer(void);

#endif /* TIMER_H_ */
