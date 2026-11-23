# Mutex vs. Semaphore

> Understanding synchronization primitives, mutual exclusion, resource management, and thread coordination in concurrent systems.

## Table of Contents

- [Introduction](#introduction)
- [Mutex (Mutual Exclusion)](#mutex-mutual-exclusion)
  - [How It Works](#how-it-works)
  - [Ownership](#ownership)
  - [Mutex in C](#mutex-in-c)
  - [Priority Inversion](#priority-inversion)
- [Semaphore](#semaphore)
  - [Binary Semaphore](#binary-semaphore)
  - [Counting Semaphore](#counting-semaphore)
  - [Semaphore in C](#semaphore-in-c)
- [Mutex vs. Semaphore](#mutex-vs-semaphore)
- [Practical Use Cases](#practical-use-cases)
- [Common Mistakes](#common-mistakes)
- [Conclusion](#conclusion)
- [References](#references)

---

## Introduction

In concurrent programming, multiple threads often execute simultaneously and attempt to access shared resources such as memory locations, files, data structures, or hardware components. When these threads read and modify shared data without proper coordination, race conditions can occur, leading to inconsistent states, unexpected program behavior, and bugs that are notoriously difficult to reproduce. To prevent these issues, operating systems and programming environments provide synchronization primitives designed to regulate concurrent execution and safely manage access to shared resources.

Among these synchronization primitives, two of the most fundamental mechanisms are **Mutex (Mutual Exclusion)** and **Semaphore**. Although both are widely used in concurrent programming to manage resource access, they serve distinct purposes and solve different synchronization problems:

- **Mutex:** Provides exclusive access to a shared resource, ensuring that only a single thread can own the lock and access the critical section at any given time.
- **Semaphore:** Controls access to one or more instances of a resource through an internal counter that represents the number of available permits.

Understanding the core differences between these two mechanisms is essential when designing reliable concurrent applications and robust operating system components.

---

## Mutex (Mutual Exclusion)

A mutex is a synchronization primitive designed to ensure that only one thread at a time can access a protected critical section. In concurrent programming, shared resources or variables are frequently accessed by multiple threads, and if they are modified simultaneously without proper coordination, race conditions can easily occur. A mutex prevents this issue by introducing a strict locking mechanism that enforces exclusive access.

When a thread intends to access a protected resource, it must first acquire the mutex lock. If the mutex is already held by another thread, the requesting thread is blocked and must wait until the lock becomes available. Once the owning thread completes its operation within the critical section, it releases the mutex, thereby allowing one of the waiting threads to acquire it and proceed safely.

### How It Works

A typical mutex operation follows this sequence:

1. Thread 1 attempts to acquire the mutex.
2. The mutex is available, so Thread 1 acquires ownership.
3. Thread 1 enters the critical section and accesses the shared resource.
4. Thread 2 attempts to acquire the same mutex.
5. Since Thread 1 currently owns the mutex, Thread 2 blocks or waits.
6. Thread 1 completes its operation and releases the mutex.
7. Another waiting thread can now acquire the mutex.

This guarantees that the protected critical section is accessed by only one thread at a time.

### Ownership

One of the most important characteristics of a mutex is **ownership**, meaning that a mutex always has a specific owner: the thread that successfully acquired it. Under this model, the exact thread that locks the mutex is solely responsible for unlocking it once it finishes executing its critical section. 

This strict ownership mechanism fundamentally distinguishes a mutex from a semaphore. Rather than functioning merely as a counter or a resource quota, a mutex represents exclusive ownership over a protected section of code or data.

### Mutex in C

The POSIX Threads (Pthreads) API provides mutex functionality through `pthread_mutex_t`.

Example:

```c
#include <pthread.h>
#include <stdio.h>


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int shared_counter = 0;

void *worker(void *arg){

    int threadNumber = (int)(long)arg;
    pthread_mutex_lock(&mutex);
    int initialized_shared_counter = shared_counter;

    for (size_t i = 0; i < 10000; i++){
        shared_counter++;
    }

    printf("Shared counter value from %d to %d for threadNumber: %d\n", initialized_shared_counter, shared_counter, threadNumber);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(void){

    pthread_t t1, t2;

    pthread_create(&t1, NULL, worker, (void*)(long)1);
    pthread_create(&t2, NULL, worker, (void*)(long)2);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&mutex);

    return 0;
}
```

Compile:

```bash
gcc -pthread mutex.c -o mutex
```

Both threads increment the same shared variable. Without synchronization, concurrent modifications could cause lost updates.

By protecting the increment operation with a mutex, only one thread can modify `shared_counter` at a time.

## Demonstrating Race Conditions: Without vs. With Mutex

To understand why mutex synchronization is necessary, consider two threads attempting to increment the same shared counter.

Assume that both threads increment the counter by 10,000. The expected final value is 20,000.

### Without Mutex

When no synchronization mechanism is used, both threads can access and modify the shared counter simultaneously.

**Example output:**

```text
$ ./mutex
Shared counter value from 0 to 10000 for threadNumber: 1
Shared counter value from 7528 to 17528 for threadNumber: 2
```

As shown in the output, Thread 2 starts its operation when the shared counter has already reached 7,528 instead of 10,000.

This happens because the execution of the two threads overlaps. Their operations are not properly synchronized, resulting in an unexpected counter value.

The expected final value is 20,000, but the actual result is 17,528.

This is an example of a **race condition**, where the program's result depends on the timing and interleaving of concurrent operations.

### With Mutex

Now, let's protect the shared counter using a mutex.

Each thread must acquire the mutex before accessing the shared counter and release it after completing its operation.

**Example output:**

```text
$ gcc -pthread mutex.c -o mutex
$ ./mutex
Shared counter value from 0 to 10000 for threadNumber: 1
Shared counter value from 10000 to 20000 for threadNumber: 2
```

This time, Thread 2 waits until Thread 1 releases the mutex.

As a result, each thread executes its critical section exclusively, and the shared counter is updated correctly.

The final value is now 20,000, as expected.

### Comparison

| Without Mutex | With Mutex |
|---|---|
| Concurrent access to shared data | Exclusive access to shared data |
| Race conditions may occur | Protected critical section |
| Unexpected results possible | Consistent result |
| No synchronization guarantee | Mutual exclusion enforced |

**Key Takeaway:** A mutex does not prevent threads from executing concurrently. Instead, it ensures that access to a particular critical section is serialized, preventing multiple threads from modifying the protected shared resource simultaneously.

*Note: Thread scheduling and execution timing are nondeterministic. Therefore, the exact output and intermediate counter values may vary between executions.*

### Priority Inversion

Priority inversion is a scheduling problem that can occur when threads with different priorities compete for a mutex.

Consider three threads:

- High-priority Thread H
- Medium-priority Thread M
- Low-priority Thread L

A possible scenario:

1. Thread L acquires a mutex.
2. Thread H attempts to acquire the same mutex and blocks.
3. Thread M becomes runnable and preempts Thread L.
4. Thread H cannot proceed until Thread L releases the mutex, but Thread L is prevented from running by Thread M.

As a result, a high-priority thread indirectly waits for a lower-priority thread.

Some operating systems and mutex implementations support **Priority Inheritance**, which temporarily raises the priority of the mutex owner to help it finish its critical section and release the mutex.

---

## Semaphore

A semaphore is a synchronization primitive that uses an internal integer counter to control access to shared resources or coordinate execution between threads. Unlike a mutex, a counting semaphore does not enforce ownership semantics, meaning that any thread with access to the semaphore can invoke a `sem_post()` operation to increment its counter, regardless of whether it was the thread that previously called `sem_wait()`.

Because of this counter-based design, a semaphore is particularly useful in scenarios where multiple threads are permitted to concurrently access a limited pool of identical resources.

### How It Works

Consider a parking lot with a maximum capacity of five vehicles.

The parking lot has five available spaces, meaning its initial semaphore value is five.

- Each arriving vehicle must acquire a permit before entering.
- When a vehicle enters, one permit is consumed.
- When all five spaces are occupied, the semaphore counter reaches zero.
- Additional vehicles must wait until a space becomes available.
- When a vehicle leaves, a permit is released.
- A waiting vehicle can then enter.

In this example, the semaphore represents the number of available parking spaces.

This is the fundamental idea behind a counting semaphore.

### Binary Semaphore

A binary semaphore has a counter typically restricted to the values `0` and `1`, making it well-suited for signaling and task synchronization between threads—such as allowing one thread to notify another that a specific operation has completed. 

Despite both mechanisms involving a binary state, a binary semaphore is not equivalent to a mutex. The fundamental distinction lies in ownership: a mutex is strictly owned by the thread that locks it, whereas a binary semaphore lacks ownership semantics entirely.

### Counting Semaphore

A counting semaphore allows its internal counter to represent multiple available permits, where the current value indicates how many threads or operations can acquire a permit immediately without waiting. Access is managed through two primary operations, both of which are guaranteed to be performed atomically by the underlying synchronization mechanism:

- `sem_wait()`: Decrements the semaphore counter; if no permits are currently available (i.e., the counter is zero), the calling thread blocks until another thread releases a permit.
- `sem_post()`: Increments the semaphore counter and, if any threads are currently blocked waiting on the semaphore, wakes one of them up to proceed.

To illustrate how this works in practice, consider a semaphore initialized with a value of `3`:

| Operation | Counter | Behavior |
|---|---:|---|
| Initial state | 3 | Three available permits |
| Thread 1 calls wait | 2 | Acquires a permit |
| Thread 2 calls wait | 1 | Acquires a permit |
| Thread 3 calls wait | 0 | Acquires a permit |
| Thread 4 calls wait | 0 | Blocks |
| Another thread calls post | 1 | Releases a permit |
| Thread 4 proceeds | 0 | Acquires the available permit |

Through this atomic counter-and-blocking cycle, the mechanism guarantees that no more than three threads can hold permits simultaneously, provided that every permit acquisition is properly paired with a corresponding release.

### Semaphore in C

The POSIX semaphore API provides semaphore functionality through `sem_t`.

Example:

```c
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_CONCURRENT_ACCESS 5

sem_t semaphore;

void *worker(void *arg)
{
    int id = *(int *)arg;

    sem_wait(&semaphore);

    printf("Thread %d entered the resource pool\n", id);

    sleep(3);

    printf("Thread %d released its permit\n", id);

    sem_post(&semaphore);

    return NULL;
}

int main(void)
{
    pthread_t threads[10];
    int thread_ids[10];

    sem_init(&semaphore, 0, MAX_CONCURRENT_ACCESS);

    for (int i = 0; i < 10; i++)
    {
        thread_ids[i] = i + 1;

        pthread_create(
            &threads[i],
            NULL,
            worker,
            &thread_ids[i]
        );
    }

    for (int i = 0; i < 10; i++)
    {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&semaphore);

    return 0;
}
```

Compile:

```bash
gcc -pthread semaphore.c -o semaphore
```

In this example:

- Ten threads are created.
- The semaphore is initialized with five permits.
- At most five threads can hold permits simultaneously.
- Additional threads wait until an existing thread releases its permit.
- `sem_post()` makes a permit available again.

The second argument of `sem_init()` is `pshared`.

When set to `0`, the semaphore is shared between threads of the same process. When set to a nonzero value, it is intended for interprocess sharing and must reside in shared memory.

---

## Mutex vs. Semaphore

Although mutexes and semaphores are both synchronization primitives, their underlying semantics and intended use cases differ.

| Characteristic | Mutex | Semaphore |
|---|---|---|
| Primary purpose | Mutual exclusion | Resource counting and signaling |
| Ownership | Yes | No |
| Counter | Binary lock state | Integer counter |
| Concurrent access | One owner | Up to N permits |
| Unlock / release | Owner releases | Any authorized thread can post |
| Blocking | When already locked | When counter reaches zero |
| Typical use | Protecting shared data | Managing limited resources |
| Priority inheritance | Supported by some implementations | Not generally an equivalent ownership mechanism |

Please review the comprasion picture below about mutex and semaphore.

![Mutex vs. Counting Semaphore](assets/mutex-and-semaphore.jpeg)

### Key Differences

**1. Ownership**

A mutex enforces a strict ownership model, meaning that the specific thread that acquires the lock is solely responsible for releasing it upon exiting the critical section. In contrast, a semaphore has no concept of ownership, allowing any thread with access to the semaphore to increment its counter using `sem_post()` regardless of which thread originally performed the wait operation.

**2. Resource Management**

While a mutex is specifically designed to protect a shared critical section from concurrent access by enforcing exclusivity, a counting semaphore is tailored to manage access to a finite pool of identical resources or regulate a limited number of concurrent operations.

**3. Synchronization**

Mutexes primarily focus on providing mutual exclusion to prevent race conditions on shared data. Semaphores, however, extend beyond simple locking and are frequently used for cross-thread signaling, such as allowing a producer thread to notify a consumer thread that new data has become available for processing.

**4. Concurrency Limits**

A mutex restricts concurrency strictly to a single thread, allowing only one thread to own the protected lock at any given moment. Conversely, a counting semaphore initialized with `N` permits provides controlled concurrency by allowing up to `N` threads to acquire permits and execute simultaneously before subsequent threads are forced to block.

---

## Practical Use Cases

### When to Use a Mutex

Mutexes are appropriate when multiple threads need to access or modify shared data, but those operations must be performed exclusively.

Common examples:

- Protecting shared variables and data structures.
- Synchronizing access to shared configuration.
- Protecting critical sections.
- Preventing race conditions in concurrent applications.

### When to Use a Semaphore

Semaphores are useful when the number of concurrent operations must be limited or when threads need to coordinate execution.

Common examples:

- Limiting concurrent database connections.
- Managing a fixed-size resource pool.
- Controlling the number of concurrent file operations.
- Producer-consumer synchronization.
- Signaling task completion between threads.

---

## Common Mistakes

- **Using a semaphore instead of a mutex without considering ownership:** A semaphore does not guarantee that the thread releasing a permit is the one that acquired it.
- **Forgetting to release a mutex:** This can leave other threads blocked indefinitely.
- **Incorrect semaphore accounting:** Calling `sem_post()` more times than intended can increase the available permit count beyond the resource capacity.
- **Assuming a semaphore protects shared data:** A counting semaphore initialized with N does not provide exclusive access when N is greater than one.
- **Ignoring deadlocks:** Incorrect lock ordering and synchronization logic can cause threads to wait indefinitely.
- **Assuming synchronization guarantees fairness:** Neither primitive should automatically be assumed to provide FIFO scheduling or fairness.

---

## Conclusion

Mutexes and semaphores are fundamental building blocks of concurrent programming. Although both primitives can block threads and coordinate execution flow, they are designed for fundamentally different use cases: a mutex is the right choice when a shared resource or critical section must be accessed exclusively by a single thread at a time, whereas a semaphore is better suited for managing access to a limited pool of resources, bounding concurrency, or signaling events between threads.

In short, the core distinction comes down to their underlying abstractions:

**Mutex = Ownership and Mutual Exclusion**

**Semaphore = Counter, Permits and Signaling**

Selecting the right synchronization mechanism for each scenario is critical to building reliable, predictable, and efficient concurrent systems.

---

## References

- [The Open Group — pthread_mutex_lock()](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_mutex_lock.html)
- [The Open Group — sem_wait()](https://pubs.opengroup.org/onlinepubs/9799919799/functions/sem_wait.html)
- [The Open Group — sem_post()](https://pubs.opengroup.org/onlinepubs/9799919799/functions/sem_post.html)
- [Linux man-pages — pthread_mutex(3)](https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3.html)
- [Linux man-pages — sem_overview(7)](https://man7.org/linux/man-pages/man7/sem_overview.7.html)
- [Linux man-pages — sem_init(3)](https://man7.org/linux/man-pages/man3/sem_init.3.html)