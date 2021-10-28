cmake_minimum_required(VERSION 3.15)
# set(CMAKE_VERBOSE_MAKEFILE ON)

project(Channel
    VERSION 1.0
    LANGUAGES CXX
)

include(compilers.cmake)

set(library rs-channel)
set(unittest ${library}-test)
include_directories(.)
find_package(Threads REQUIRED)

add_library(${library} STATIC
    ${library}/thread-pool.cpp
    ${library}/channel.cpp
    ${library}/net.cpp
    ${library}/process.cpp
    ${library}/signal.cpp
)

add_executable(${unittest}
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

target_link_libraries(${unittest}
    PRIVATE ${library}
    PRIVATE Threads::Threads
)

if(WIN32)
    target_link_libraries(${unittest}
        PRIVATE ws2_32
    )
endif()

file(GLOB headers "${library}/*.hpp")
install(FILES ${headers} DESTINATION include/${library})
install(TARGETS ${library} LIBRARY DESTINATION lib)
