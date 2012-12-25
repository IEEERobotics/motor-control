/*
 * servo_parallax.c
 *
 *  Created on: Dec 8, 2012
 *      Author: eal
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "buffer.h"
#include "servo_parallax.h"

#define UART 			USARTC1
#define UART_DRE_VECT	USARTC1_DRE_vect
#define TX_PIN			PIN7_bm
#define RX_PIN			PIN6_bm
#define UART_DREINTLVL	USART_DREINTLVL_MED_gc	// Data Register Empty interrupt priority
#define BUFFER_SIZE		128

buffer_t write_buffer;
volatile uint8_t write_buffer_data[BUFFER_SIZE];
FILE uart_out;


/**
 * Write a character to the UART. This function is connected to the uart_out FILE stream,
 * which is set to stdout and stderr.
 *
 * @param c Character to transmit
 * @param f FILE pointer to operate on. This parameter is ignored, but is required by
 * 			stdio.
 *
 * @return Character that was transmitted, casted to an int.
 */
int servo_uart_putchar(char c, FILE *f)
{
	buffer_put(&write_buffer, c);

	// Enable Data Register Empty interrupt. This will be disabled once all of the data
	// in write_buffer has been sent.
	UART.CTRLA = (UART.CTRLA & ~USART_DREINTLVL_gm) | UART_DREINTLVL;

	return c;
}


/**
 * Initialize the Parallax servo controller.
 */
void init_servo_parallax()
{
	// Initialize buffer
	buffer_init(&write_buffer, write_buffer_data, BUFFER_SIZE);

	// Set up port pins
	PORTC.DIRSET = TX_PIN;
	PORTC.DIRCLR = RX_PIN;

	// Set interrupt priority levels
	UART.CTRLA = USART_DREINTLVL_OFF_gc;

	// Enable transmit on the UART
	UART.CTRLB = USART_TXEN_bm;

	// Asynchronous mode, 8 bits, no parity, 2 stop bits
	UART.CTRLC = USART_CMODE_ASYNCHRONOUS_gc
			   | USART_PMODE_DISABLED_gc
			   | USART_SBMODE_bm
			   | USART_CHSIZE_8BIT_gc;

	// BSEL=3329, BSCALE=-2
	// Baudrate is 2400 at 32 MHz system clock
	UART.BAUDCTRLA = 0x01;
	UART.BAUDCTRLB = 0xed;

	// Attach the uart_getchar and uart_putchar functions to the UART_IN and UART_OUT
	// file streams
	fdev_setup_stream(&uart_out, servo_uart_putchar, NULL, _FDEV_SETUP_WRITE);

	// Enable medium-priority interrupts
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;
	sei();
}


/**
 * Set the angle of a servo.
 *
 * @param channel Servo channel (0-15)
 * @param angle Angle (between 0 and 1000)
 * @param ramp Rate at which to ramp the servo (between 0 and 63). 63 corresponds to
 * 			   maximum ramping, and 0 disables this feature. This is "roughly linear,
 * 			   but no equation exists to describe it" (see the Parallax servo controller
 * 			   datasheet for details).
 * 	@return True if successful, or false if bad input
 */
int parallax_set_angle(int channel, int angle, int ramp)
{
	if(angle < 0 || angle > 1000)
		return 0;
	if(channel < 0 || channel > 15)
		return 0;
	if(ramp < 0 || ramp > 63)
		return 0;

	angle += 250;
	const char data[] = {'!',
					  	 'S',
					  	 'C',
					  	 (char) channel,
					  	 (char) ramp,
					  	 (char) angle | 0xff,
					  	 (char) (angle >> 8) | 0xff,
					  	 '\r'
	};

	fprintf(&uart_out, data);

	return 1;
}


/**
 * Data Register Empty ISR
 *
 * This ISR is called whenever the UART is ready to transmit the next byte, and it has
 * been enabled in uart_putchar(). This interrupt is disabled as soon as write_buffer
 * becomes empty (or else it would be called continuously).
 */
ISR(UART_DRE_VECT)
{
	uint8_t c;

	if(buffer_get(&write_buffer, &c))
		UART.DATA = c;
	else
		UART.CTRLA = (UART.CTRLA & ~USART_DREINTLVL_gm) | USART_DREINTLVL_OFF_gc;
}
