# Target library
lib  := libuthread.a
objs := $(patsubst %.c,%.o,$(wildcard *.c))

# GCC options
CC     := gcc
CFLAGS := -MMD -Wall
#CFLAGS += -Wextra -Werror

# Static library options
LIBFLAGS := ar rcs
# Verbose mode
ifneq ($(V),1)
Q = @
endif

# Default rule
all: $(lib)

# Include dependencies
deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

# Generic rule for compiling static library
$(lib): $(objs)
	@echo "CC $@"
	$(Q)$(LIBFLAGS) $(lib) $(objs)

# Generic compile rule
%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

# Cleaning rule
clean:
	@echo "clean"
	$(Q)rm -rf $(lib) $(objs) $(deps)