/**
 * @file
 * @author Ethan LaMaster <ealamast@ncsu.edu>
 * @version 0.1
 *
 * @section Description
 *
 * Functions to initialize the UART, and to parse commands sent over serial.
 */

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "buffer.h"
#include "uart.h"

#define UART USARTC0
#define UART_DREINTLVL	USART_DREINTLVL_MED_gc	// Data Register Empty interrupt priority
#define USE_DOS_NEWLINES						// Use CRLF line endings
#define BUFFER_SIZE		128						// UART read and write buffer size
//#define UART_ECHO_ON					// Echo received characters back to the terminal


/**
 * Serial read and write buffers
 */
buffer_t read_buffer, write_buffer;
volatile uint8_t read_buffer_data[BUFFER_SIZE], write_buffer_data[BUFFER_SIZE];


/**
 * These are the FILE streams that become stdin, stdout, and stderr.
 */
FILE uart_in, uart_out;


/**
 * Initializes the UART. stdio.h won't work until this function is called.
 */
void init_uart()
{
	// Initialize buffers
	buffer_init(&read_buffer, read_buffer_data, BUFFER_SIZE);
	buffer_init(&write_buffer, write_buffer_data, BUFFER_SIZE);

	// Set up port pins
	PORTC.DIRSET = PIN3_bm;
	PORTC.DIRCLR = PIN2_bm;

	// Set interrupt priority levels
	UART.CTRLA = USART_RXCINTLVL_MED_gc
			   | USART_DREINTLVL_OFF_gc;

	// Enable transmit and receive on the UART
	UART.CTRLB = USART_RXEN_bm
			   | USART_TXEN_bm;

	// Asynchronous mode, 8 bits, no parity
	UART.CTRLC = USART_CMODE_ASYNCHRONOUS_gc
			   | USART_PMODE_DISABLED_gc
			   | USART_CHSIZE_8BIT_gc;

	// BSEL=2094, BSCALE=-7
	// Baudrate is 115200 at 32 MHz system clock
	UART.BAUDCTRLA = 0x2e;
	UART.BAUDCTRLB = 0x98;

	// Attach the uart_getchar and uart_putchar functions to the UART_IN and UART_OUT
	// file streams
	fdev_setup_stream(&uart_in, NULL, uart_getchar, _FDEV_SETUP_READ);
	fdev_setup_stream(&uart_out, uart_putchar, NULL, _FDEV_SETUP_WRITE);

	// Connect stdin, stdout, and stderr to the UART
	stdin = &uart_in;
	stdout = &uart_out;
	stderr = &uart_out;

	// Enable medium-priority interrupts
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;
	sei();
}


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
int uart_putchar(char c, FILE *f)
{
#ifdef USE_DOS_NEWLINES
	if(c == '\n')
		uart_putchar('\r', stdout);
#endif

	buffer_put(&write_buffer, c);

	// Enable Data Register Empty interrupt. This will be disabled once all of the data
	// in write_buffer has been sent.
	UART.CTRLA = (UART.CTRLA & ~USART_DREINTLVL_gm) | UART_DREINTLVL;

	return c;
}


/**
 * Read a character from the UART. This function is connected to the uart_in FILE stream,
 * which is set to stdin.
 *
 * @param f FILE pointer to operate on. This parameter is ignored, but is required by
 * 			stdio.
 *
 * @return Received character, casted to an int.
 */
int uart_getchar(FILE *f)
{
	uint8_t c;

	while(! buffer_get(&read_buffer, &c));	// Block while read_buffer is empty
#ifdef UART_ECHO_ON
	uart_putchar(c);
#endif
	return (int) c;
}


/**
 * Data Register Empty ISR
 *
 * This ISR is called whenever the UART is ready to transmit the next byte, and it has
 * been enabled in uart_putchar(). This interrupt is disabled as soon as write_buffer
 * becomes empty (or else it would be called continuously).
 */
ISR(USARTC0_DRE_vect)
{
	uint8_t c;

	if(buffer_get(&write_buffer, &c))
		UART.DATA = c;
	else
		UART.CTRLA = (UART.CTRLA & ~USART_DREINTLVL_gm) | USART_DREINTLVL_OFF_gc;
}


/**
 * Receive Complete ISR
 *
 * This ISR is called whenever the UART has received a byte.
 */
ISR(USARTC0_RXC_vect)
{
	buffer_put(&read_buffer, UART.DATA);
}
