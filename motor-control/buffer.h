/**
 * @file
 * @author Ethan LaMaster <ealamast@ncsu.edu>
 * @version 0.1
 *
 * A simple implementation of a circular buffer, intended for interrupt-driven serial I/O.
 *
 * The underlying array must be supplied externally, to avoid having to link malloc()
 * in with our code.
 */

#ifndef BUFFER_H_
#define BUFFER_H_


/**
 * Struct defining a single buffer.
 */
typedef struct buffer {
	volatile uint8_t *data;
	volatile uint8_t head;
	volatile uint8_t tail;
	uint8_t size;
} buffer_t;


/**
 * Initializes a buffer_t struct.
 *
 * @param buffer Pointer to the buffer_t to initialize
 * @param data Pointer to the external array to use
 * @param size Length of the data array
 */
inline void buffer_init(buffer_t *buffer, volatile uint8_t *data, uint8_t size)
{
	buffer->data = data;
	buffer->size = size;
	buffer->head = 0;
	buffer->tail = 0;
}


/**
 * Tests whether a buffer is full
 *
 * @param buffer Pointer to a buffer_t struct
 * @return True if buffer is full, otherwise false
 */
inline int buffer_full(buffer_t *buffer)
{
	return ((buffer->tail % buffer->size) == ((buffer->head-1) % buffer->size));
}


/**
 * Tests whether a buffer is empty
 *
 * @param buffer Pointer to a buffer_t struct
 * @return True if buffer is empty, otherwise false
 */
inline int buffer_empty(buffer_t *buffer)
{
	return (buffer->tail == buffer->head);
}


/**
 * Adds a byte to the buffer
 *
 * @param buffer Pointer to a buffer_t struct
 * @param byte Byte to add to the buffer
 * @return True if the byte was written to the buffer, or false if the buffer is full
 */
inline int buffer_put(buffer_t *buffer, uint8_t byte)
{
	if(! buffer_full(buffer))
	{
		buffer->data[buffer->tail] = byte;
		buffer->tail = (buffer->tail+1) % (buffer->size);
		return 1;
	}
	else
	{
		return 0;
	}
}


/**
 * Gets a byte from the buffer
 *
 * @param buffer Pointer to a buffer_t struct
 * @param byte Pointer in which to store the read byte
 * @return True if a byte was read, or false if the buffer is empty. The memory pointed
 * 		   to by 'byte' will not be modified in the event that the buffer is empty.
 */
inline int buffer_get(buffer_t *buffer, uint8_t *byte)
{
	if(! buffer_empty(buffer))
	{
		*byte = buffer->data[buffer->head];
		buffer->head = (buffer->head+1) % (buffer->size);
		return 1;
	}
	else
	{
		return 0;
	}
}


#endif /* BUFFER_H_ */
