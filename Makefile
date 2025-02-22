# Pick up the first (and only) .cpp file in src, regardless of its name.
SRC := $(firstword $(wildcard src/*.cpp))
BUILD_DIR := build
BIN := main

CXX := g++
CXXFLAGS := -std=c++17 -Isrc
LDFLAGS := -lsfml-graphics -lsfml-window -lsfml-system

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LIB_PATHS := -L/opt/homebrew/lib
else ifeq ($(UNAME_S),Linux)
    LIB_PATHS := -L/usr/lib/aarch64-linux-gnu -L/usr/lib/x86_64-linux-gnu
endif

all: $(BIN)

$(BIN): $(BUILD_DIR)/main.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIB_PATHS)

$(BUILD_DIR)/main.o: $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC) -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN)

.PHONY: run
run: $(BIN)
	./$(BIN)
