# Thread Pool

_[I/O Library by Ross Smith](index.html)_

```c++
#include "rs-io/thread-pool.hpp"
namespace RS::IO;
```

## Contents

* TOC
{:toc}

## Class ThreadPool

```c++
class ThreadPool;
```

This class runs an internal thread pool. Jobs are processed by a work stealing
algorithm, and may not be executed in the order in which they were queued.

All member functions, except the constructors and destructor, are async safe
and can be called from any thread. Functions other than `clear()` and the
wait functions can be called from inside an executing job.

```c++
using ThreadPool::callback = std::function<void()>;
using ThreadPool::clock = std::chrono::system_clock;
```

Member types.

```c++
ThreadPool::ThreadPool();
explicit ThreadPool::ThreadPool(int threads);
```

Constructors. `ThreadPool` is not copyable or movable. The number of system
threads is specified to the constructor. If no thread count is specified, or
the count is zero or negative, `std::thread::hardware_concurrency()` will be
used instead.

```c++
ThreadPool::~ThreadPool() noexcept;
```

The destructor calls `clear()` and waits for it to finish.

```c++
void ThreadPool::clear() noexcept;
```

Discards any queued jobs that have not yet been started, and waits for all
currently executing jobs to finish before returning. New jobs can be queued
after it returns. It is safe for one thread to call `insert()` while another
is calling `clear()`, but the newly inserted job may or may not be discarded
without being executed.

```c++
template <typename F> void ThreadPool::insert(F&& f);
```

Queues a job for execution. `F` must be a function-like type callable with no
arguments. Behaviour is undefined if the callback is a null function pointer
or `std::function`, or if a callback throws an exception.

```c++
bool ThreadPool::poll();
```

True when there are no jobs queued or executing.

```c++
int ThreadPool::threads() const noexcept;
```

Returns the thread count. This is always positive, and always constant for the
lifetime of the `ThreadPool` object.

```c++
void ThreadPool::wait() noexcept;
template <typename R, typename P>
    bool ThreadPool::wait_for(std::chrono::duration<R, P> t) noexcept;
bool ThreadPool::wait_until(clock::time_point t) noexcept;
```

Block until there are no jobs queued or executing, or the timeout expires.
