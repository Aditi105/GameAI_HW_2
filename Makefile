# Get all .cpp files in src and determine their corresponding binary names.
SOURCES   := $(wildcard src/*.cpp)
BINARIES  := $(patsubst src/%.cpp, %, $(SOURCES))
BUILD_DIR := build

CXX      := g++
CXXFLAGS := -std=c++17 -Isrc
LDFLAGS  := -lsfml-graphics -lsfml-window -lsfml-system

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LIB_PATHS := -L/opt/homebrew/lib
else ifeq ($(UNAME_S),Linux)
    LIB_PATHS := -L/usr/lib/aarch64-linux-gnu -L/usr/lib/x86_64-linux-gnu
endif

# 'all' target builds every binary.
all: $(BINARIES)

# Link each executable from its corresponding object file.
%: $(BUILD_DIR)/%.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIB_PATHS)

# Compile each .cpp file into an object file in the build directory.
$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BINARIES)

.PHONY: run
# Optional: run a specific binary by specifying its name on the command line, e.g., make run EXE=part1
run: $(EXE)
	./$(EXE)
