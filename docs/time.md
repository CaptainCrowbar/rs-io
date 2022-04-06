# Time Utilities

_[I/O Library by Ross Smith](index.html)_

```c++
#include "rs-io/time.hpp"
namespace RS::IO;
```

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
