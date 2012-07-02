/*
 * serial-stdio.c
 *
 *  Created on: Jul 1, 2012
 *      Author: eal
 */

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>

#define UART USARTC0
#define USE_DOS_NEWLINES
//#define UART_ECHO_ON

FILE uart_in, uart_out, uart_io;

void init_serial()
{
	fdev_setup_stream(&uart_in, NULL, uart_getchar, _FDEV_SETUP_READ);
	fdev_setup_stream(&uart_out, uart_putchar, NULL, _FDEV_SETUP_WRITE);
	fdev_setup_stream(&uart_io, uart_putchar, uart_getchar, _FDEV_SETUP_RW);
}


void uart_putchar(char c)
{
#ifdef USE_DOS_NEWLINES
	if(c == '\n')
		uart_putchar('\r');
#endif

	while(USARTC0.STATUS & USART_DREIF_bm);
	UART.DATA = c;
}


char uart_getchar()
{
#ifdef UART_ECHO_ON
	char c;
#endif
	while(USARTC0.STATUS & USART_RXCIF_bm);
#ifdef UART_ECHO_ON
	c = UART.DATA;
	uart_putchar(c);
	return c;
#else
	return UART.DATA;
#endif
}
