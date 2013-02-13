/*
 * ultrasonic.c
 *
 *  Created on: Dec 12, 2012
 *      Author: eal
 */

#include <avr/io.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <stdbool.h>
#include "debug.h"
#include "ultrasonic.h"

#define CURRENT_SENSOR			(usensors[current_sensor])
#define NEXT_SENSOR_INDEX()		((current_sensor+1) % ULTRASONIC_NUM_SENSORS)

volatile ultrasonic_t usensors[ULTRASONIC_NUM_SENSORS];
volatile unsigned char current_sensor = ULTRASONIC_LEFT;
volatile bool measurement_in_progress = false;


/**
 * Initializes an ultrasonic_t struct
 *
 * @param u Struct to initialize
 * @param port Port that trig and echo pins are connected to
 * @param trig_bm "Trig" pin bitmask
 * @param echo_bm "Echo" pin bitmask
 * @param echo_chmux EVSYS_CHMUX_t corresponding to the echo pin
 */
static inline void init_ultrasonic_struct(volatile ultrasonic_t *u,
					 	 				  PORT_t *port,
					 	 				  uint8_t trig_bm,
					 	 				  uint8_t echo_bm,
					 	 				  EVSYS_CHMUX_t echo_chmux)
{
	port->OUTSET = trig_bm;		// Set trigger high (falling edge triggered)
	port->DIRSET = trig_bm;		// Set trigger as output
	port->DIRCLR = echo_bm;		// Set echo as input

	u->port = port;
	u->trig_bm = trig_bm;
	u->echo_bm = echo_bm;
	u->echo_chmux = echo_chmux;
	u->distance = -1;
}


/**
 * Trigger an ultrasonic sensor measurement
 */
static inline void ping_ultrasonic(void)
{
	volatile ultrasonic_t *u = &CURRENT_SENSOR;

	measurement_in_progress = true;
	ULTRASONIC_CHMUX = u->echo_chmux;			// Connect event channel 4 to echo pin
	u->port->OUTCLR = u->trig_bm;				// Falling edge triggers sensor measurement
}


/**
 * Set the result of a sensor measurement, and reset for the next run
 */
static inline void set_result(int result)
{
	volatile ultrasonic_t *u = &CURRENT_SENSOR;

	u->distance = result;
	u->port->OUTSET = usensors[current_sensor].trig_bm;
	current_sensor = NEXT_SENSOR_INDEX();
	measurement_in_progress = false;
}


/**
 * Top-level function to initialize the ultrasonic sensors
 */
void init_ultrasonic()
{
	init_ultrasonic_struct(&usensors[ULTRASONIC_LEFT],
						   &ULTRASONIC_LEFT_PORT,
						   ULTRASONIC_LEFT_TRIG,
						   ULTRASONIC_LEFT_ECHO,
						   ULTRASONIC_LEFT_CHMUX);
	init_ultrasonic_struct(&usensors[ULTRASONIC_FRONT],
						   &ULTRASONIC_FRONT_PORT,
						   ULTRASONIC_FRONT_TRIG,
						   ULTRASONIC_FRONT_ECHO,
						   ULTRASONIC_FRONT_CHMUX);
	init_ultrasonic_struct(&usensors[ULTRASONIC_BOTTOM],
						   &ULTRASONIC_BOTTOM_PORT,
						   ULTRASONIC_BOTTOM_TRIG,
						   ULTRASONIC_BOTTOM_ECHO,
						   ULTRASONIC_BOTTOM_CHMUX);
	init_ultrasonic_struct(&usensors[ULTRASONIC_RIGHT],
						   &ULTRASONIC_RIGHT_PORT,
						   ULTRASONIC_RIGHT_TRIG,
						   ULTRASONIC_RIGHT_ECHO,
						   ULTRASONIC_RIGHT_CHMUX);
	init_ultrasonic_struct(&usensors[ULTRASONIC_BACK],
						   &ULTRASONIC_BACK_PORT,
						   ULTRASONIC_BACK_TRIG,
						   ULTRASONIC_BACK_ECHO,
						   ULTRASONIC_BACK_CHMUX);

	/* Initialize ultrasonic timer */
	ULTRASONIC_TIMER.CTRLB = TC0_CCAEN_bm | TC_WGMODE_NORMAL_gc;
	ULTRASONIC_TIMER.CTRLD = TC_EVACT_PW_gc | ULTRASONIC_EVSEL;	// Connect event channel 4 to timer
	ULTRASONIC_TIMER.INTCTRLA = TC_OVFINTLVL_MED_gc;
	ULTRASONIC_TIMER.INTCTRLB = TC_CCAINTLVL_MED_gc;
	ULTRASONIC_TIMER.PER = ULTRASONIC_TIMEOUT;
	ULTRASONIC_TIMER.CTRLA = TC_CLKSEL_DIV64_gc;

	/* Enable medium priority interrupts */
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;
	sei();

	/* Trigger first sensor measurement */
	ping_ultrasonic();
}


/**
 * Returns the last measured distance of an ultrasonic sensor.
 *
 * @param u Pointer to ultrasonic_t struct
 * @return Distance (units still undecided), or -1 if no valid distance has been measured.
 */
int get_ultrasonic_distance(ultrasonic_id_t index)
{
	int distance;

	/* This ensures that the memory access is atomic, i.e. not interrupted, as it takes multiple
	 * clock cycles to complete.
	 */
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		distance = usensors[index].distance;
	}

	return distance;
}


/**
 * This interrupt is triggered when the duration of the echo pulse has been measured. The timer is
 * then reconfigured to generate a delay before the next measurement, to allow the echo to dissipate.
 */
ISR(ULTRASONIC_TIMER_VECT)
{
	DEBUG_ENTER_ISR(DEBUG_ISR_US_TIMER);
	set_result(ULTRASONIC_TIMER.CCA);
	DEBUG_EXIT_ISR(DEBUG_ISR_US_TIMER);
}


/**
 * This interrupt triggers the next sensor measurement.
 */
ISR(ULTRASONIC_TIMER_OVF_VECT)
{
	DEBUG_ENTER_ISR(DEBUG_ISR_US_TIMER_OVF);

	if(measurement_in_progress)
		set_result(-1);

	ping_ultrasonic();

	DEBUG_EXIT_ISR(DEBUG_ISR_US_TIMER_OVF);
}
