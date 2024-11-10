TARGET = pingpongos

SRC = ./src/ppos-core-aux.c ./src/pingpong-semaphore.c

LIB = ./src/libppos_static.a

CC = gcc

CFLAGS = -o $(TARGET)

all: build run

build:
	$(CC) $(CFLAGS) $(SRC) $(LIB)

run: build
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all build run clean