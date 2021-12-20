#include "rs-channel/net.hpp"
#include "rs-channel/channel.hpp"
#include "rs-unit-test.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace RS::Channel;

void test_channel_net_dns_query() {

    static const std::string good_name = "en.cppreference.com";
    static const std::string bad_name = "no-such-host.xyz";

    std::string name;
    SocketAddress addr;
    std::vector<SocketAddress> addrs;

    TRY(addr = Dns::host_to_ip(good_name));
    TEST(addr);
    TEST(addr.family() == AF_INET || addr.family() == AF_INET6);

    TRY(addrs = Dns::host_to_ips(good_name));
    TEST(! addrs.empty());
    if (! addrs.empty())
        TEST_EQUAL(addrs[0], addr);
    // std::cout << "... " << good_name << " => " << RS::UnitTest::format_range(addrs) << "\n";

    for (auto& a: addrs) {
        name.clear();
        TRY(name = Dns::ip_to_host(a));
        // std::cout << "... " << a << " => " << name << "\n";
    }

    TRY(addr = Dns::host_to_ip(bad_name));
    TEST(! addr);

    TRY(addrs = Dns::host_to_ips(bad_name));
    TEST(addrs.empty());

}
