			+--------------------+
			| PINTOS             |
			| PROJECT 1: THREADS |
			| DESIGN DOCUMENT    |
			+--------------------+
				   
---- GROUP ----

>> Fill in the names and email addresses of your group members.

Dalton Burke
Christina Tsui

---- PRELIMINARIES ----

>> If you have any preliminary comments on your submission, notes for the
>> TAs, or extra credit, please give them here.

Running low on time, MLFQS design area is sparse, comments in the code for that
particular part of the assignment are also somewhat sparse  



##### (old) Output:
```
pass tests/threads/alarm-single
pass tests/threads/alarm-multiple
pass tests/threads/alarm-simultaneous
pass tests/threads/alarm-priority
pass tests/threads/alarm-zero
pass tests/threads/alarm-negative
pass tests/threads/priority-change
pass tests/threads/priority-donate-one
pass tests/threads/priority-donate-multiple
pass tests/threads/priority-donate-multiple2
pass tests/threads/priority-donate-nest
pass tests/threads/priority-donate-sema
pass tests/threads/priority-donate-lower
pass tests/threads/priority-fifo
pass tests/threads/priority-preempt
pass tests/threads/priority-sema
pass tests/threads/priority-condvar
pass tests/threads/priority-donate-chain
FAIL tests/threads/mlfqs-load-1
FAIL tests/threads/mlfqs-load-60
FAIL tests/threads/mlfqs-load-avg
FAIL tests/threads/mlfqs-recent-1
pass tests/threads/mlfqs-fair-2
pass tests/threads/mlfqs-fair-20
FAIL tests/threads/mlfqs-nice-2
FAIL tests/threads/mlfqs-nice-10
FAIL tests/threads/mlfqs-block
7 of 27 tests failed.
```

			     ALARM CLOCK
			     ===========

---- DATA STRUCTURES ----

>> A1: Copy here the declaration of each new or changed `struct' or
>> `struct' member, global or static variable, `typedef', or
>> enumeration.  Identify the purpose of each in 25 words or less.

#### `thread.h`

`thread.wake_time: int64_t`  
Time in ticks from the start of the computer that the thread should be woken up.

#### `timer.c`

`sleep_list: static struct list`  
List of sleeping threads.


---- ALGORITHMS ----

>> A2: Briefly describe what happens in a call to timer_sleep(),
>> including the effects of the timer interrupt handler.

call: `timer_sleep(int64_t sleep_ticks)`  
Computes `sleep_ticks + ticks` to find what `ticks` will be when it
is time for the thread to be unblocked. That is stored
in `wake_time` then the current thread is moved to `sleep_list`, and
blocks the thread. The `timer_interrupt` handler checks if the `wake_time`
of the thread at the front of the list has passed, if it has then the thread
is removed from `sleep_list` and is unblocked.


>> A3: What steps are taken to minimize the amount of time spent in
>> the timer interrupt handler?

Only the front of `sleep_list` is checked on each interrupt (O(1)), since it is
sorted by ascending `wake_time`. This is a trade off between having the list
take O(n) time to add a thread in it's correct place (versus O(1) to just slap
it on the front of the list), or having the interrupt handler take O(n) time to
see if a thread needs to be unblocked. To minimize interrupt handler time, of
course we choose to add threads in their correct position ahead of time.


---- SYNCHRONIZATION ----

>> A4: How are race conditions avoided when multiple threads call
>> timer_sleep() simultaneously?

The only concern with multiple threads calling timer_sleep simultaneously
is the potential for two (or more) threads to be attempting to modify `sleep_list`
at the same time. This isn't an issue, as the `list` operations are atomic.

>> A5: How are race conditions avoided when a timer interrupt occurs
>> during a call to timer_sleep()?

Timer interrupts occuring during a `timer_sleep` call are held off, as interrupts
are disabled while the function is being executed. This has to be done, since
a timer interrupt occuring during `timer_sleep` can cause issues with the value
of `ticks`, as well as potentially unblocking a thread before it has even been
blocked (suppose `timer_interrupt` occurs right after the thread has been put 
in `sleep_list`, but before `thread_block` has been called).

---- RATIONALE ----

>> A6: Why did you choose this design?  In what ways is it superior to
>> another design you considered?

This problem is not very complex, the only real room for optimization
was in the choice to have `sleep_list` sorted by ascending `wake_time`.
While it was possible to choose something even more efficient, a la min-heap,
the `sleep_list` will not be growing very large in pintos. There could
certainly be a case for this design choice in a much bigger operating system,
however the ease of implementation was also a consideration.


			 PRIORITY SCHEDULING
			 ===================

---- DATA STRUCTURES ----

>> B1: Copy here the declaration of each new or changed `struct' or
>> `struct' member, global or static variable, `typedef', or
>> enumeration.  Identify the purpose of each in 25 words or less.

#### `thread.h`  
`int eff_priority`  
Member of `thread`, the 'effective' priority of a thread, maybe different
than what it has been assigned due to priority donations.

`struct list locks`  
Member of `thread`, a list of locks which are owned by the `thread`

`struct lock* blocking_lock`  
Member of `thread`, a lock which is needed, but cannot be acquired due to
ownership by another thread of lower priority.

