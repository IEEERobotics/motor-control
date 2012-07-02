/*
 * serial-stdio.h
 *
 *  Created on: Jul 2, 2012
 *      Author: eal
 */

#ifndef SERIAL_STDIO_H_
#define SERIAL_STDIO_H_

void init_serial();
void uart_putchar(char c);
char uart_getchar();


#endif /* SERIAL_STDIO_H_ */
