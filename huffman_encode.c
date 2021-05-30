#include "defines.h"
#include "file_header.h"
#include "huffman.h"
#include "io.h"
#include "raw_file_header.h"

#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define SIZE_OF_TEMP_FILE_NAME 35 // Max size of the temporary file name.
#define OPTIONS				   "hvi:o:" // Valid options for the program.

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
		"SYNOPSIS\n   A Huffman encoder implementation.\n\nUSAGE\n   %s [-hv] [-i infile] [-o outfile]\n\nOPTIONS\n   -h             Prints the program help text.\n   -v             Prints "
		"compression statistics to stderr.\n   -i infile      Input file to compress.\n   -o outfile     File to output the compressed data to.\n",
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

	if ( output_file_name && ( output_file = open( output_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0600 ) ) <= 0 ) {
		fprintf( stderr, "Error: failed to open outfile.\n" );

		return false;
	}

	return true;
}

// Description:
// Generates a histogram for the input file, and creates a temporary file
// to use as the input file, if the input file is not seekable.
//
// Parameters:
// uint64_t histogram[static ALPHABET] - The histogram to write to.
//
// Returns:
// uint32_t - The number of unique symbols found in the input.
static uint32_t generate_histogram_and_temp_input_file( uint64_t histogram[ static ALPHABET ] ) {
	int temp_file_fd = -1;

	// Create a temporary file with file's contents to allow for seeking.
	if ( lseek( input_file, 0, SEEK_CUR ) == -1 ) { // input_file not seekable.
		char temp_file_name[ SIZE_OF_TEMP_FILE_NAME ];
		// Build "unique" file name.
		snprintf( temp_file_name, SIZE_OF_TEMP_FILE_NAME, "/tmp/huffman.%d", getpid( ) );
		temp_file_fd = open( temp_file_name, O_RDWR | O_CREAT | O_EXCL | O_TRUNC, 0600 );
		unlink( temp_file_name ); // Unlink, so temp file is deleted when program exits.
	}

	uint32_t unique_symbols = 0;
	uint8_t read_buffer[ BLOCK ] = { 0 };
	uint32_t read_byte_top = 0; // Next byte to read.
	uint32_t read_byte_buffer_size = 0; // Number of bytes read into buffer.
	bool more_to_read = true;

	while ( more_to_read ) {
		if ( read_byte_top == 0 ) { // Refill buffer.
			read_byte_buffer_size = read_bytes( input_file, read_buffer, BLOCK );

			// Write to temp file if input file is not seekable.
			if ( read_byte_buffer_size && temp_file_fd != -1 ) {
				write_bytes( temp_file_fd, read_buffer, read_byte_buffer_size );
			}
		}

		if ( read_byte_buffer_size != 0 ) {
			uint8_t byte = read_buffer[ read_byte_top ];

			if ( histogram[ byte ] == 0 ) { // Found a unique symbol.
				unique_symbols++;
			}

			histogram[ byte ]++; // Add to histogram.
			read_byte_top++;

			if ( read_byte_top == read_byte_buffer_size ) { // Done with read buffer.
				read_byte_top = 0;
			}
		} else {
			more_to_read = false;
		}
	}

	if ( temp_file_fd != -1 ) { // Set input file to temp file if input_file is not seekable.
		close( input_file );
		input_file = temp_file_fd;
	}

	return unique_symbols;
}

// Description:
// Writes a Huffman tree dump to the output file.
//
// Parameters:
// Node *huffman_tree - The root node of the Huffman tree to write.
// uint64_t *compressed_size - Pointer to uint64_t to add number of bytes written to.
//
// Returns:
// Nothing.
static void write_tree_to_outfile( Node *huffman_tree, uint64_t *compressed_size ) {
	if ( !huffman_tree ) {
		return;
	}

	write_tree_to_outfile( huffman_tree->left, compressed_size );
	write_tree_to_outfile( huffman_tree->right, compressed_size );

	if ( !huffman_tree->left && !huffman_tree->right ) { // Node is a leaf.
		// Write 'L' followed by the symbol.
		uint8_t symbol[ 2 ] = { 'L', huffman_tree->symbol };
		write_bytes( output_file, symbol, 2 );
		*compressed_size += 2;
	} else { // Node is an interior node.
		// Write 'I'.
		uint8_t symbol = 'I';
		write_bytes( output_file, &symbol, 1 );
		*compressed_size += 1;
	}
}