#### `synch.h`  
`struct list_elem elem`
Member of `lock`, used to track `lock` position in a list
(like `thread.locks`).

`int highest priority`  
Member of `lock`, used to track the highest priority thread that has
donated priority to the current owner.


>> B2: Explain the data structure used to track priority donation.
There was no explicit data structure to keep track of donation, however
`thread.locks`, `thread.blocking_lock`, and `lock.elem` together
were used to create a linked list of priority donation chains. This was used
in a tail recursive loop to traverse and propogate priority donation.
This loop:  
```
while(l &&
      l->highest_priority < t->eff_priority &&
      depth++ < MAX_DEPTH)
  {
    l->highest_priority = t->eff_priority;
    priority_donate(l->holder);
    l = l->holder->blocking_lock;
  }
```
Terminates when no more locks are blocking, `l`'s highest priority thread
has become , or MAX_DEPTH (10 donation chain) has been reached.



---- ALGORITHMS ----

>> B3: How do you ensure that the highest priority thread waiting for
>> a lock, semaphore, or condition variable wakes up first?  

After a `lock_release`, `sema_up` is called and wakes the thread of
highest priority (as the `waiters` list was sorted in descending order
of priority when a thread is added during `sema_down`).


>> B4: Describe the sequence of events when a call to lock_acquire()
>> causes a priority donation.  How is nested donation handled?  

This was described in B2, pasted here for your convenience:  
There was no explicit data structure to keep track of donation, however
`thread.locks`, `thread.blocking_lock`, and `lock.elem` together
were used to create a linked list of priority donation chains. This was used
in a tail recursive loop to traverse and propogate priority donation.
This loop:  
```
while(l &&
      l->highest_priority < t->eff_priority &&
      depth++ < MAX_DEPTH)
  {
    l->highest_priority = t->eff_priority;
    priority_donate(l->holder);
    l = l->holder->blocking_lock;
  }
```
Terminates when no more locks are blocking, `l`'s highest priority thread
has become , or MAX_DEPTH (10 donation chain) has been reached.


>> B5: Describe the sequence of events when lock_release() is called
>> on a lock that a higher-priority thread is waiting for.

As described in B3:  
After a `lock_release`, `sema_up` is called and wakes the thread of
highest priority (as the `waiters` list was sorted in descending order
of priority when a thread is added during `sema_down`).

---- SYNCHRONIZATION ----

>> B6: Describe a potential race in thread_set_priority() and explain
>> how your implementation avoids it.  Can you use a lock to avoid
>> this race?  

While setting the priority, another thread with higher priority may preempt
it before it gets a chance to be assigned a higher priority. To fix this,
interrupts were disabled for this function.


---- RATIONALE ----

>> B7: Why did you choose this design?  In what ways is it superior to
>> another design you considered?

A more traditional recursive solution was considered for the implementation
of the priority donation chain, though ultimately the tail recursion solution
was found to be quite elegant, well behaved, and less costly for a thread than
having an internal list of priority donors.

			  ADVANCED SCHEDULER
			  ==================

---- DATA STRUCTURES ----

>> C1: Copy here the declaration of each new or changed `struct' or
>> `struct' member, global or static variable, `typedef', or
>> enumeration.  Identify the purpose of each in 25 words or less.

#### `fixed_point.h`
`fixed_pt`  
Integer typedef representing fixed point numbers. Arithmetic operations
managed by macros defined in fixed_point.h, shif value of 14.

#### `thread.h`  
`r_cpu`


---- ALGORITHMS ----

>> C2: Suppose threads A, B, and C have nice values 0, 1, and 2.  Each
>> has a recent_cpu value of 0.  Fill in the table below showing the
>> scheduling decision and the priority and recent_cpu values for each
>> thread after each given number of timer ticks:

timer  recent_cpu    priority   thread
ticks   A   B   C   A   B   C   to run
-----  --  --  --  --  --  --   ------
 0
 4
 8
12
16
20
24
28
32
36

>> C3: Did any ambiguities in the scheduler specification make values
>> in the table uncertain?  If so, what rule did you use to resolve
>> them?  Does this match the behavior of your scheduler?

>> C4: How is the way you divided the cost of scheduling between code
>> inside and outside interrupt context likely to affect performance?

---- RATIONALE ----

>> C5: Briefly critique your design, pointing out advantages and
>> disadvantages in your design choices.  If you were to have extra
>> time to work on this part of the project, how might you choose to
>> refine or improve your design?

			   SURVEY QUESTIONS
			   ================

Answering these questions is optional, but it will help us improve the
course in future quarters.  Feel free to tell us anything you
want--these questions are just to spur your thoughts.  You may also
choose to respond anonymously in the course evaluations at the end of
the quarter.

>> In your opinion, was this assignment, or any one of the three problems
>> in it, too easy or too hard?  Did it take too long or too little time?

>> Did you find that working on a particular part of the assignment gave
>> you greater insight into some aspect of OS design?

>> Is there some particular fact or hint we should give students in
>> future quarters to help them solve the problems?  Conversely, did you
>> find any of our guidance to be misleading?

>> Do you have any suggestions for the TAs to more effectively assist
>> students, either for future quarters or the remaining projects?

>> Any other comments?
