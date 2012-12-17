/*
 * ultrasonic.c
 *
 *  Created on: Dec 12, 2012
 *      Author: eal
 */

#include "ultrasonic.h"


/**
 * Initializes an ultrasonic_t struct
 *
 * @param u Struct to initialize
 * @param port Port that trig and echo pins are connected to
 * @param trig_bm "Trig" pin bitmask
 * @param echo_bm "Echo" pin bitmask
 * @param echo_chmux EVSYS_CHMUX_t corresponding to the echo pin
 */
void init_ultrasonic(ultrasonic_t *u,
					 PORT_t *port,
					 uint8_t trig_bm,
					 uint8_t echo_bm,
					 EVSYS_CHMUX_t echo_chmux)
{
	u->port = port;
	u->trig_bm = trig_bm;
	u->echo_bm = echo_bm;
	u->echo_chmux = echo_chmux;
}


/**
 * Trigger an ultrasonic sensor measurement
 *
 * @param u Pointer to ultrasonic_t struct
 * @return Distance in millimeters
 */
int ping_ultrasonic(ultrasonic_t *u)
{
	int time;

	u->port->OUTCLR = u->trig_bm;
	u->port->DIRSET = u->trig_bm;
	u->port->DIRCLR = u->echo_bm;

	ULTRASONIC_CHMUX = u->echo_chmux;
	ULTRASONIC_TIMER.CTRLA = TC_CLKSEL_DIV1_gc;
	ULTRASONIC_TIMER.CTRLB = TC1_CCAEN_bm;
	ULTRASONIC_TIMER.CTRLD = TC_EVACT_PW_gc | ULTRASONIC_EVSEL;

	u->port->OUTSET = u->trig_bm;
	while(ULTRASONIC_TIMER.INTFLAGS & TC1_CCAIF_bm);
	u->port->OUTCLR = u->trig_bm;

	time = ULTRASONIC_TIMER.CCA;
	return time;
}
