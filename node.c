#include "node.h"

#include <stdint.h>
#include <stdlib.h>

// Description:
// Initializes a node with a specified symbol and frequency.
//
// Parameters:
// uint8_t symbol - The symbol of the node.
// uint64_t frequency - The frequency of the node.
//
// Returns:
// Node * - A pointer to the newly initialized node.
Node *node_create( uint8_t symbol, uint64_t frequency ) {
	Node *n = ( Node * ) malloc( sizeof( Node ) );

	if ( n ) {
		n->left = n->right = NULL;
		n->symbol = symbol;
		n->frequency = frequency;
	}

	return n;
}

// Description:
// Frees the memory given to a node.
//
// Parameters:
// Node **s - A pointer to a pointer to the node to free the memory of.
//
// Returns:
// Nothing.
void node_delete( Node **n ) {
	if ( *n ) {
		free( *n );
		*n = NULL;
	}
}

// Description:
// Joins two child nodes and returns a pointer to a created parent node.
//
// Parameters:
// Node *left - The left child node.
// Node *right - The right child node.
//
// Returns:
// Node * - The created parent node.
Node *node_join( Node *left, Node *right ) {
	Node *n = node_create( '$', left->frequency + right->frequency );
	n->left = left;
	n->right = right;

	return n;
}
