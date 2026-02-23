# Name of the final executable
TARGET = ../canny

# Compiler and flags
CXX = g++                     
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2

# List of source files (.cpp)
SRCS = main.cpp Image.cpp

# List of object files (.o) (convert .cpp to .o)
OBJS = $(SRCS:.cpp=.o)

# Default rule
all: $(TARGET)

# Rule to create the executable from object files
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

# Rule to compile .cpp files into .o object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up generated files (object files and executable)
clean:
	rm -f $(OBJS) $(TARGET)

# Phony rules (these are not actual files, just make targets)
.PHONY: all clean
