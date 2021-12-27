# Time Utilities

_[Channel Library by Ross Smith](index.html)_

```c++
#include "rs-channel/time.hpp"
namespace RS::Channel;
```

## Timed-wait base class

```c++
class Waiter;
    using Waiter::clock = std::chrono::system_clock;
    using Waiter::duration = clock::duration;
    using Waiter::time_point = clock::time_point;
    virtual Waiter::~Waiter() noexcept = 0;
    virtual bool Waiter::poll();
    virtual void Waiter::wait();
    virtual bool Waiter::wait_for(duration t);
    virtual bool Waiter::wait_until(time_point t);
```

A general purpose base class for waitable objects. The `poll()` function
returns true if whatever condition was being waited on is ready, false if it
is not. The `wait_for()` and `wait_until()` functions return true on success,
false on timeout. The `wait()` function will wait indefinitely.

A derived class must implement at least one of `wait_for()` or `wait_until()`,
whichever is more convenient for the particular class; the default
implementations call each other. The default implementations of `poll()` and
`wait()` call `wait_for()`; inheritors may optionally also implement either or
both of `poll()` or `wait()`, if a more efficient implementation is possible.

## Time conversion functions

```c++
template <typename C1, typename D1, typename C2, typename D2>
    void convert_time_point(time_point<C1, D1> src, time_point<C2, D2>& dst);
```

Converts a time point from one representation to another, possibly on a
different clock. This will call `time_point_cast()` if possible; otherwise,
it will check the current times on both clocks and use that to convert from
one clock to the other.

```c++
// Unix
void timepoint_to_timespec(const system_clock::time_point& tp,
    timespec& ts) noexcept;
void timepoint_to_timeval(const system_clock::time_point& tp,
    timeval& tv) noexcept;
void timespec_to_timepoint(const timespec& ts,
    system_clock::time_point& tp) noexcept;
void timeval_to_timepoint(const timeval& tv,
    system_clock::time_point& tp) noexcept;
template <typename R, typename P>
    void duration_to_timespec(const duration<R, P>& d, timespec& ts) noexcept;
template <typename R, typename P>
    void duration_to_timeval(const duration<R, P>& d, timeval& tv) noexcept;
template <typename R, typename P>
    void timespec_to_duration(const timespec& ts, duration<R, P>& d) noexcept;
template <typename R, typename P>
    void timeval_to_duration(const timeval& tv, duration<R, P>& d) noexcept;

// Windows
void filetime_to_timepoint(const FILETIME& ft,
    system_clock::time_point& tp) noexcept;
void timepoint_to_filetime(const system_clock::time_point& tp,
    FILETIME& ft) noexcept;
```

Conversion functions between C++ chrono types and system API types. These
return their result through a reference argument to avoid having to include
system headers in this header. Behaviour is undefined if the value being
represented is out of range for either the source or destination type.

The Windows functions are only defined on Windows builds; the Unix functions
are always defined, since the relevant time structures are also defined in
the Windows API.

For reference, the system types are:

```c++
// Unix
#include <time.h>
    struct timespec {
        time_t tv_sec;  // seconds
        long tv_nsec;   // nanoseconds
    };
#include <sys/time.h>
    struct timeval {
        time_t tv_sec;        // seconds
        suseconds_t tv_usec;  // microseconds
    };

// Windows
#include <windows.h>
    struct FILETIME {
        DWORD dwLowDateTime;   // low 32 bits
        DWORD dwHighDateTime;  // high 32 bits
    };
```
