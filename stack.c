#include "stack.h"

#include "node.h"

#include <stdint.h>
#include <stdlib.h>

// Description:
// A struct for the Stack ADT.
//
// Members:
// uint32_t top - Index of the next empty slot in the stack.
// uint32_t capacity - Capacity of the stack.
// Node **items - Holds the items.
struct Stack {
	uint32_t top;
	uint32_t capacity;
	Node **items;
};

// Description:
// Initializes a stack with a specified capacity.
//
// Parameters:
// uint32_t capacity - The max capacity of the stack.
//
// Returns:
// Stack * - A pointer to the newly initialized stack.
Stack *stack_create( uint32_t capacity ) {
	Stack *s = ( Stack * ) malloc( sizeof( Stack ) );

	if ( s ) {
		s->top = 0;
		s->capacity = capacity;
		s->items = malloc( capacity * sizeof( Node * ) );

		if ( !s->items ) {
			free( s );
			s = NULL;
		}
	}

	return s;
}

// Description:
// Frees the memory given to a stack.
//
// Parameters:
// Stack **s - A pointer to a pointer to the stack to free the memory of.
//
// Returns:
// Nothing.
void stack_delete( Stack **s ) {
	if ( *s && ( *s )->items ) {
		free( ( *s )->items );
		free( *s );
		*s = NULL;
	}
}

// Description:
// Checks if a stack is empty.
//
// Parameters:
// Stack *s - The stack to check.
//
// Returns:
// bool - Whether the stack is empty.
bool stack_empty( Stack *s ) {
	return s->top == 0;
}

// Description:
// Checks if a stack is full.
//
// Parameters:
// Stack *s - The stack to check.
//
// Returns:
// bool - Whether the stack is full.
bool stack_full( Stack *s ) {
	return s->top == s->capacity;
}

// Description:
// Pushes a node to a stack.
//
// Parameters:
// Stack *s - The stack to push to.
// Node *n - The node to push to the stack.
//
// Returns:
// bool - Whether the operation was successful.
bool stack_push( Stack *s, Node *n ) {
	if ( stack_full( s ) ) {
		return false;
	}

	s->items[ s->top ] = n;
	s->top++;

	return true;
}

// Description:
// Pops a node from a stack.
//
// Parameters:
// Stack *s - The stack to pop from.
// Node **x - A pointer to a pointer to a Node to set the popped node to.
//
// Returns:
// bool - Whether the operation was successful.
bool stack_pop( Stack *s, Node **n ) {
	if ( stack_empty( s ) ) {
		return false;
	}

	s->top--;
	*n = s->items[ s->top ];

	return true;
}
