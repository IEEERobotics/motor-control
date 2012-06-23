/*
 * serial.c
 *
 *  Created on: Jun 15, 2012
 *      Author: eal
 */

#include <stdlib.h>
#include <string.h>
#include "serial.h"
#include "motor.h"

#define TOKEN_A			0
#define TOKEN_B			1
#define TOKEN_C			2
#define TOKEN_D			3
#define TOKEN_HELP		4
#define TOKEN_RUN		5
#define TOKEN_STATUS	6

#define TOKENS_LEN		7

extern motor_t motor_a, motor_b, motor_c, motor_d;

#ifdef USE_UNIX_NEWLINES
static const char newline[] = {0x0a, '\0'};
#else
static const char newline[] = {0x0d, 0x0a, '\0'};
#endif

/* Serial Commands */
static const char *prompt = "% ";
static const char *delimiters = " ";
static const char *error_bad_command = "Bad command.";
static const char *error_unimplemented = "Unimplemented.";

static const char *tokens[] = {		// In strcmp() sorted order
		"A",
		"B",
		"C",
		"D",
		"help",
		"run",
		"status",
};

static volatile buffer_t read_buffer;
static volatile buffer_t write_buffer;

static int comptoken(const void *a, const void *b);


static inline int init_buffer(volatile buffer_t *buffer, int size)
{
	buffer->head = 0;
	buffer->tail = 0;
	buffer->size = size;
	buffer->bfr = (char *) malloc(size * sizeof(char));

	if(buffer->bfr)
		return 1;
	else
		return 0;	// malloc failed
}


static inline void free_buffer(volatile buffer_t *buffer)
{
	free(buffer->bfr);
}


/**
 * Adds a character to a buffer.
 *
 * @param buffer Pointer to a buffer_t struct
 * @param input Character to write to the buffer.
 *
 * @todo Enable transmit interrupt at the end of this function
 */
static inline int bfr_putchar(volatile buffer_t *buffer, char input)
{
	// If the buffer is full, block until a byte is written from the serial interface
	while( ((buffer->tail+1) % buffer->size) == (buffer->head % buffer->size) );

	buffer->bfr[buffer->tail] = input;
	buffer->tail++;

	return 1;
}


/**
 * Gets a character from a buffer.
 *
 * @param buffer Pointer to a buffer_t struct
 * @param output Pointer to write the character to
 *
 * @return False if buffer is empty, otherwise true.
 *
 * @todo Unit test
 */
static inline int bfr_getchar(volatile buffer_t *buffer, char *output)
{
	if( (buffer->tail % buffer->size) == (buffer->head % buffer->size) )
	{
		return 0;	// buffer empty
	}

	*output = buffer->bfr[buffer->head];
	buffer->head++;

	return 1;
}


static inline int findtoken(char *token, const char **tokens, int tokens_len)
{
	char **match;

	if(token == NULL)
		return -1;

	//qsort(tokens, tokens_len, sizeof(char *), comptoken);
	match = (char **) bsearch(token, tokens, tokens_len, sizeof(char *), comptoken);

	if(match == NULL)
		return -1;
	else
		return (int)match - (int)tokens;	// Return index
}


void init_serial()
{
	init_buffer(&read_buffer, BUFSIZE);
	init_buffer(&write_buffer, BUFSIZE);
}


/**
 * Reads a line from a buffer.
 *
 * This function treats CR and/or LF as newline delimiters. If the terminal is
 * sending DOS line endings (ie. CRLF), this is treated as two newlines. This has
 * no effect on parsing.
 *
 * @param buffer Pointer to a buffer_t struct
 * @param line Pointer to a buffer to store the line
 * @param size Size of the line buffer
 *
 * @return Number of characters written to line (excluding null character).
 * 		   -1 if no newline is found
 * 		   -2 if line buffer is too small
 *
 */
int bfr_readln(volatile buffer_t *buffer, char *line, int size)
{
	unsigned char old_head = buffer->head;
	int line_index = 0;
	char c;

	while(bfr_getchar(buffer, &c))
	{
		if(line_index < size)
		{
			if(c == 0x0a || c == 0x0d)	// is newline
			{
				line[line_index] = '\0';
				return line_index;
			}

			line[line_index] = c;
			line_index++;
		}
		else
		{
			return -2;		// Line buffer overflow
		}
	}

	/* No newline in serial buffer */
	buffer->head = old_head;
	return -1;
}


/**
 * Writes a line to a buffer.
 *
 * @param buffer Pointer to a buffer_t struct
 * @param line Pointer to the string to write
 * @param print_prompt If true, print a command prompt ("% ") on the next
 * 		  line.
 *
 */
void bfr_putln(volatile buffer_t *buffer, const char *line, int print_prompt)
{
	int i;

	/* write line */
	for(i=0; line[i]; i++)
	{
		bfr_putchar(buffer, line[i]);
	}

	/* write newline sequence */
	for(i=0; newline[i]; i++)
	{
		bfr_putchar(buffer, newline[i]);
	}

	if(print_prompt)
	{
		for(i=0; prompt[i]; i++)
		{
			bfr_putchar(buffer, prompt[i]);
		}
	}
}


static int comptoken(const void *a, const void *b)
{
	return strcmp((char *) a, (char *) b);
}


void parse_line(command_t *c)
{
	char line[BUFSIZE];
	char *token0, *token1, *token2;

	switch(bfr_readln(&read_buffer, line, BUFSIZE))
	{
	case -1:	// No newline found
		return;

	case -2:	// line buffer too small
		// Consider adding LED debug code
		return;
	default:
		break;
	}

	token0 = strtok(line, delimiters);
	token1 = strtok(NULL, delimiters);
	token2 = strtok(NULL, delimiters);

	c->token0 = findtoken(token0, tokens, TOKENS_LEN);
	c->token1 = findtoken(token1, tokens, TOKENS_LEN);
	c->setpoint = atoi(token2);
}


void run_command(command_t *c)
{
	motor_t *motor;
	direction_t dir;

	switch(c->token1)
	{
	case TOKEN_A:
		motor = &motor_a;
		break;
	case TOKEN_B:
		motor = &motor_b;
		break;
	case TOKEN_C:
		motor = &motor_c;
		break;
	case TOKEN_D:
		motor = &motor_d;
		break;
	default:
		motor = NULL;
		break;
	}

	switch(c->token0)
	{
	case TOKEN_RUN:
		if(motor)
		{
			if(c->setpoint < 0)
				dir = DIR_REVERSE;
			else if(c->setpoint > 0)
				dir = DIR_FORWARD;
			else
				dir = DIR_BRAKE;

			change_direction(motor, dir);
			change_setpoint(motor, c->setpoint);
		}
		else	// motor == NULL
		{
			bfr_putln(&write_buffer, error_bad_command, 1);
		}

		break;

	case TOKEN_STATUS:
		bfr_putln(&write_buffer, error_unimplemented, 1);
		break;

	case TOKEN_HELP:
		bfr_putln(&write_buffer, error_unimplemented, 1);
		break;

	default:
		bfr_putln(&write_buffer, error_bad_command, 1);
		break;
	}
}
