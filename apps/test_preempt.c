#include <stdio.h>
#include <stdlib.h>
#include <uthread.h>

int thread3(void)
{
	while (1) {
		printf("a\n");
	}
	return 0;
}

int thread2(void)
{
	uthread_create(thread3);
	while (1) {
		printf("b\n");
	}
	return 0;
}

int thread1(void)
{
	uthread_create(thread2);
	while (1) {
		printf("c\n");
	}
	return 0;
}

int main(void)
{
	uthread_start(1);
	uthread_create(thread1);
	while (uthread_stop()) {
		// do nothing
	}
	return 0;
}
