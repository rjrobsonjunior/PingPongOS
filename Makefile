TARGET = pingpongos-semaphore
TARGET_QUEUE = pingpongos-mqueue
TARGET_RACECOND = pingpongos-racecond

SRC = ./src/ppos-core-aux.c ./src/testes/pingpong-semaphore.c 
SRC_QUEUE = ./src/ppos-core-aux.c ./src/testes/pingpong-mqueue.c 
SRC_RACECOND = ./src/ppos-core-aux.c ./src/testes/pingpong-racecond.c

OBJ = $(wildcard ./objects/*.o)

LIB = ./libppos_static.a

CC = gcc
INC=-I./lib

CFLAGS = -o $(TARGET) -Wall
CFLAGS_QUEUE = -o $(TARGET_QUEUE)
CFLAGS_RACECOND= -o $(TARGET_RACECOND) -Wall

all: build run

build-semaphore:build_lib
	$(CC) $(CFLAGS) $(SRC) $(LIB) $(INC) -lm

build-mqueue:build_lib
	$(CC) $(CFLAGS_QUEUE) $(SRC_QUEUE) $(LIB) $(INC) -lm $(INC) -lm

build-racecond:build_lib
	$(CC) $(CFLAGS_RACECOND) $(SRC_RACECOND) $(LIB) $(INC) -lm

run: build
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET_QUEUE) $(TARGET_RACECOND)

build_lib:
	ar rcs libppos_static.a $(OBJ)

.PHONY: all build run clean
