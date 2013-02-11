/**
 * @file
 * @author Ethan LaMaster <ealamast@ncsu.edu>
 * @version 0.1
 *
 * @section Description
 *
 * Functions to initialize the UART, and to parse commands sent over serial.
 */

#ifndef SERIAL_STDIO_H_
#define SERIAL_STDIO_H_

#include <stdio.h>
#include "buffer.h"

#define UART_BUFFER_SIZE	255					// UART read and write buffer size
#define UART_DREINTLVL	USART_DREINTLVL_MED_gc	// Data Register Empty interrupt priority

#define DEBUG_USART					USARTC0
#define DEBUG_USART_DRE_VECT		USARTC0_DRE_vect
#define DEBUG_USART_RXC_VECT		USARTC0_RXC_vect

#define PANDABOARD_USART			USARTE0
#define PANDABOARD_USART_DRE_VECT	USARTE0_DRE_vect
#define PANDABOARD_USART_RXC_VECT	USARTE0_RXC_vect

#define SERVO_USART					USARTC1
#define SERVO_USART_DRE_VECT		USARTC1_DRE_vect
#define SERVO_USART_RXC_VECT		USARTC1_RXC_vect

typedef struct uart {
	USART_t *usart;
	FILE f_in, f_out;
	buffer_t read_buffer, write_buffer;
	volatile uint8_t read_buffer_data[UART_BUFFER_SIZE], write_buffer_data[UART_BUFFER_SIZE];
} uart_t;

void init_uart(uart_t *u, USART_t *usart, uint16_t bsel, int8_t bscale);
void init_uarts();
int uart_putchar(char c, FILE *f);
int uart_getchar(FILE *f);

extern uart_t debug_uart, pandaboard_uart, servo_uart;

#endif /* SERIAL_STDIO_H_ */
