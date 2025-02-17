TARGET = pingpongos-semaphore
TARGET_QUEUE = pingpongos-mqueue
TARGET_RACECOND = pingpongos-racecond
TARGET_DISK1 = pingpongos-disk1


SRC = ./src/ppos-core-aux.c ./src/testes/pingpong-semaphore.c 
SRC_QUEUE = ./src/ppos-core-aux.c ./src/testes/pingpong-mqueue.c 
SRC_RACECOND = ./src/ppos-core-aux.c ./src/testes/pingpong-racecond.c
SRC_DISK1 = ./src/testes/disk-driver.c ./src/testes/ppos-disk-manager.c ./objects/ppos-all.o ./objects/queue.o ./src/testes/pingpong-disco1.c ./src/ppos-core-aux.c
OBJ = $(wildcard ./objects/*.o)

LIB = ./libppos_static.a

CC = gcc
INC=-I./lib
CFLAGS = -o $(TARGET) -Wall
CFLAGS_QUEUE = -o $(TARGET_QUEUE)
CFLAGS_RACECOND= -o $(TARGET_RACECOND) -Wall
CFLAGS_DISK1 = -o $(TARGET_DISK1)
all: build run

build-semaphore:build_lib
	$(CC) $(CFLAGS) $(SRC) $(LIB) $(INC) -lm

build-mqueue:build_lib
	$(CC) $(CFLAGS_QUEUE) $(SRC_QUEUE) $(LIB) $(INC) -lm $(INC) -lm

build-racecond:build_lib
	$(CC) $(CFLAGS_RACECOND) $(SRC_RACECOND) $(LIB) $(INC) -lm

build-disk1:
	$(CC) $(CFLAGS_DISK1) $(SRC_DISK1) $(INC) -lrt -Wall

run: build
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET_QUEUE) $(TARGET_RACECOND)

build_lib:
	ar rcs libppos_static.a $(OBJ)

.PHONY: all build run clean
