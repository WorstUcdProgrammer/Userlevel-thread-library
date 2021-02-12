# Compiler
CC := gcc

# Flags
CFLAGS := -Wall -Wextra -Werror

# Target library
lib := libuthread.a

# object files
objs := queue.o uthread.o context.o preempt.o

all: $(lib)

deps := $(patsubset %.o, %.d, $(objs))
-include $(deps)

$(lib): $(objs)
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(lib) $(objs) $(deps)