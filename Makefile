SRC_DIR := src
BUILD_DIR := build
BIN_DIR := .

SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC))
BIN := $(BIN_DIR)/main

CXX := g++
CXXFLAGS := -std=c++17 -I$(SRC_DIR)
LDFLAGS := -lsfml-graphics -lsfml-window -lsfml-system

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LIB_PATHS := -L/opt/homebrew/lib
else ifeq ($(UNAME_S),Linux)
    LIB_PATHS := -L/usr/lib/aarch64-linux-gnu -L/usr/lib/x86_64-linux-gnu
endif

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIB_PATHS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN)
