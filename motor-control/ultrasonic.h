/*
 * ultrasonic.h
 *
 *  Created on: Dec 12, 2012
 *      Author: eal
 */

#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#include <avr/io.h>

#define ULTRASONIC_TIMER	TCE1
#define ULTRASONIC_CHMUX	EVSYS_CH7MUX
#define ULTRASONIC_EVSEL	TC_EVSEL_CH7_gc

typedef struct ultrasonic {
	PORT_t *port;
	uint8_t trig_bm;
	uint8_t echo_bm;
	EVSYS_CHMUX_t echo_chmux;
} ultrasonic_t;

#endif /* ULTRASONIC_H_ */