// Description:
// Writes codes for each symbol in the input file.
//
// Parameters:
// Code huffman_code_table[static ALPHABET] - The code table for the Huffman tree.
//
// Returns:
// uint64_t - The size of all of the codes in bytes (rounded up).
static uint64_t write_codes_for_symbols( Code huffman_code_table[ static ALPHABET ] ) {
	lseek( input_file, 0, SEEK_SET ); // Seek to beginning of file.
	uint64_t byte_count = 0;
	uint8_t read_buffer[ BLOCK ] = { 0 };
	uint32_t read_byte_top = 0; // Next byte to read.
	uint32_t read_byte_buffer_size = 0; // Number of bytes read into buffer.
	bool more_to_read = true;

	// Loop through all symbols in file and write code for symbol.
	while ( more_to_read ) {
		if ( read_byte_top == 0 ) { // Refill buffer.
			read_byte_buffer_size = read_bytes( input_file, read_buffer, BLOCK );
		}

		if ( read_byte_buffer_size != 0 ) {
			uint8_t byte = read_buffer[ read_byte_top ];
			byte_count += write_code( output_file, &huffman_code_table[ byte ] );
			read_byte_top++;

			if ( read_byte_top == read_byte_buffer_size ) { // Done with read buffer.
				read_byte_top = 0;
			}
		} else {
			more_to_read = false;
		}
	}

	byte_count += flush_codes( output_file ); // Flush write code buffer.

	return byte_count;
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

	uint64_t histogram[ ALPHABET ] = { 0 };
	uint32_t unique_symbols = generate_histogram_and_temp_input_file( histogram );

	Node *huffman_tree = build_tree( histogram );
	Code huffman_code_table[ ALPHABET ] = { 0 };
	build_codes( huffman_tree, huffman_code_table );

	struct stat input_file_stats;
	fstat( input_file, &input_file_stats );

	if ( output_file_name ) { // Write perms to output_file, if it exists.
		fchmod( output_file, input_file_stats.st_mode );
	}

	uint64_t compressed_size = 0;
	FileHeader output_header = { 0 };
	output_header.magic_number = MAGIC;
	output_header.permissions = input_file_stats.st_mode;

	if ( unique_symbols != 0 ) {
		output_header.tree_size = 3 * unique_symbols - 1;
	}

	output_header.original_file_size = input_file_stats.st_size;
	RawFileHeader output_raw_header = raw_file_header_create( output_header );
	write_bytes( output_file, ( uint8_t * ) &output_raw_header, sizeof( output_raw_header ) ); // Write raw file header.
	compressed_size += sizeof( output_raw_header );
	write_tree_to_outfile( huffman_tree, &compressed_size );
	compressed_size += write_codes_for_symbols( huffman_code_table );

	if ( verbose ) {
		double space_saving = 100 * ( 1 - ( ( double ) compressed_size / output_header.original_file_size ) );
		fprintf( stderr, "Uncompressed file size: %" PRIu64 " bytes\n", output_header.original_file_size );
		fprintf( stderr, "Compressed file size: %" PRIu64 " bytes\n", compressed_size );
		fprintf( stderr, "Space saving: %.2f%%\n", space_saving );
	}

	delete_tree( &huffman_tree );
	cleanup_memory( );

	return 0;
}

// TO-DO: Remove file permissions from file header and raw file header. Make decoder take permissions from input file instead.
