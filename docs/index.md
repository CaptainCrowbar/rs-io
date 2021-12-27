# Message Dispatch Library

By Ross Smith

_[GitHub repository](https://github.com/CaptainCrowbar/rs-channel)_

## Overview

```c++
#include "rs-channel.hpp"
namespace RS::Channel;
```

This library contains some facilities for message dispatch between threads,
and related multithreading and interprocess communication facilities.

The CMake file includes an `install` target that copies the compiled library
and headers into `/usr/local/bin` and `/usr/local/include` or the equivalent.
Headers can be included individually as required, or the entire library can
be included using `"rs-channel.hpp"`.

My [unit test library](https://github.com/CaptainCrowbar/rs-unit-test) is used
for this library's unit tests. This a header-only library, and is not
required if you only want to use this library without building its unit
tests.

## Index

* [Timed-wait base class](waiter.html)
* [Channel-based message dispatch](channel.html)
* [TCP/IP networking](net.html)
* [Process control](process.html)
* [Signal handling](signal.html)
* [Named mutex](named-mutex.html)
* [Thread pool](thread-pool.html)
