# Name of the final executable
TARGET = ../canny_cuda

# Compilers
CXX = g++
NVCC = nvcc

# Flags
# Standard C++ flags
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O3

# CUDA flags
# -Xcompiler passes the standard C++ warnings to g++ under the hood
NVCCFLAGS = -std=c++17 -ccbin g++-10 -Xcompiler "-Wall -Wextra"

# List your source files separately based on their extension
CPP_SRCS = ImageLoader.cpp
CU_SRCS = main.cu Canny.cu CudaTimer.cu

# Generate object file names
CPP_OBJS = $(CPP_SRCS:.cpp=.o)
CU_OBJS = $(CU_SRCS:.cu=.o)
OBJS = $(CPP_OBJS) $(CU_OBJS)

# Default rule
all: $(TARGET)

# Rule to create the executable from ALL object files
# CRITICAL: We use $(NVCC) to link, not $(CXX)! 
# This automatically links the required CUDA runtime libraries.
$(TARGET): $(OBJS)
	$(NVCC) $(NVCCFLAGS) -o $@ $^

# Rule to compile standard .cpp files into .o object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to compile CUDA .cu files into .o object files
%.o: %.cu
	$(NVCC) $(NVCCFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean