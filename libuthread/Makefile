# Target library
lib 	:= libuthread.a
targets := queue#, sem, uthread
objs 	:= queue.o, sem.o, uthread.o

# GCC options
CC      := gcc
CFLAGS  := -MMD # -Wall -Wextra -Werror
CFLAGS  += -g

# Verbose mode
ifneq ($(V),1)
Q = @
endif

# Default rule
all: $(targets) #$(lib)

# Include dependencies
deps := $(patsubst %.o, %.d, $(objs))
-include $(deps)

# Generic rule for compiling objects
%.o: %.c
	@echo "CC	$@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

# Cleaning rule TODO: Deps not cleaning
clean:
	@echo "clean"
	$(Q)rm -rf $(objs) $(deps)