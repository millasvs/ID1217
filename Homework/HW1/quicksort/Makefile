  # the compiler:
  CC = gcc

  # compiler flags:
  #  -g    adds debugging information to the executable file
  #  -Wall turns on most, but not all, compiler warnings
  CFLAGS  = -g -Wall
  LDFLAGS=  -lpthread -lm

  # the build target executable
  TARGET = quickSort

  all: $(TARGET)

  $(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(LDFLAGS)

  clean:
	$(RM) $(TARGET)

  run: $(TARGET) 
	./$(TARGET)
  
