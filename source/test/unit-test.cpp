// This file is generated by the rs-update-tests script

#include "rs-unit-test.hpp"

int main(int argc, char** argv) {

    RS::UnitTest::begin_tests(argc, argv);

    // time-test.cpp
    UNIT_TEST(rs_io_time_point_conversion)
    UNIT_TEST(rs_io_time_system_specific_conversions)

    // path-name-test.cpp
    UNIT_TEST(rs_io_path_legal_names)
    UNIT_TEST(rs_io_path_unicode_names)
    UNIT_TEST(rs_io_path_name_normalization)
    UNIT_TEST(rs_io_path_name_properties)
    UNIT_TEST(rs_io_path_name_breakdown)
    UNIT_TEST(rs_io_path_name_combination)
    UNIT_TEST(rs_io_path_name_manipulation)
    UNIT_TEST(rs_io_path_name_comparison)

    // path-file-system-test.cpp
    UNIT_TEST(rs_io_path_resolution)
    UNIT_TEST(rs_io_path_file_system_queries)
    UNIT_TEST(rs_io_path_file_system_updates)
    UNIT_TEST(rs_io_path_io)
    UNIT_TEST(rs_io_path_links)
    UNIT_TEST(rs_io_path_metadata)

    // path-directory-test.cpp
    UNIT_TEST(rs_io_path_directory_iterators)
    UNIT_TEST(rs_io_path_current_directory)
    UNIT_TEST(rs_io_path_deep_search)

    // stdio-test.cpp
    UNIT_TEST(rs_io_stdio_cstdio)
    UNIT_TEST(rs_io_stdio_fdio)
    UNIT_TEST(rs_io_stdio_pipe)
    UNIT_TEST(rs_io_stdio_winio)
    UNIT_TEST(rs_io_stdio_null_device)
    UNIT_TEST(rs_io_stdio_anonymous_temporary_file)
    UNIT_TEST(rs_io_stdio_named_temporary_file)

    // channel-classes-test.cpp
    UNIT_TEST(rs_io_channel_generator)
    UNIT_TEST(rs_io_channel_buffer)
    UNIT_TEST(rs_io_channel_queue)
    UNIT_TEST(rs_io_channel_value)
    UNIT_TEST(rs_io_channel_timer)

    // channel-dispatch-test.cpp
    UNIT_TEST(rs_io_channel_dispatch_empty)
    UNIT_TEST(rs_io_channel_dispatch_sync_close)
    UNIT_TEST(rs_io_channel_dispatch_async_close)
    UNIT_TEST(rs_io_channel_dispatch_sync_exception)
    UNIT_TEST(rs_io_channel_dispatch_async_exception)
    UNIT_TEST(rs_io_channel_dispatch_multiple_sync_exception)
    UNIT_TEST(rs_io_channel_dispatch_multiple_async_exception)
    UNIT_TEST(rs_io_channel_dispatch_heterogeneous_sync_exception)
    UNIT_TEST(rs_io_channel_dispatch_heterogeneous_async_exception)
    UNIT_TEST(rs_io_channel_dispatch_async_message_channel)
    UNIT_TEST(rs_io_channel_dispatch_async_stream_channel)

    // net-address-test.cpp
    UNIT_TEST(rs_io_net_ipv4)
    UNIT_TEST(rs_io_net_ipv6)
    UNIT_TEST(rs_io_net_socket_address)
    UNIT_TEST(rs_io_net_ip_literals)

    // net-dns-test.cpp
    UNIT_TEST(rs_io_net_dns_query)

    // net-tcp-test.cpp
    UNIT_TEST(rs_io_net_tcp_client_server)
    UNIT_TEST(rs_io_net_socket_set)

    // process-test.cpp
    UNIT_TEST(rs_io_process_stream)
    UNIT_TEST(rs_io_process_text)
    UNIT_TEST(rs_io_process_shell_command)

    // signal-test.cpp
    UNIT_TEST(rs_io_signal_channel)
    UNIT_TEST(rs_io_signal_name)

    // named-mutex-test.cpp
    UNIT_TEST(rs_io_named_mutex)

    // thread-pool-test.cpp
    UNIT_TEST(rs_io_thread_pool_class)
    UNIT_TEST(rs_io_thread_pool_each)
    UNIT_TEST(rs_io_thread_pool_timing)

    // uri-schemes-test.cpp
    UNIT_TEST(rs_io_uri_parse_http)
    UNIT_TEST(rs_io_uri_parse_file)
    UNIT_TEST(rs_io_uri_parse_mailto)

    // uri-general-test.cpp
    UNIT_TEST(rs_io_uri_assembly)
    UNIT_TEST(rs_io_uri_modifiers)
    UNIT_TEST(rs_io_uri_navigation)
    UNIT_TEST(rs_io_uri_encoding)
    UNIT_TEST(rs_io_uri_query)

    // unit-test.cpp

    return RS::UnitTest::end_tests();

}
