/*
 * ultrasonic.h
 *
 *  Created on: Dec 12, 2012
 *      Author: eal
 */

#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#include <avr/io.h>
#include "clock.h"

#define ULTRASONIC_TIMER			TCE0
#define ULTRASONIC_TIMER_VECT		TCE0_CCA_vect
#define ULTRASONIC_TIMER_OVF_VECT	TCE0_OVF_vect
#define ULTRASONIC_TIMER_OVF_PER	5000						// 10 ms
#define ULTRASONIC_CHMUX			EVSYS_CH4MUX
#define ULTRASONIC_EVSEL			TC_EVSEL_CH4_gc
#define ULTRASONIC_NUM_SENSORS		5
#define ULTRASONIC_TIMEOUT			19000						// 38 ms

//#define ULTRASONIC_LEFT_INDEX		0
#define ULTRASONIC_LEFT_PORT		PORTA
#define ULTRASONIC_LEFT_TRIG		PIN0_bm
#define ULTRASONIC_LEFT_ECHO		PIN1_bm
#define ULTRASONIC_LEFT_CHMUX		EVSYS_CHMUX_PORTA_PIN1_gc

//#define ULTRASONIC_FRONT_INDEX		1
#define ULTRASONIC_FRONT_PORT		PORTA
#define ULTRASONIC_FRONT_TRIG		PIN2_bm
#define ULTRASONIC_FRONT_ECHO		PIN3_bm
#define ULTRASONIC_FRONT_CHMUX		EVSYS_CHMUX_PORTA_PIN3_gc

//#define ULTRASONIC_BOTTOM_INDEX		2
#define ULTRASONIC_BOTTOM_PORT		PORTC	// !!!
#define ULTRASONIC_BOTTOM_TRIG		PIN4_bm
#define ULTRASONIC_BOTTOM_ECHO		PIN5_bm
#define ULTRASONIC_BOTTOM_CHMUX		EVSYS_CHMUX_PORTC_PIN5_gc

//#define ULTRASONIC_RIGHT_INDEX		3
#define ULTRASONIC_RIGHT_PORT		PORTA
#define ULTRASONIC_RIGHT_TRIG		PIN4_bm
#define ULTRASONIC_RIGHT_ECHO		PIN5_bm
#define ULTRASONIC_RIGHT_CHMUX		EVSYS_CHMUX_PORTA_PIN5_gc

//#define ULTRASONIC_BACK_INDEX		4
#define ULTRASONIC_BACK_PORT		PORTA
#define ULTRASONIC_BACK_TRIG		PIN6_bm
#define ULTRASONIC_BACK_ECHO		PIN7_bm
#define ULTRASONIC_BACK_CHMUX		EVSYS_CHMUX_PORTA_PIN7_gc


typedef struct ultrasonic {
	PORT_t *port;
	uint8_t trig_bm;
	uint8_t echo_bm;
	EVSYS_CHMUX_t echo_chmux;
	int distance;
} ultrasonic_t;

/* Constants correspond to indices in the usensors array */
typedef enum ultrasonic_id {
	ULTRASONIC_LEFT 	= 0,
	ULTRASONIC_FRONT 	= 1,
	ULTRASONIC_BOTTOM 	= 2,
	ULTRASONIC_RIGHT 	= 3,
	ULTRASONIC_BACK 	= 4
} ultrasonic_id_t;

void init_ultrasonic();
void init_ultrasonic_struct(volatile ultrasonic_t *u,
					 	 	PORT_t *port,
					 	 	uint8_t trig_bm,
					 	 	uint8_t echo_bm,
					 	 	EVSYS_CHMUX_t echo_chmux);
void ping_ultrasonic(volatile ultrasonic_t *u);
int get_ultrasonic_distance(ultrasonic_id_t index);


#endif /* ULTRASONIC_H_ */
