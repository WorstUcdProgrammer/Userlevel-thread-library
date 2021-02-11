#include <stdio.h>
#include <stdlib.h>
#include <uthread.h>

int thread2(void)
{
	printf("thread2\n");
	return 0;
}

int thread1(void)
{
	uthread_join(uthread_create(thread2), NULL);
	printf("thread1\n");
	return 0;
}

int main(void)
{
	uthread_start(0);
	uthread_join(uthread_create(thread1), NULL);
	printf("main thread\n");
	uthread_stop();
	return 0;
}
