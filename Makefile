SOURCEFILES_1 = huffman_encode.c
OBJECTFILES_1 = huffman_encode.o
OUTPUT_1 = huffman_encode

SOURCEFILES_2 = huffman_decode.c
OBJECTFILES_2 = huffman_decode.o
OUTPUT_2 = huffman_decode

SOURCEFILES_DEPENDENCIES_1_2 = code.c huffman.c io.c node.c pq.c stack.c
OBJECTFILES_DEPENDENCIES_1_2 = code.o huffman.o io.o node.o pq.o stack.o

CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic

.PHONY: all debug clean format

all: $(OUTPUT_1) $(OUTPUT_2)

$(OUTPUT_1): $(OBJECTFILES_1) $(OBJECTFILES_DEPENDENCIES_1_2)
	$(CC) -o $(OUTPUT_1) $(OBJECTFILES_1) $(OBJECTFILES_DEPENDENCIES_1_2)

$(OUTPUT_2): $(OBJECTFILES_2) $(OBJECTFILES_DEPENDENCIES_1_2)
	$(CC) -o $(OUTPUT_2) $(OBJECTFILES_2) $(OBJECTFILES_DEPENDENCIES_1_2)

$(OBJECTFILES_1): $(SOURCEFILES_1)
	$(CC) $(CFLAGS) -c $(SOURCEFILES_1)

$(OBJECTFILES_2): $(SOURCEFILES_2)
	$(CC) $(CFLAGS) -c $(SOURCEFILES_2)

$(OBJECTFILES_DEPENDENCIES_1_2): $(SOURCEFILES_DEPENDENCIES_1_2)
	$(CC) $(CFLAGS) -c $(SOURCEFILES_DEPENDENCIES_1_2)

debug: CFLAGS += -g -O0
debug: all

clean:
	rm -f $(OUTPUT_1) $(OUTPUT_2) $(OBJECTFILES_1) $(OBJECTFILES_2) $(OBJECTFILES_DEPENDENCIES_1_2)

format:
	clang-format -i -style=file *.[ch]
