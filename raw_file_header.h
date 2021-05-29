#ifndef __RAW_FILE_HEADER_H__
#define __RAW_FILE_HEADER_H__

#include <stdint.h>

typedef struct RawFileHeader {
	uint8_t magic_number[ 4 ];
	uint8_t permissions[ 2 ];
	uint8_t tree_size[ 2 ];
	uint8_t original_file_size[ 8 ];
} FileHeader;

#endif
