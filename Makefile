CC = gcc
TARGET_EXEC = main
CFLAGS = -Wall -std=c11

LIBS = 

.PHONY: all clean

all: $(TARGET_EXEC) run clean

$(TARGET_EXEC): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET_EXEC) $(LIBS)

run: $(TARGET_EXEC)
	./$(TARGET_EXEC)

dry-run: $(TARGET_EXEC)
		clear
		./$(TARGET_EXEC)

clean:
	rm -f $(TARGET_EXEC)