CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread
TARGET = kramerscript

# Default target
all: $(TARGET)

# Compile
$(TARGET): kramerscript.c
	$(CC) $(CFLAGS) -o $(TARGET) kramerscript.c

# Clean build artifacts
clean:
	rm -f $(TARGET) *.o

# Install to /usr/local/bin
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Test
test: $(TARGET)
	./$(TARGET) server.kramer &
	sleep 1
	curl http://127.0.0.1:413/ || true
	pkill -f "kramerscript server" || true

.PHONY: all clean install test
