#include "defines.h"
#include "file_header.h"
#include "huffman.h"
#include "io.h"

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define OPTIONS "hvi:o:" // Valid options for the program.

static int input_file = -1;
static int output_file = -1;

// Description:
// Prints the help message to stderr.
//
// Parameters:
// char *program_path - The path to the program.
//
// Returns:
// Nothing.
static void print_help( char *program_path ) {
	fprintf( stderr,
		"SYNOPSIS\n   A Huffman decoder implementation.\n\nUSAGE\n   %s [-hv] [-i infile] [-o outfile]\n\nOPTIONS\n   -h             Prints the program help text.\n   -v             Prints "
		"compression statistics to stderr.\n   -i infile      Input file to decompress.\n   -o outfile     File to output the decompressed data to.\n",
		program_path );
}

// Description:
// Cleans up memory used by the program if it's been allocated.
//
// Parameters:
// Nothing.
//
// Returns:
// Nothing.
static void cleanup_memory( ) {
	if ( output_file != -1 ) {
		close( output_file );
		output_file = -1;
	}

	if ( input_file != -1 ) {
		close( input_file );
		input_file = -1;
	}
}

// Description:
// Processes the file names inputted by the user.
//
// Parameters:
// char *input_file_name - The input file name given by the user.
// char *output_file_name - The output file name given by the user.
//
// Returns:
// bool - Whether processing was successful.
static bool process_input_output_files( char *input_file_name, char *output_file_name ) {
	if ( input_file_name && ( input_file = open( input_file_name, O_RDONLY ) ) <= 0 ) {
		fprintf( stderr, "Error: failed to open infile.\n" );

		return false;
	}

	if ( output_file_name && ( output_file = open( output_file_name, O_WRONLY | O_CREAT | O_TRUNC ) ) <= 0 ) {
		fprintf( stderr, "Error: failed to open outfile.\n" );

		return false;
	}

	return true;
}

// Description:
// Decodes codes read from the file and writes the decoded symbol to the output file.
//
// Parameters:
// Node *huffman_tree - The Huffman tree used to find symbols.
// uint64_t file_size - The size of the decoded file in bytes.
// uint64_t *compressed_size - Pointer to uint64_t to add number of bytes written to.
//
// Returns:
// bool - Whether the codes were able to be decoded.
static bool write_decoded_codes( Node *huffman_tree, uint64_t file_size, uint64_t *compressed_size ) {
	uint64_t symbols_written = 0;
	Node *current_node = huffman_tree;
	uint8_t write_buffer[ BLOCK ] = { 0 };
	uint32_t write_buffer_top = 0;

	while ( symbols_written < file_size ) {
		uint8_t bit;

		if ( !read_bit( input_file, &bit ) ) {
			return false;
		}

		// Walk down the correct node.
		if ( bit == 0 ) {
			current_node = current_node->left;
		} else { // Bit is a 1.
			current_node = current_node->right;
		}

		if ( !current_node ) {
			return false;
		}

		if ( !current_node->left && !current_node->right ) { // Node is a leaf.
			write_buffer[ write_buffer_top ] = current_node->symbol; // Write to buffer.
			write_buffer_top++;

			if ( write_buffer_top == BLOCK ) { // Write buffer is full.
				write_bytes( output_file, write_buffer, BLOCK );
				*compressed_size += BLOCK;
				write_buffer_top = 0;
			}

			symbols_written++;
			current_node = huffman_tree;
		}
	}

	write_bytes( output_file, write_buffer, write_buffer_top ); // Flush write buffer.
	*compressed_size += write_buffer_top;

	return true;
}

// Description:
// The entry point of the program.
//
// Parameters:
// int argc - The argument count.
// char **argv - An array of argument strings.
//
// Returns:
// int - The exit status of the program (0 = success, otherwise error).
int main( int argc, char **argv ) {
	int opt = 0;
	bool verbose = false;
	char *input_file_name = NULL;
	char *output_file_name = NULL;

	while ( ( opt = getopt( argc, argv, OPTIONS ) ) != -1 ) { // Process each option specified.
		switch ( opt ) {
		case 'h': print_help( *argv ); return 0; // Help.
		case 'v': verbose = true; break; // Verbose.
		case 'i': input_file_name = optarg; break; // Input file.
		case 'o': output_file_name = optarg; break; // Output file.
		default: print_help( *argv ); return 1; // Invalid flag.
		}
	}

	input_file = STDIN_FILENO;
	output_file = STDOUT_FILENO;

	if ( !process_input_output_files( input_file_name, output_file_name ) ) {
		cleanup_memory( );

		return 1;
	}

	uint64_t compressed_size = 0;
	FileHeader header = { 0 };
	read_bytes( input_file, ( uint8_t * ) &header, sizeof( header ) );
	compressed_size += sizeof( header );

	if ( header.magic_number != MAGIC ) {
		fprintf( stderr, "Error: unable to read file header. Invalid input file or input file corrupted.\n" );

		if ( output_file_name ) {
			unlink( output_file_name ); // Delete output file.
		}

		cleanup_memory( );

		return 1;
	}

	if ( output_file_name ) { // Write perms to output_file, if it exists.
		fchmod( output_file, header.permissions );
	}

	uint8_t tree_dump[ header.tree_size ];
	int tree_dump_bytes_read = read_bytes( input_file, tree_dump, header.tree_size );

	if ( tree_dump_bytes_read != header.tree_size ) {
		fprintf( stderr, "Error: input file corrupted.\n" );

		if ( output_file_name ) {
			unlink( output_file_name ); // Delete output file.
		}

		cleanup_memory( );

		return 1;
	}

	compressed_size += header.tree_size;
	Node *huffman_tree = rebuild_tree( header.tree_size, tree_dump );

	if ( !write_decoded_codes( huffman_tree, header.original_file_size, &compressed_size ) ) {
		fprintf( stderr, "Error: input file corrupted.\n" );

		if ( output_file_name ) {
			unlink( output_file_name ); // Delete output file.
		}

		delete_tree( &huffman_tree );
		cleanup_memory( );

		return 1;
	}

	if ( verbose ) {
		double space_saving = 100 * ( 1 - ( ( double ) compressed_size / header.original_file_size ) );
		fprintf( stderr, "Compressed file size: %" PRIu64 " bytes\n", compressed_size );
		fprintf( stderr, "Decompressed file size: %" PRIu64 " bytes\n", header.original_file_size );
		fprintf( stderr, "Space saving: %.2f%%\n", space_saving );
	}

	delete_tree( &huffman_tree );
	cleanup_memory( );

	return 0;
}
