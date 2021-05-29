#include "io.h"

#include "code.h"
#include "defines.h"

#include <stdint.h>
#include <unistd.h>

static uint8_t write_bit_buffer[ BLOCK ] = { 0 };
static uint32_t write_bit_top = 0;

// Description:
// Reads a certain number of bytes into a buffer or until no more can be read.
//
// Parameters:
// int infile - The input file.
// uint8_t *buf - The buffer to read to.
// uint32_t nbytes - The max number of bytes to read.
//
// Returns:
// uint32_t - How many bytes were read.
uint32_t read_bytes( int infile, uint8_t *buf, uint32_t nbytes ) {
	if ( nbytes == 0 ) {
		return 0;
	}

	uint32_t bytes_read = 0;
	uint32_t bytes_read_current_round = 0;

	while ( ( bytes_read_current_round = read( infile, buf + bytes_read, nbytes - bytes_read ) ) > 0 ) {
		bytes_read += bytes_read_current_round;

		if ( bytes_read == nbytes ) { // Finished reading max number of bytes.
			break;
		}
	}

	return bytes_read;
}

// Description:
// Writes a certain number of bytes into a buffer or until no more can be written. Will
// write to outfile when the buffer is full.
//
// Parameters:
// int outfile - The output file.
// uint8_t *buf - The buffer to write to.
// uint32_t nbytes - The max number of bytes to write.
//
// Returns:
// uint32_t - How many bytes were written.
uint32_t write_bytes( int outfile, uint8_t *buf, uint32_t nbytes ) {
	if ( nbytes == 0 ) {
		return 0;
	}

	uint32_t bytes_wrote = 0;
	uint32_t bytes_wrote_current_round = 0;

	while ( ( bytes_wrote_current_round = write( outfile, buf + bytes_wrote, nbytes - bytes_wrote ) ) > 0 ) {
		bytes_wrote += bytes_wrote_current_round;

		if ( bytes_wrote == nbytes ) { // Finished writing max number of bytes.
			break;
		}
	}

	return bytes_wrote;
}

// Description:
// Reads a bit from a file with a read buffer.
//
// Parameters:
// int infile - The input file.
// uint8_t *bit - The pointer to the uint8_t to set the bit to.
//
// Returns:
// bool - Whether the bit was read successfully.
bool read_bit( int infile, uint8_t *bit ) {
	static uint8_t read_bit_buffer[ BLOCK ] = { 0 }; // Read bit buffer.
	static uint32_t read_bit_top = 0; // Next bit to read.
	static uint32_t read_bit_buffer_size = 0; // Number of bytes read into buffer.

	if ( read_bit_top == 0 ) { // Refill buffer.
		read_bit_buffer_size = read_bytes( infile, read_bit_buffer, BLOCK );
	}

	if ( read_bit_buffer_size == 0 ) {
		return false;
	}

	*bit = ( 1 & ( read_bit_buffer[ read_bit_top / 8 ] >> ( read_bit_top % 8 ) ) );
	read_bit_top++;

	if ( read_bit_top == read_bit_buffer_size * 8 ) { // Done with read bit buffer.
		read_bit_top = 0;
	}

	return true;
}

// Description:
// Writes a code's bits to a file with a write buffer.
//
// Parameters:
// int outfile - The output file.
// Code *c - The code to write.
//
// Returns:
// uint64_t - Bytes actually written to file.
uint64_t write_code( int outfile, Code *c ) {
	uint64_t bytes_written = 0;

	for ( uint32_t i = 0; i < c->top; i++ ) {
		uint8_t bit = 1 & ( c->bytes[ i / 8 ] >> ( i % 8 ) );

		if ( bit == 0 ) { // Clear bit.
			write_bit_buffer[ write_bit_top / 8 ] &= ~( 1 << ( write_bit_top % 8 ) );
		} else { // Set bit.
			write_bit_buffer[ write_bit_top / 8 ] |= ( 1 << ( write_bit_top % 8 ) );
		}

		write_bit_top++;

		if ( write_bit_top == BLOCK * 8 ) { // Write bit buffer full.
			write_bytes( outfile, write_bit_buffer, BLOCK );
			bytes_written += BLOCK;
			write_bit_top = 0;
		}
	}

	return bytes_written;
}

// Description:
// Finishes writing out the write bit buffer and flushes the buffer.
//
// Parameters:
// int outfile - The output file.
//
// Returns:
// uint64_t - Bytes written to file.
uint64_t flush_codes( int outfile ) {
	if ( write_bit_top == 0 ) {
		return 0;
	}

	uint64_t bytes_written = 0;

	if ( write_bit_top % 8 == 0 ) { // Write buffer not on byte boundary.
		write_bytes( outfile, write_bit_buffer, write_bit_top / 8 );
		bytes_written += write_bit_top / 8;
	} else { // Write buffer on byte boundary.
		write_bit_buffer[ write_bit_top / 8 ] &= ( ( 1 << ( write_bit_top % 8 ) ) - 1 );
		write_bytes( outfile, write_bit_buffer, write_bit_top / 8 + 1 );
		bytes_written += write_bit_top / 8 + 1;
	}

	write_bit_top = 0;

	return bytes_written;
}
