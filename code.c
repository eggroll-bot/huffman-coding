#include "code.h"

#include "defines.h"

#include <stdint.h>
#include <stdio.h>

// Description:
// Checks the size of a code.
//
// Parameters:
// Code *c - The code to check.
//
// Returns:
// uint32_t - The size of the code.
uint32_t code_size( Code *c ) {
	return c->top;
}

// Description:
// Checks if a code is empty.
//
// Parameters:
// Code *c - The code to check.
//
// Returns:
// bool - Whether the code is empty.
bool code_empty( Code *c ) {
	return c->top == 0;
}

// Description:
// Checks if a code is full.
//
// Parameters:
// Code *c - The code to check.
//
// Returns:
// bool - Whether the code is full.
bool code_full( Code *c ) {
	return c->top == MAX_CODE_SIZE * 8;
}

// Description:
// Returns a bit from a code.
//
// Parameters:
// Code *c - The code to check.
// uint32_t pos - The position of the bit to get.
//
// Returns:
// uint8_t - The bit.
uint8_t code_get_bit( Code *c, uint32_t pos ) {
	return 1 & ( c->bytes[ pos / 8 ] >> ( pos % 8 ) );
}

// Description:
// Pushes a bit to a code.
//
// Parameters:
// Code *c - The code to push to.
// uint8_t bit - The bit to push to the code.
//
// Returns:
// bool - Whether the bit was pushed successfully.
bool code_push_bit( Code *c, uint8_t bit ) {
	if ( code_full( c ) ) {
		return false;
	}

	if ( bit == 0 ) { // Clear bit.
		c->bytes[ c->top / 8 ] &= ~( 1 << ( c->top % 8 ) );
	} else { // Set bit.
		c->bytes[ c->top / 8 ] |= ( 1 << ( c->top % 8 ) );
	}

	c->top++;

	return true;
}

// Description:
// Pops a bit from a code.
//
// Parameters:
// Code *c - The code to pop from..
// uint8_t *bit - The pointer to a uint8_t to set to the popped bit.
//
// Returns:
// bool - Whether the bit was popped successfully.
bool code_pop_bit( Code *c, uint8_t *bit ) {
	if ( code_empty( c ) ) {
		return false;
	}

	c->top--;
	*bit = ( 1 & ( c->bytes[ c->top / 8 ] >> ( c->top % 8 ) ) );

	return true;
}
