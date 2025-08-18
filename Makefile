CC = gcc
CFLAGS = -Wall -pedantic -std=c99

BIN = build/asm

OBJS = build/assembler.o build/preprocess.o build/hashtable.o build/first_pass.o build/second_pass.o

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BIN)

build/assembler.o: src/assembler.c src/assembler.h src/preprocess.h src/hashtable.h src/first_pass.h
	$(CC) $(CFLAGS) -c src/assembler.c -o build/assembler.o

build/preprocess.o: src/preprocess.c src/preprocess.h
	$(CC) $(CFLAGS) -c src/preprocess.c -o build/preprocess.o

build/hashtable.o: src/hashtable.c src/hashtable.h
	$(CC) $(CFLAGS) -c src/hashtable.c -o build/hashtable.o

build/first_pass.o: src/first_pass.c src/first_pass.h src/preprocess.h src/hashtable.h
	$(CC) $(CFLAGS) -c src/first_pass.c -o build/first_pass.o

build/second_pass.o: src/second_pass.c src/second_pass.h src/preprocess.h src/hashtable.h
	$(CC) $(CFLAGS) -c src/second_pass.c -o build/second_pass.o

clean:
	rm -f build/*.o $(BIN)