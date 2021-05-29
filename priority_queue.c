#include "priority_queue.h"

#include "node.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Description:
// A struct for the priority queue ADT.
//
// Members:
// uint32_t head - The head of the priority queue.
// uint32_t tail - The tail of the priority queue.
// uint32_t size - The size of the priority queue.
// uint32_t capacity - The capacity of the priority queue.
// Node **items - The array of Node *s backing the priority queue.
struct PriorityQueue {
	uint32_t head;
	uint32_t tail;
	uint32_t size;
	uint32_t capacity;
	Node **items;
};

// Description:
// Creates a priority queue.
//
// Parameters:
// uint32_t capacity - The capacity of the priority queue.
//
// Returns:
// PriorityQueue * - A pointer to the newly created priority queue.
PriorityQueue *pq_create( uint32_t capacity ) {
	PriorityQueue *pq = ( PriorityQueue * ) malloc( sizeof( PriorityQueue ) );

	if ( pq ) {
		pq->head = pq->tail = pq->size = 0;
		pq->capacity = capacity;
		pq->items = ( Node ** ) malloc( capacity * sizeof( Node * ) );

		if ( !pq->items ) {
			free( pq );
			pq = NULL;
		}
	}

	return pq;
}

// Description:
// Frees the memory taken by a priority queue.
//
// Parameters:
// PriorityQueue **q - The priority queue to delete.
//
// Returns:
// Nothing.
void pq_delete( PriorityQueue **q ) {
	if ( *q && ( *q )->items ) {
		free( ( *q )->items );
		free( *q );
		*q = NULL;
	}
}

// Description:
// Checks if a priority queue is empty.
//
// Parameters:
// PriorityQueue *q - The priority queue to check.
//
// Returns:
// bool - Whether the priority queue is empty.
bool pq_empty( PriorityQueue *q ) {
	return q->size == 0;
}

// Description:
// Checks if a priority queue is full.
//
// Parameters:
// PriorityQueue *q - The priority queue to check.
//
// Returns:
// bool - Whether the priority queue is full.
bool pq_full( PriorityQueue *q ) {
	return q->size == q->capacity;
}

// Description:
// Checks the size of a priority queue.
//
// Parameters:
// PriorityQueue *q - The priority queue to check.
//
// Returns:
// uint32_t - The size of the priority queue.
uint32_t pq_size( PriorityQueue *q ) {
	return q->size;
}

// Description:
// Gets an element from the priority queue as if the priority queue were an array.
//
// Parameters:
// PriorityQueue *q - The priority queue to get from.
// uint32_t index - The index of the element to get.
//
// Returns:
// Node * - The retrieved element.
static Node *get_pq_element( PriorityQueue *q, uint32_t index ) {
	uint32_t head = q->head;

	if ( head + index < q->capacity ) { // No wrap around.
		return q->items[ head + index ];
	} else { // Wrap around.
		return q->items[ head + index - q->capacity ];
	}
}

// Description:
// Sets an element in the priority queue as if the priority queue were an array.
//
// Parameters:
// PriorityQueue *q - The priority queue to set.
// uint32_t index - The index of the element to set.
// Node *n - The node to set the index in the priority queue to.
//
// Returns:
// Nothing.
static void set_pq_element( PriorityQueue *q, uint32_t index, Node *n ) {
	uint32_t head = q->head;

	if ( head + index < q->capacity ) { // No wrap around.
		q->items[ head + index ] = n;
	} else { // Wrap around.
		q->items[ head + index - q->capacity ] = n;
	}
}

// Description:
// Finds the position to insert an element of a certain frequency to with a binary search algorithm.
//
// Parameters:
// PriorityQueue *q - The priority queue to search.
// uint64_t frequency - The frequency of the element to insert.
// int64_t left - The left bound of the range to search.
// int64_t right - The right bound of the range to search.
//
// Returns:
// uint32_t - The position to insert the element to.
static uint32_t find_queue_insert_position( PriorityQueue *q, uint64_t frequency, int64_t left, int64_t right ) {
	if ( pq_empty( q ) ) {
		return 0;
	}

	if ( left >= right ) { // Done with search.
		uint64_t left_element_frequency = get_pq_element( q, left )->frequency;

		if ( frequency <= left_element_frequency ) {
			return left;
		} else {
			return left + 1;
		}
	}

	int64_t middle = left + ( right - left ) / 2;
	uint64_t middle_element_frequency = get_pq_element( q, middle )->frequency;

	// Find next search bounds.
	if ( frequency == middle_element_frequency ) {
		return middle;
	} else if ( frequency < middle_element_frequency ) {
		return find_queue_insert_position( q, frequency, left, middle - 1 );
	} else {
		return find_queue_insert_position( q, frequency, middle + 1, right );
	}
}

// Description:
// Enqueues an element into a priority queue.
//
// Parameters:
// PriorityQueue *q - The priority queue to insert to.
// Node *n - The element to insert.
//
// Returns:
// bool - Whether the element was enqueued successfully.
bool enqueue( PriorityQueue *q, Node *n ) {
	if ( pq_full( q ) ) {
		return false;
	}

	uint32_t insert_position = find_queue_insert_position( q, n->frequency, 0, q->size - 1 );
	q->tail++;

	if ( q->tail == q->capacity ) { // Wrap tail around.
		q->tail = 0;
	}

	q->size++;

	if ( !pq_empty( q ) && insert_position != q->size - 1 ) {
		// Shift all elements above insert_position up by one to make space for the element to be inserted.
		for ( int64_t i = q->size - 2; i >= insert_position; i-- ) {
			set_pq_element( q, i + 1, get_pq_element( q, i ) );
		}
	}

	set_pq_element( q, insert_position, n ); // Insert element in empty space.

	return true;
}

// Description:
// Dequeues an element from a priority queue.
//
// Parameters:
// PriorityQueue *q - The priority queue to dequeue from.
// Node **n - A pointer to the Node * to set.
//
// Returns:
// bool - Whether the element was dequeued successfully.
bool dequeue( PriorityQueue *q, Node **n ) {
	if ( pq_empty( q ) ) {
		return false;
	}

	*n = q->items[ q->head ];
	q->head++;

	if ( q->head == q->capacity ) { // Wrap head around.
		q->head = 0;
	}

	q->size--;

	return true;
}
