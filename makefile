# Makefile

# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g

# Target executable
TARGET = shell

# Source files
SRCS = Shell.cpp Commands.cpp commandUtils.cpp

# Header files
HDRS = Shell.h Commands.h commandUtils.h

# Object files
OBJS = Shell.o Commands.o commandUtils.o

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compile source files into object files
Shell.o: Shell.cpp Shell.h Commands.h commandUtils.h
	$(CXX) $(CXXFLAGS) -c Shell.cpp

Commands.o: Commands.cpp Commands.h
	$(CXX) $(CXXFLAGS) -c Commands.cpp

commandUtils.o: commandUtils.cpp commandUtils.h
	$(CXX) $(CXXFLAGS) -c commandUtils.cpp

# Clean target to remove compiled files
clean:
	rm -f $(TARGET) $(OBJS)
