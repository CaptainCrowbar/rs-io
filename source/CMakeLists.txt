cmake_minimum_required(VERSION 3.15)
# set(CMAKE_VERBOSE_MAKEFILE ON)

project(Channel
    VERSION 1.0
    LANGUAGES CXX
)

include(compilers.cmake)

include_directories(.)
find_package(Threads REQUIRED)

add_library(rs-channel STATIC
    rs-channel/thread-pool.cpp
    rs-channel/channel.cpp
    rs-channel/net.cpp
    rs-channel/process.cpp
    rs-channel/signal.cpp
)

add_executable(channel-test
    test/thread-pool-test.cpp
    test/channel-classes-test.cpp
    test/channel-dispatch-test.cpp
    test/net-address-test.cpp
    test/net-dns-test.cpp
    test/net-tcp-test.cpp
    test/process-test.cpp
    test/signal-test.cpp
    test/unit-test.cpp
)

target_link_libraries(channel-test
    PRIVATE rs-channel
    PRIVATE Threads::Threads
)

if(WIN32)
    target_link_libraries(channel-test
        PRIVATE ws2_32
    )
endif()
