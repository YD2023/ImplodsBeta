# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Ofast

# Target executable name
TARGET = ImplodsBeta

# Source files
SRC = ImplodsBeta.cpp

# Default rule to build the target
all: $(TARGET)

# Rule to compile the target
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Rule to clean the build directory
clean:
	rm -f $(TARGET)

# Rule to run the program
run: $(TARGET)
	./$(TARGET)
