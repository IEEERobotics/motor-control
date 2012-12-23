/*
 * serial_pandaboard.c
 *
 *  Created on: Dec 23, 2012
 *      Author: eal
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "SerialCommands.h"
#include "serial_pandaboard.h"

#define BUFSIZE		32


/**
 * Read n bytes from stdin
 *
 * @param buffer Buffer to write to
 * @param num_bytes Number of bytes to read in
 */
static inline void getchar_n(char *buffer, int num_bytes)
{
	while(num_bytes > 0)
	{
		*buffer = getchar();
		buffer++;
		num_bytes--;
	}
}


static inline void exec_move_data(move_data *data)
{

}


static inline void exec_arm_rotate_data(arm_rotate_data *data)
{

}


static inline void exec_get_sensor_data(get_sensor_data *data)
{

}


void get_command_pandaboard(void)
{
	char buffer[BUFSIZE];

//	assert(sizeof(move_data) <= BUFSIZE);
//	assert(sizeof(arm_rotate_data) <= BUFSIZE);
//	assert(sizeof(get_sensor_data) <= BUFSIZE);

	buffer[0] = getchar();	// Command/response ID

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
		// Bad command
		break;
	}
}
