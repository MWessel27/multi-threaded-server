• A verbal description of the structure of your threadpool implementation, including a
list of any design decisions you made.

Starting with the threadpool structure, it is composed of a thread count, a queue count
to hold the number of threads in the queue, a shutdown flag, a mutex, conditional variable,
a thread pointer, and a pointer to the head, and end of the linked list.

I also defined a structure call node worker which was composed of a function that it is 
holding on to execute, a node_worker pointer to the next node worker in the linked list,
and the function arguments to pass into the function it will be calling.

Inside of the create_threadpool function, I first allocate memory for the the threads
pointer in the pool with the integer value passed in, then set up the threadpool structure 
by initializing its variables like thread count to the integer passed in, head and end of 
queue to NULL, queue count to 0, and shutdown flag. Then initialize the mutex and conditional
with the pthread API. I then create the threads with the pthread_create API function
and return the threadpool.

In dispatch, I allocate a node worker, initialize the values in the node worker structure
and set the next value in the linked list for this object to NULL. Then lock the mutex,
update the struct linked list entries for head and end while adding the node to the queue,
setting the conditional signal with the pthread API, incrementing the queue count, and
releasing the mutex.

In destroy threadpool, first lock the mutex, set the shutdown flag, broadcast and wake up
all threads with the pthread API call, release the mutex, wait for all the threads to exit
with the pthread join API, and then free the resources of the threadpool.

Finally, the thread main function which every thread executes, allocate a new node worker,
and start the infinite loop. Inside of the infinite loop, lock the mutex, if shutdown is
not set, and queue count is 0, wait for conditional variable with pthread API. If shutdown
is set, release the mutex and break. Then dequeue the task, update the linked list,
release the mutex, do the work of the function and pass in the arguments, then free
resources of threadpool.

• A list of critical sections inside your threadpool code (and why they are critical sections).

The critical sections inside my threadpool code is between when I lock the mutex, and then
release the mutex. This happens in thread_main, dispatch, and destroy_threadpool. The reason
that these sections of code between locking the mutex and releasing it, is that we give
a set of consecutive instructions which we always want to execute without switching context.
In this implementation, we use mutex locks to make sure that when we update the linked list,
and threadpool, that nothing interrupts our commands so that we do not switch context and 
continue on another process yielding back inconsistent results and possibly breaking the 
program completely.

• A description of the synchronization inside your threadpool. What condition variables
did you need, where, and why? Under what conditions do threads block, and which thread
is responsible for waking up a blocked thread?

Dispatch handles the main synchronization inside my threadpool, and thread_main also does
a little as well. In dispatch, if there are no available threads in the pool because they
have all been dispatched, it blocks until one becomes available by returning from its
previous dispatch. I used pthread_cond_signal API to signal the threadpool conditional
variable (condvar) to handle this. 

In thread_main, if the threadpool is not shutting down and the queue is empty, I keep
waiting for the conditional variable condvar to be set. I do this because the queue is
empty and it is supposed to still be running because the shutdown variable is not set. I
used pthread API pthread_cond_wait call to do this.

The two situations above are the reasons why we block in the threadpool. In 
detroy_threadpool, we use pthread_cond_broadcast API to wake up all the blocked threads.
This is called when we need to destroy the threadpool.