# Compiler
CC := gcc

# Flags
CFLAGS := -Wall -Wextra -Werror -c -o

# Queue
QUEUE := queue

# Target library
lib := libuthread.a

all: $(lib)

$(lib): $(QUEUE).o
	ar rcs $@ $<

$(QUEUE).o: $(QUEUE).c $(QUEUE).h
	$(CC) $(CFLAGS) $@ $<

clean:
	rm -f $(lib) $(QUEUE).o
