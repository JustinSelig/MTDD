#==========================================================================
# Makefile
#
# Justin S. Selig
#==========================================================================
# @brief: A makefile that compiles the eeprom driver test program
#
# @desc: 1. "make" runs eeprom by default
#        2. "make clean" cleans up the directory

# use native gcc compiler
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#  -lm for potential use of math library
#  -lpthread for pthreads
CFLAGS = -g -Wall -lm -lpthread

# target built executable
TARGET = eeprom_test

# src file dependencies including within subdirectory
C_SRCS = $(wildcard *.c) $(wildcard */*.c)

# compiled object files
C_OBJS = ${C_SRCS:.c=.o}

all: $(TARGET)

$(TARGET): $(C_OBJS)
	@echo "Compiling & simulating eeprom_test program ..."
	$(CC) $(C_OBJS) -o $(TARGET) $(CFLAGS)
	./$(TARGET)

clean:
	rm -rf $(TARGET) *.o */*.o
