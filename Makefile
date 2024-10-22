# Makefile

# Compiler & Flags
CC=gcc
CFLAGS= -Iinclude -D_GNU_SOURCE -std=c99 -Wall -g

# Directories
SRC_DIR = src
INC_DIR = includes
OBJ_DIR = build
INPUT_DIR = input
OUTPUT_DIR = output

# Compilation Files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))
TARGET = autograder

all: $(TARGET) $(INPUT_DIR) $(OUTPUT_DIR) solutions

# Build target
$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $(TARGET)

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure the objdir is created
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Ensure the input dir is created
$(INPUT_DIR):
	mkdir -p $(INPUT_DIR)

# Ensure the output dir is created
$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)



SOURCE_FILE=soln.c
SOL_DIR=./solutions
LETTERS=a b c d e f g h i j k l k l m n o p q r s t
BINARIES=$(addsuffix _sol, $(addprefix $(SOL_DIR)/, $(LETTERS)))

# Compile soln.c into binaries named a_sol, b_sol, c_sol, etc.
# Target to generate all solution binaries
solutions: $(BINARIES)
$(SOL_DIR)/%_sol: $(SOURCE_FILE)
	mkdir -p $(SOL_DIR)
	$(CC) $(CFLAGS) -o $@ $<

kill:
	pkill -9 ".*_sol"

# Clean the build
clean:
	rm -f autograder
	rm -f ./build/*.o
	rm -f ./output/*.*
	rm -f ./input/*.*
	rm -f ./solutions/*

clean_solutions:
	rm -f ./solutions/*

.PHONY: all clean
