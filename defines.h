#ifndef __DEFINES_H__
#define __DEFINES_H__

#define BLOCK		  4096 // 4KB blocks for I/O.
#define ALPHABET	  256 // Number ASCII + extended ASCII characters.
#define MAGIC		  0x121DDBC0 // 32-bit magic number.
#define MAX_CODE_SIZE ( ALPHABET / 8 ) // Bytes for a maximum, 256-bit code.

#endif
