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


void init_uart();
int uart_putchar(char c, FILE *f);
int uart_getchar(FILE *f);


#endif /* SERIAL_STDIO_H_ */
