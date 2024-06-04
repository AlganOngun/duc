# Compiler and compiler flags
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu2x
DEBUG_FLAGS = -g

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source files
SRC_FILES = $(SRC_DIR)/error.c $(SRC_DIR)/lexer.c $(SRC_DIR)/ast.c $(SRC_DIR)/parser.c $(SRC_DIR)/interpreter.c $(SRC_DIR)/scope_table.c $(SRC_DIR)/symbol_table.c $(SRC_DIR)/command.c $(SRC_DIR)/command_funcs.c $(SRC_DIR)/main.c

# Object files
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

# Executables
EXEC = interpreter
DEBUG_EXEC = interpreter_debug

# Default rule
all: $(EXEC)
	./rere.py replay test.list

# Rule to build the final executable
$(EXEC): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to build the debug executable
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(DEBUG_EXEC)

$(DEBUG_EXEC): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile and run
run: $(EXEC)
	./$(EXEC) test.duc

# Clean up build files
clean:
	rm -rf $(BUILD_DIR) $(EXEC) $(DEBUG_EXEC)

# Phony targets
.PHONY: all clean test run debug
