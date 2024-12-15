TARGET = pingpongos

SRC = ./src/ppos-core-aux.c ./src/testes/pingpong-semaphore.c 

OBJ = $(wildcard ./objects/*.o)

LIB = ./libppos_static.a

CC = gcc

CFLAGS = -o $(TARGET) -Wall

all: build run

build:build_lib
	$(CC) $(CFLAGS) $(SRC) $(LIB) -lm

run: build
	./$(TARGET)

clean:
	rm -f $(TARGET)

build_lib:
	ar rcs libppos_static.a $(OBJ)

.PHONY: all build run clean
