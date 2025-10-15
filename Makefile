
CC = gcc
CFLAGS = -Wall -Wextra -g -O0 -fopenmp -Iinclude
LDFLAGS = -fopenmp -lm

SRC_DIR = src
OBJ_DIR = build
BIN_DIR = .


# Detect OS and set executable extension
ifeq ($(OS),Windows_NT)
    EXE = .exe
    MKDIR = if not exist $1 mkdir $1
    RM = del /Q $1 2>nul || true
    RMDIR = rmdir /S /Q $1 2>nul || true
else
    EXE =
    MKDIR = mkdir -p $1
    RM = rm -f $1
    RMDIR = rm -rf $1
endif

TARGET = $(BIN_DIR)/newton$(EXE)

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Default target
all: $(TARGET)

# Link final executable
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create directories if missing
$(OBJ_DIR):
	$(call MKDIR,$(OBJ_DIR))

$(BIN_DIR):
	$(call MKDIR,$(BIN_DIR))


clean:
ifeq ($(OS),Windows_NT)
	del /Q $(OBJ_DIR)\*.o 2>nul
	rmdir /S /Q $(OBJ_DIR) 2>nul
	del /Q $(BIN_DIR)\*.exe 2>nul
else
	rm -f $(OBJ_DIR)/*.o
	rm -rf $(OBJ_DIR)
	rm -f $(BIN_DIR)/newton
endif


frm:
	rm newton_convergence_x*.ppm
	rm newton_attractors_x*.ppm
