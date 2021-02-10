# Compiler
CC := gcc

# Flags
CFLAGS := -Wall -Wextra -Werror

# Queue
QUEUE := queue

# UThread
UTHREAD := uthread

# Target library
lib := libuthread.a

all: $(lib)

$(lib): $(QUEUE).o $(UTHREAD).o context.o
	ar rcs $@ $^

$(QUEUE).o: $(QUEUE).c $(QUEUE).h
	$(CC) $(CFLAGS) -o $@ -c $<

$(UTHREAD).o : $(UTHREAD).c $(UTHREAD).h context.o
	$(CC) $(CFLAGS) -o $@ -c $<

context.o : context.c private.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(lib) $(QUEUE).o $(UTHREAD).o context.o core
