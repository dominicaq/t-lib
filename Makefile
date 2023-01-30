CC = gcc
CCFLAGS = -Wall -Wextra -Werror -O2
TARGET = sshell

all: $(TARGET)

%: %.c
	$(CC) $(CCFLAGS) -o $@ $@.c

.PHONY: br
br: $(TARGET) run

.PHONY: brv
brv: $(TARGET) val

.PHONY: val
val:
	valgrind --track-origins=yes --leak-check=full ./$(TARGET)

.PHONY: run
run:
	./$(TARGET)

.PHONY: clean
clean:
	rm $(TARGET)
