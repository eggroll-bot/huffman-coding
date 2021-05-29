#ifndef __IO_H__
#define __IO_H__

#include "code.h"

#include <stdbool.h>
#include <stdint.h>

uint32_t read_bytes( int infile, uint8_t *buf, uint32_t nbytes );

uint32_t write_bytes( int outfile, uint8_t *buf, uint32_t nbytes );

bool read_bit( int infile, uint8_t *bit );

uint64_t write_code( int outfile, Code *c );

uint64_t flush_codes( int outfile );

#endif
