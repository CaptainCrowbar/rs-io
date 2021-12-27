#include "rs-unit-test.hpp"

int main(int argc, char** argv) {

    RS::UnitTest::begin_tests(argc, argv);

    // channel-classes-test.cpp
    UNIT_TEST(channel_generator)
    UNIT_TEST(channel_buffer)
    UNIT_TEST(channel_queue)
    UNIT_TEST(channel_value)
    UNIT_TEST(channel_timer)

    // channel-dispatch-test.cpp
    UNIT_TEST(channel_dispatch_empty)
    UNIT_TEST(channel_dispatch_sync_close)
    UNIT_TEST(channel_dispatch_async_close)
    UNIT_TEST(channel_dispatch_sync_exception)
    UNIT_TEST(channel_dispatch_async_exception)
    UNIT_TEST(channel_dispatch_multiple_sync_exception)
    UNIT_TEST(channel_dispatch_multiple_async_exception)
    UNIT_TEST(channel_dispatch_heterogeneous_sync_exception)
    UNIT_TEST(channel_dispatch_heterogeneous_async_exception)
    UNIT_TEST(channel_dispatch_async_message_channel)
    UNIT_TEST(channel_dispatch_async_stream_channel)

    // net-address-test.cpp
    UNIT_TEST(channel_net_ipv4)
    UNIT_TEST(channel_net_ipv6)
    UNIT_TEST(channel_net_socket_address)
    UNIT_TEST(channel_net_ip_literals)

    // net-dns-test.cpp
    UNIT_TEST(channel_net_dns_query)

    // net-tcp-test.cpp
    UNIT_TEST(channel_net_tcp_client_server)
    UNIT_TEST(channel_net_socket_set)

    // process-test.cpp
    UNIT_TEST(channel_process_stream)
    UNIT_TEST(channel_process_text)
    UNIT_TEST(channel_process_shell_command)

    // signal-test.cpp
    UNIT_TEST(channel_signal_channel)
    UNIT_TEST(channel_signal_name)

    // named-mutex-test.cpp
    UNIT_TEST(channel_named_mutex)

    // thread-pool-test.cpp
    UNIT_TEST(channel_thread_pool_class)
    UNIT_TEST(channel_thread_pool_timing)

    // unit-test.cpp

    return RS::UnitTest::end_tests();

}
