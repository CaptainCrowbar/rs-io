# Thread Pool

```c++
#include "rs-core/thread-pool.hpp"
namespace RS::Channel;
```

## Contents

* TOC
{:toc}

## Class ThreadPool

```c++
class ThreadPool: public Wait;
    using ThreadPool::callback = std::function<void()>;
    using ThreadPool::clock = std::chrono::system_clock;
    ThreadPool::ThreadPool();
    explicit ThreadPool::ThreadPool(size_t threads);
    ThreadPool::~ThreadPool() noexcept;
    void ThreadPool::clear() noexcept;
    void ThreadPool::insert(const callback& call);
    void ThreadPool::insert(callback&& call);
    bool ThreadPool::poll();
    size_t ThreadPool::threads() const noexcept;
    void ThreadPool::wait() noexcept;
    template <typename R, typename P>
        bool ThreadPool::wait_for(std::chrono::duration<R, P> t) noexcept;
    bool ThreadPool::wait_until(clock::time_point t) noexcept;
```

This class runs an internal thread pool, with the number of system threads
specified to the constructor. If no thread count is specified, or the count
is zero, `std::thread::hardware_concurrency()` will be used instead. The
`size()` function returns the thread count (always positive, and always
constant for the lifetime of the `ThreadPool` object).

The `insert()` function queues a job for execution. Behaviour is undefined if
a callback throws an exception. Jobs are processed by a work stealing
algorithm, and may not be executed in the order in which they were queued.

The `clear()` function discards any queued jobs that have not yet been
started, and waits for all currently executing jobs to finish before
returning. New jobs can be queued after it returns. The destructor calls
`clear()` and waits for it to finish.

The `poll()` function returns true when there are no queued or executing jobs
left; the wait functions will block until this is the case or the timeout
expires.

All member functions, except the constructors and destructor, are async safe
and can be called from any thread. Functions other than `clear()` and the
wait functions can be called from inside an executing job. It is legal for
one thread to call `insert()` while another is calling `clear()`, but the
newly inserted job will probably be discarded without being executed.
