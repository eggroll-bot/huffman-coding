#include "huffman.h"

#include "code.h"
#include "defines.h"
#include "node.h"
#include "priority_queue.h"
#include "stack.h"

#include <stdint.h>
#include <stdlib.h>

// Description:
// Builds a Huffman tree from a histogram.
//
// Parameters:
// uint64_t hist[static ALPHABET] - The histogram to build the tree from.
//
// Returns:
// Node * - The root node of the newly created Huffman tree.
Node *build_tree( uint64_t hist[ static ALPHABET ] ) {
	PriorityQueue *huffman_pq = pq_create( ALPHABET );

	for ( uint32_t i = 0; i < ALPHABET; i++ ) { // Enqueue all leaf nodes.
		uint64_t frequency = hist[ i ];

		if ( frequency > 0 ) {
			Node *node = node_create( i, frequency );
			enqueue( huffman_pq, node );
		}
	}

	while ( pq_size( huffman_pq ) >= 2 ) { // Join leaf nodes until there are no more.
		Node *left_child, *right_child;
		dequeue( huffman_pq, &left_child );
		dequeue( huffman_pq, &right_child );
		enqueue( huffman_pq, node_join( left_child, right_child ) );
	}

	Node *root_node = NULL;
	dequeue( huffman_pq, &root_node ); // Dequeue root node.
	pq_delete( &huffman_pq );

	return root_node;
}

// Description:
// Builds a table of codes from a Huffman tree.
//
// Parameters:
// Node *root - The root node of the Huffman tree.
// Code table[static ALPHABET] - The table of codes.
//
// Returns:
// Nothing.
void build_codes( Node *root, Code table[ static ALPHABET ] ) {
	static Code code = { 0 };

	if ( !root ) { // Empty Huffman tree.
		return;
	}

	if ( !root->left && !root->right ) { // Node is a leaf.
		table[ root->symbol ] = code;
	} else { // Node is an interior node.
		uint8_t popped_bit;
		code_push_bit( &code, 0 );
		build_codes( root->left, table );
		code_pop_bit( &code, &popped_bit );
		code_push_bit( &code, 1 );
		build_codes( root->right, table );
		code_pop_bit( &code, &popped_bit );
	}
}

// Description:
// Builds a Huffman tree from a post-order tree dump.
//
// Parameters:
// uint16_t nbytes - The number of bytes in the tree dump.
// uint8_t tree[static nbytes] - The tree dump.
//
// Returns:
// Node * - The root node of the built Huffman tree.
Node *rebuild_tree( uint16_t nbytes, uint8_t tree[ static nbytes ] ) {
	Stack *huffman_stack = stack_create( nbytes );

	for ( uint16_t i = 0; i < nbytes; i++ ) {
		if ( tree[ i ] == 'L' ) { // Leaf node.
			stack_push( huffman_stack, node_create( tree[ i + 1 ], 1 ) );
			i++; // Skip next element.
		} else if ( tree[ i ] == 'I' ) { // Interior node.
			Node *left_child;
			Node *right_child;
			stack_pop( huffman_stack, &right_child );
			stack_pop( huffman_stack, &left_child );
			stack_push( huffman_stack, node_join( left_child, right_child ) );
		}
	}

	Node *root_node = NULL;
	stack_pop( huffman_stack, &root_node );
	stack_delete( &huffman_stack );

	return root_node;
}

// Description:
// Deletes a Huffman tree.
//
// Parameters:
// Node **root - A pointer to a pointer to the root node of the Huffman tree.
//
// Returns:
// Nothing.
void delete_tree( Node **root ) {
	if ( !*root ) {
		return;
	}

	delete_tree( &( *root )->left );
	delete_tree( &( *root )->right );
	node_delete( root );
}
