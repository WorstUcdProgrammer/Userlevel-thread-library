# Userlevel-thread-library

## Author
Yizeng Zhou, Ruike Qiu

## Summary

This project will build a static user-level thread library `uthread.a`, which
will provide interface and functions to let program achieve multi-threading.

## Implementation

The implementation of this project was seperated into 3 parts:

1. Implement the essential data structure - queue, to store the data.
2. Implement the uthread library functions
3. Implement the preempt functionality.

### Queue

The way that we chose to implement the queue was using another data structure,
node, so that it would be easier to manipulate the data inside the queue. In 
genral, in my node data structure, it contains a void pointer which would be the
address of the data to be stored, and a struct pointer that pointing to the 
next connecting node(ie. inside the queue). We will set the pointer of next 
node to be default "NULL", which will make determing the last node inside 
the queue easier.

We also made a node_create function to make initialization easier to be done, 
and it also make the code look clean.

In the structure queue, We stored the address of the head node and the tail 
node as well as the length of the node. It will be convenient for all
operations. For example, enqueue will mostly require only access to the tail and
dequeue will mostly require only access to the head.

#### Limitation for queue

We think there exist a limitation that we could fix. In order to determine if 
the data that we want to delete exists. It could be done easily by calling the 
queue_iterate with a custom function to find the item. But instead, we chose to 
use the while loop to manually iterate, since at that time, we have not started 
to think about how queue_iterate works.

### Uthread

#### Data Strucutre

We thought for a long time before we started to work on this part since it would
be really efficient to have a good data strcuture for future use. We defined our
data structure to be `struct tcb`. Each one instance of this object will 
represent one individual thread inside the process. The structure will contain 
that 1. the id of itself 2. the environment(registers, etc.) 3. the stack
(top_of_stack) 4. the enum uthread_states(we defined this macro to make the 
code more readable) 5. the return value of current thread 6. the address that 
will receive the return value of current thread 7. the address of the thread 
that was block by current thread(if any)

##### TID

It will be useful to check the TID of the thread such as determine if the thread
is the main thread, which would be not joinable.

##### Environment

It is required to save the current environment since we need to switch back to 
current thread after switching content from one to another.

##### Stack

Same as above, it would be essential to save the place where the current thread 
was executing before switching content.

##### Uthread_states

It was really useful to track the state of the thread, the thing that we made 
really special was that we had another state other than ZOMBIE, which is DELETD.
Because in our implementation, all the resources would not be freed until the 
main thread called uthread_stop(), which in our opinions would be the safest 
point to free the resources.

##### Return value

We decided to have this value to be stored inside the data strcutre was because 
in the case that a thread exited without being collected, it would wait for 
another thread to collect it, so it would be hard to retrace the returned state.

##### Address to receive return value

We decied to include this inside the structure because there might be the case
that a thread would joined another thread that was not finished yet. It would be
convenient to have this return address inside their struct, so that when they 
exited(ie. finished), we can check if the address is NULL or not, if not, we
change the value at the address to be the returned value.

##### Blocked thread

We used this member to save the address of the thread that is waiting for the
thread to finish. So when the thread exit, it could scheduled the blocked thread
to the queue.

#### Global variables

We have five global variables.

##### Uthread_counter

It is the counter to count the number of the threads, which would also be 
convenient to assign TID for new generated threads

##### Queue

This is the waiting queue containing all threads of state WAITING.

##### All_threads

This is the queue that containing all threads no matter what their states are, 
it is helpful to free all the resouces at one at the end of the stop function. 
It is also good to track if there exist a specific thread inside the process.

##### Preempt_flag

We stored the preempt is true or not because if it's not started, we don't need
to call the preempt_stop() inside the uthread_stop()

##### Current_thread

This is the address of current running thread which would be useful to switch 
content to perform call back on nearly all functions

#### Helper functions

##### Free_resources

This function is used to free the context as well as the stack inside the thread
when it finished. And it will set the its blocking thread ready to run(WAITING),
if any. It will also set the current thread to be DELETED, waiting to be finally
freed after uthread_stop()

##### Delete_all

This function will be called inside the uthread_stop which will call the
queue_iterate to perform freeing all resources on any threads inside the
all_threads global variables.

##### Find_thread

This function is used to find if a thread is inside a queue by calling 
queue_iterate

#### Uthread_start

When main thread called this function, we will initialize all the global 
variables and allocating space for them as well as creating a main thread to be
running. We didn't enqueue anything since there is no threads waiting.

#### Uthread_create

We initialized a new thread and enqueue it to the waiting queue and performed
content switch

#### Uthread_join

We divided it into two cases 1. if the thread to join is already dead(ZOMBIE),
collect the return value and continue running current thread. 2. if the thread 
is not finished, it block the current thread, then we set the return address in
the structure to function's parameter.


### Preempt

#### Usage

We chose to disable the preempt whenever inside a function to modify global
varible which might cause some issue and enable it afterwards. We also notice
that sometimes we don't have to call enable since inside the uthread_ctx_switch,
it called bootstrap which will eventually called preempt_enable

## Testing

### Queue

We used professor's example tester and added some our cases to test it.

### Uthread

We built upon professor's code to see if the output sequence is matched.

### Preempt

For this part, we include a test_preempt.c to see if it works. It started
multithreading and created a thread inside the main function. Then it will start
to looping through an empty infinite loop. Inside thread it created, it will 
created another thread. it will then also perform infinite loop to print a
letter. We at last had four threads which are all executing infinite loop, 
three of them are printing different values and the main thread didn't do
anything. The way that we think preempt works was that the output was keep
changing from 'a' to 'b' to 'c' to 'a' ... which indicates it was forcing to 
call yield to other threads.