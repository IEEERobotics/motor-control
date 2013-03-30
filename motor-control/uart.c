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
#include "debug.h"
#include "uart.h"

uart_t debug_uart;
//uart_t pandaboard_uart;
uart_t servo_uart;


void init_uart(uart_t *u, USART_t *usart, uint16_t bsel, int8_t bscale)
{
	u->usart = usart;

	// Initialize buffers
	buffer_init(&(u->read_buffer), u->read_buffer_data, UART_BUFFER_SIZE);
	buffer_init(&(u->write_buffer), u->write_buffer_data, UART_BUFFER_SIZE);

	// Set interrupt priority levels
	usart->CTRLA = USART_RXCINTLVL_MED_gc
			   	 | USART_DREINTLVL_OFF_gc;

	// Enable transmit and receive on the UART
	usart->CTRLB = USART_RXEN_bm
			   	 | USART_TXEN_bm;

	// Asynchronous mode, 8 bits, no parity
	usart->CTRLC = USART_CMODE_ASYNCHRONOUS_gc
			   	 | USART_PMODE_DISABLED_gc
			   	 | USART_CHSIZE_8BIT_gc;

	usart->BAUDCTRLA = bsel & 0xff;
	usart->BAUDCTRLB = ((bsel >> 8) & 0x0f) | ((bscale << 4) & 0xf0);

	// Attach the uart_t struct to the file streams. This is so that uart_putchar and uart_getchar
	// know which buffer to use.

	// Attach the uart_getchar and uart_putchar functions to the UART_IN and UART_OUT
	// file streams
	fdev_setup_stream(&(u->f_in), NULL, uart_getchar, _FDEV_SETUP_READ);
	fdev_setup_stream(&(u->f_out), uart_putchar, NULL, _FDEV_SETUP_WRITE);
	fdev_set_udata(&(u->f_in), (void *)u);
	fdev_set_udata(&(u->f_out), (void *)u);
}


//#define NEW_INIT
/**
 * Initializes the UART. stdio.h won't work until this function is called.
 */
void init_uarts()
{
	// Set up port pins
	PORTC.DIRSET = PIN3_bm;		// debug_uart
	PORTC.DIRCLR = PIN2_bm;
	PORTC.DIRSET = PIN7_bm;		// servo_uart
	PORTC.DIRCLR = PIN6_bm;
	PORTE.DIRSET = PIN3_bm;		// pandaboard_uart
	PORTE.DIRCLR = PIN2_bm;

//	init_uart(&debug_uart, &DEBUG_USART, 2094, -7);				// 115200 baud at 32 MHz clock
//	init_uart(&debug_uart, &DEBUG_USART, 2158, -6);				// 57600 baud at 32 MHz clock
//	init_uart(&debug_uart, &DEBUG_USART, 3269, -6);				// 38400 baud at 32 MHz clock
	init_uart(&debug_uart, &DEBUG_USART, 3301, -5);				// 19200 baud at 32 MHz clock
//	init_uart(&debug_uart, &DEBUG_USART, 3317, -4);				// 9600 baud at 32 MHz clock

//	init_uart(&pandaboard_uart, &PANDABOARD_USART, 2094, -7);
	init_uart(&servo_uart, &SERVO_USART, 3329, -2);				// 2400 baud at 32 MHz clock

	// Connect stdin, stdout, and stderr to the UART
	stdin = &(debug_uart.f_in);
	stdout = &(debug_uart.f_out);
	stderr = &(debug_uart.f_out);

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
	uart_t *u = (uart_t *)fdev_get_udata(f);

	while(! buffer_put(&(u->write_buffer), c));

	// Enable Data Register Empty interrupt. This will be disabled once all of the data
	// in write_buffer has been sent.
	u->usart->CTRLA = (u->usart->CTRLA & ~USART_DREINTLVL_gm) | UART_DREINTLVL;

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
	uart_t *u = (uart_t *)fdev_get_udata(f);
	uint8_t c;

	while(! buffer_get(&(u->read_buffer), &c));	// Block while read_buffer is empty

	return (int) c;
}


/**
 * Data Register Empty ISR
 *
 * This ISR is called whenever the UART is ready to transmit the next byte, and it has
 * been enabled in uart_putchar(). This interrupt is disabled as soon as write_buffer
 * becomes empty (or else it would be called continuously).
 */
inline void dre_interrupt_handler(uart_t *u)
{
	DEBUG_ENTER_ISR(DEBUG_ISR_DRE);

	uint8_t c;

	if(buffer_get(&(u->write_buffer), &c))
		u->usart->DATA = c;
	else
		u->usart->CTRLA = (u->usart->CTRLA & ~USART_DREINTLVL_gm) | USART_DREINTLVL_OFF_gc;

	DEBUG_EXIT_ISR(DEBUG_ISR_DRE);
}


/**
 * Receive Complete ISR
 *
 * This ISR is called whenever the UART has received a byte.
 */
inline void rxc_interrupt_handler(uart_t *u)
{
	DEBUG_ENTER_ISR(DEBUG_ISR_RXC);
	buffer_put(&(u->read_buffer), u->usart->DATA);
	DEBUG_EXIT_ISR(DEBUG_ISR_RXC);
}


ISR(DEBUG_USART_DRE_VECT)
{
	dre_interrupt_handler(&debug_uart);
}


ISR(DEBUG_USART_RXC_VECT)
{
	rxc_interrupt_handler(&debug_uart);
}


//ISR(PANDABOARD_USART_DRE_VECT)
//{
//	dre_interrupt_handler(&pandaboard_uart);
//}


//ISR(PANDABOARD_USART_RXC_VECT)
//{
//	rxc_interrupt_handler(&pandaboard_uart);
//}

ISR(SERVO_USART_DRE_VECT)
{
	dre_interrupt_handler(&servo_uart);
}

ISR(SERVO_USART_RXC_VECT)
{
	rxc_interrupt_handler(&servo_uart);
}
