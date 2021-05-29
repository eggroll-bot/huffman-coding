#ifndef __HEADER_H__
#define __HEADER_H__

#include <stdint.h>

typedef struct FileHeader {
	uint32_t magic_number;
	uint16_t permissions;
	uint16_t tree_size;
	uint64_t original_file_size;
} FileHeader;

#endif
