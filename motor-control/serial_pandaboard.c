/*
 * serial_pandaboard.c
 *
 *  Created on: Dec 23, 2012
 *      Author: eal
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "uart.h"
#include "SerialCommands.h"
#include "serial_pandaboard.h"

#define BUFSIZE			32
#define getchar_pb()	uart_getchar(&(pandaboard_uart.f_in))	// get character from pandaboard uart
#define putchar_pb(c)	uart_putchar((c), &(pandaboard_uart.f_out))


/**
 * Read n bytes from stdin
 *
 * @param buffer Buffer to write to
 * @param num_bytes Number of bytes to read in
 */
static inline void getchar_n(uint8_t *buffer, uint8_t num_bytes)
{
	while(num_bytes > 0)
	{
		*buffer = getchar_pb();
		buffer++;
		num_bytes--;
	}
}


static inline void putchar_n(uint8_t *buffer, uint8_t num_bytes)
{
	while(num_bytes > 0)
	{
		putchar_pb(*buffer);
		buffer++;
		num_bytes--;
	}
}


static inline void exec_move_data(move_data *data)
{
	const char *fmt = "Received move_data struct\n"
					  "heading=%d\n"
					  "distance=%d\n\n";

	putchar_pb(MOVE_ACK_ID);
	printf(fmt, data->heading, data->distance);
}


static inline void exec_arm_rotate_data(arm_rotate_data *data)
{
	const char *fmt = "Received arm_rotate_data struct\n"
					  "angle=%d\n\n";

	putchar_pb(ROTATE_ACK_ID);
	printf(fmt, data->angle);
}


static inline void exec_get_sensor_data(get_sensor_data *data)
{
	int i;
	sensor_values sv;
	const char *fmt = "Received get_sensor_data struct. Sending back sensor_values struct.\n"
					  "num=%d\n\n";

	sv.resp = SENSOR_RESP_ID;
	sv.heading = 1800;
	for(i=0; i<USS_NUM; i++)
		sv.USS_arr[i] = 10;
	for(i=0; i<SERVO_NUM; i++)
		sv.servo_arr[i] = 20;

	printf(fmt, data->num);
	putchar_n((uint8_t *)&sv, sizeof(sensor_values));
}


void get_command_pandaboard(void)
{
	uint8_t buffer[BUFSIZE];

	assert(sizeof(move_data) <= BUFSIZE);
	assert(sizeof(arm_rotate_data) <= BUFSIZE);
	assert(sizeof(get_sensor_data) <= BUFSIZE);

	buffer[0] = getchar_pb();	// Command/response ID

	switch(buffer[0])
	{
	case MOVE_CMD_ID:
		getchar_n(buffer+1, sizeof(move_data)-1);
		exec_move_data((move_data *) buffer);
		break;
	case ROTATE_CMD_ID:
		getchar_n(buffer+1, sizeof(arm_rotate_data)-1);
		exec_arm_rotate_data((arm_rotate_data *) buffer);
		break;
	case DATA_CMD_ID:
		getchar_n(buffer+1, sizeof(get_sensor_data)-1);
		exec_get_sensor_data((get_sensor_data *) buffer);
		break;
	default:
		printf("Unrecognized command: %d\n\n", buffer[0]);
		break;
	}
}
