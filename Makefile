CC = gcc
CFLAGS = -Wall -pedantic -std=c99

build/asm: build/assembler.o build/preprocess.o
	$(CC) $(CLFAGS) build/assembler.o build/preprocess.o -o build/asm

build/assembler.o: src/assembler.c src/assembler.h src/preprocess.h
	$(CC) $(CFLAGS) -c src/assembler.c -o build/assembler.o

build/preprocess.o: src/preprocess.c
	$(CC) $(CFLAGS) -c src/preprocess.c -o build/preprocess.o

clean:
	rm build/*