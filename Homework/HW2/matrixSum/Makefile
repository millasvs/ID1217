  # the compiler:
  CC = gcc

  # compiler flags:
  #  -g    adds debugging information to the executable file
  #  -Wall turns on most, but not all, compiler warnings
  CFLAGS  = -g -Wall -fopenmp
  LDFLAGS = #-DDEBUG

  # the build target executable
  TARGET = matrixSum-openmp

  all: $(TARGET)

  $(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(LDFLAGS)

  clean:
	$(RM) $(TARGET)

  run: $(TARGET) 
	./$(TARGET)
  
