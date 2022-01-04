# I/O Library

By Ross Smith

_[GitHub repository](https://github.com/CaptainCrowbar/rs-io)_

## Overview

```c++
#include "rs-io.hpp"
namespace RS::IO;
```

This library contains some facilities for I/O and related concepts, including
message dispatch, multithreading, and interprocess communication.

The CMake file includes an `install` target that copies the compiled library
and headers into `/usr/local/bin` and `/usr/local/include` or the equivalent.
Headers can be included individually as required, or the entire library can
be included using `"rs-io.hpp"`.

My [formatting](https://github.com/CaptainCrowbar/rs-format)
and [unit test](https://github.com/CaptainCrowbar/rs-unit-test) libraries
are used by this library. These are header-only libraries.

## Index

* General utilities
    * [Time utilities](time.html)
* File system
    * [File path](path.html)
* Multithreading
    * [Thread pool](thread-pool.html)
* Message dispatch
    * [Channel-based message dispatch](channel.html)
    * [Signal handling](signal.html)
* Interprocess communication
    * [Named mutex](named-mutex.html)
    * [Process control](process.html)
* Networking
    * [TCP/IP networking](net.html)
