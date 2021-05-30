#include "raw_file_header.h"

#include "file_header.h"

#include <stdint.h>

RawFileHeader raw_file_header_create( FileHeader file_header ) {
	RawFileHeader raw_file_header = { 0 };
	// Write struct members in little-endian format to raw_file_header.
	raw_file_header.magic_number[ 0 ] = file_header.magic_number;
	raw_file_header.magic_number[ 1 ] = file_header.magic_number >> 8;
	raw_file_header.magic_number[ 2 ] = file_header.magic_number >> 16;
	raw_file_header.magic_number[ 3 ] = file_header.magic_number >> 24;
	raw_file_header.tree_size[ 0 ] = file_header.tree_size;
	raw_file_header.tree_size[ 0 ] = file_header.tree_size;
	raw_file_header.original_file_size[ 0 ] = file_header.original_file_size;
	raw_file_header.original_file_size[ 1 ] = file_header.original_file_size >> 8;
	raw_file_header.original_file_size[ 2 ] = file_header.original_file_size >> 16;
	raw_file_header.original_file_size[ 3 ] = file_header.original_file_size >> 24;
	raw_file_header.original_file_size[ 4 ] = file_header.original_file_size >> 32;
	raw_file_header.original_file_size[ 5 ] = file_header.original_file_size >> 40;
	raw_file_header.original_file_size[ 6 ] = file_header.original_file_size >> 48;
	raw_file_header.original_file_size[ 7 ] = file_header.original_file_size >> 56;

	return raw_file_header;
}

FileHeader file_header_create( RawFileHeader raw_file_header ) {
	FileHeader file_header = { 0 };
	// Read struct members in little-endian format to file_header.
	file_header.magic_number = raw_file_header.magic_number[ 0 ] | ( uint_fast16_t ) raw_file_header.magic_number[ 1 ] << 8 | ( uint_fast32_t ) raw_file_header.magic_number[ 2 ] << 16
							   | ( uint_fast32_t ) raw_file_header.magic_number[ 3 ] << 24;

	file_header.tree_size = raw_file_header.tree_size[ 0 ] | ( uint_fast16_t ) raw_file_header.tree_size[ 1 ] << 8;

	file_header.original_file_size = raw_file_header.original_file_size[ 0 ] | ( uint_fast16_t ) raw_file_header.original_file_size[ 1 ] << 8
									 | ( uint_fast32_t ) raw_file_header.original_file_size[ 2 ] << 16 | ( uint_fast32_t ) raw_file_header.original_file_size[ 3 ] << 24
									 | ( uint_fast64_t ) raw_file_header.original_file_size[ 4 ] << 32 | ( uint_fast64_t ) raw_file_header.original_file_size[ 5 ] << 40
									 | ( uint_fast64_t ) raw_file_header.original_file_size[ 6 ] << 48 | ( uint_fast64_t ) raw_file_header.original_file_size[ 7 ] << 56;

	return file_header;
}
