#include "rs-channel/channel.hpp"
#include "test/unit-test.hpp"
#include <atomic>
#include <chrono>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using namespace RS::Channel;
using namespace std::chrono;
using namespace std::literals;

void test_channel_generator() {

    int i = 0, j = 0;
    GeneratorChannel<int> chan([&i] { return ++i; });
    std::optional<int> oi;

    TEST(chan.wait_for(10ms));
    TEST(chan.read(j));
    TEST_EQUAL(j, 1);

    TEST(chan.wait_for(10ms));
    TEST(chan.read(j));
    TEST_EQUAL(j, 2);

    TEST(chan.wait_for(10ms));
    TRY(oi = chan.read_maybe());
    TEST(oi.has_value());
    TEST_EQUAL(oi.value(), 3);

    TRY(chan.close());
    TEST(chan.wait_for(10ms));
    TEST(chan.is_closed());
    TEST(! chan.read(j));
    TRY(oi = chan.read_maybe());
    TEST(! oi.has_value());

}

void test_channel_buffer() {

    BufferChannel chan;
    std::string s;
    size_t n = 0;

    TEST(! chan.wait_for(10ms));

    TEST(chan.write("Hello"));
    TEST(chan.write("World"));
    TEST(chan.wait_for(10ms));
    TRY(n = chan.append(s));
    TEST_EQUAL(n, 10u);
    TEST_EQUAL(s, "HelloWorld");
    TEST(! chan.wait_for(10ms));

    s.clear();
    TEST(chan.write("Hello"));
    TEST(chan.wait_for(10ms));
    TRY(n = chan.append(s));
    TEST_EQUAL(n, 5u);
    TEST_EQUAL(s, "Hello");
    TEST(! chan.wait_for(10ms));

    s.clear();
    TEST(chan.write("Hello"));
    TEST(chan.wait_for(10ms));
    TRY(n = chan.append(s));
    TEST_EQUAL(n, 5u);
    TEST_EQUAL(s, "Hello");
    TEST(! chan.wait_for(10ms));
    TEST(chan.write("World"));
    TEST(chan.wait_for(10ms));
    TRY(n = chan.append(s));
    TEST_EQUAL(n, 5u);
    TEST_EQUAL(s, "HelloWorld");
    TEST(! chan.wait_for(10ms));

    s.clear();
    TEST(! chan.wait_for(10ms));
    TRY(n = chan.append(s));
    TEST_EQUAL(n, 0u);
    TEST_EQUAL(s, "");

    s.clear();
    TRY(chan.close());
    TEST(chan.wait_for(10ms));
    TEST(chan.is_closed());
    TRY(n = chan.append(s));
    TEST_EQUAL(n, 0u);
    TEST_EQUAL(s, "");

}

void test_channel_queue() {

    QueueChannel<std::string> chan;
    std::string s;

    TEST(! chan.wait_for(10ms));

    TRY(chan.write("Hello"));
    TEST(chan.wait_for(10ms));
    TEST(chan.read(s));
    TEST_EQUAL(s, "Hello");
    TEST(! chan.wait_for(10ms));

    TRY(chan.write("Hello"));
    TRY(chan.write("World"));
    TEST(chan.wait_for(10ms));
    TEST(chan.read(s));
    TEST_EQUAL(s, "Hello");
    TEST(chan.wait_for(10ms));
    TEST(chan.read(s));
    TEST_EQUAL(s, "World");
    TEST(! chan.wait_for(10ms));

    TRY(chan.close());
    TEST(chan.wait_for(10ms));
    TEST(chan.is_closed());

}

void test_channel_value() {

    ValueChannel<std::string> chan;
    std::string s;

    TEST(! chan.wait_for(10ms));

    TRY(chan.write("Hello"));
    TEST(chan.wait_for(10ms));
    TEST(chan.read(s));
    TEST_EQUAL(s, "Hello");
    TEST(! chan.wait_for(10ms));

    TRY(chan.write("Hello"));
    TRY(chan.write("World"));
    TEST(chan.wait_for(10ms));
    TEST(chan.read(s));
    TEST_EQUAL(s, "World");
    TEST(! chan.wait_for(10ms));

    TRY(chan.write("World"));
    TEST(! chan.wait_for(10ms));

    TRY(chan.close());
    TEST(chan.wait_for(10ms));
    TEST(chan.is_closed());

}

void test_channel_timer() {

    TimerChannel chan(25ms);

    TEST(! chan.wait_for(1ms));
    TEST(chan.wait_for(100ms));

    TRY(std::this_thread::sleep_for(150ms));
    TEST(chan.wait_for(1ms));
    TEST(chan.wait_for(1ms));

    TRY(chan.close());
    TEST(chan.wait_for(10ms));
    TEST(chan.is_closed());

}

namespace {

    constexpr int cycles = 100;
    constexpr auto time_interval = 1ms;

    class SyncTimerChannel:
    public TimerChannel {
    public:
        explicit SyncTimerChannel(Channel::duration t) noexcept: TimerChannel(t) {}
        bool is_synchronous() const noexcept override { return true; }
    };

}

void test_channel_dispatch_empty() {

    Dispatch disp;
    Dispatch::result rc;
    TRY(rc = disp.run());
    TEST_EQUAL(rc.channel, nullptr);
    TEST(! rc.error);
    TRY(disp.stop());

}

void test_channel_dispatch_sync_close() {

    SyncTimerChannel chan(time_interval);
    Dispatch disp;
    Dispatch::result rc;
    int n = 0;
    TRY(disp.add(chan, [&] { ++n; if (n == cycles) chan.close(); }));
    TRY(rc = disp.run());
    TEST_EQUAL(rc.channel, &chan);
    TEST(! rc.error);
    TEST(chan.wait_for(1ms));
    TEST(chan.is_closed());
    TEST_EQUAL(n, cycles);
    TRY(disp.stop());

}

void test_channel_dispatch_async_close() {

    TimerChannel chan(time_interval);
    Dispatch disp;
    Dispatch::result rc;
    int n = 0;
    TRY(disp.add(chan, [&] { ++n; if (n == cycles) chan.close(); }));
    TRY(rc = disp.run());
    TEST_EQUAL(rc.channel, &chan);
    TEST(! rc.error);
    TEST(chan.wait_for(1ms));
    TEST(chan.is_closed());
    TEST_EQUAL(n, cycles);
    TRY(disp.stop());

}

void test_channel_dispatch_sync_exception() {

    SyncTimerChannel chan(time_interval);
    Dispatch disp;
    Dispatch::result rc;
    int n = 0;
    TRY(disp.add(chan, [&] { ++n; if (n == cycles) throw std::runtime_error("Test"); }));
    TRY(rc = disp.run());
    TEST_EQUAL(rc.channel, &chan);
    TEST(rc.error);
    TEST_THROW(std::rethrow_exception(rc.error), std::runtime_error);
    TRY(chan.wait_for(1ms));
    TEST_EQUAL(n, cycles);
    TRY(disp.stop());

}

void test_channel_dispatch_async_exception() {

    TimerChannel chan(time_interval);
    Dispatch disp;
    Dispatch::result rc;
    int n = 0;
    TRY(disp.add(chan, [&] { ++n; if (n == cycles) throw std::runtime_error("Test"); }));
    TRY(rc = disp.run());
    TEST_EQUAL(rc.channel, &chan);
    TEST(rc.error);
    TEST_THROW(std::rethrow_exception(rc.error), std::runtime_error);
    TRY(chan.wait_for(1ms));
    TEST_EQUAL(n, cycles);
    TRY(disp.stop());

}

void test_channel_dispatch_multiple_sync_exception() {

    SyncTimerChannel chan1(time_interval);
    SyncTimerChannel chan2(time_interval);
    Dispatch disp;
    Dispatch::result rc;
    int n1 = 0, n2 = 0;
    {
        TRY(disp.add(chan1, [&] { ++n1; if (n1 == cycles) throw std::runtime_error("Test"); }));
        TRY(disp.add(chan2, [&] { ++n2; }));
        TRY(rc = disp.run());
        TEST_EQUAL(rc.channel, &chan1);
        TEST(rc.error);
        TEST_THROW(std::rethrow_exception(rc.error), std::runtime_error);
        TEST(chan1.wait_for(1ms));
        TEST_EQUAL(n1, cycles);
        TRY(disp.stop());
    }
    TEST(chan2.wait_for(1ms));
    TEST(chan2.is_closed());

}

void test_channel_dispatch_multiple_async_exception() {

    TimerChannel chan1(time_interval);
    TimerChannel chan2(time_interval);
    Dispatch disp;
    Dispatch::result rc;
    int n1 = 0, n2 = 0;
    {
        TRY(disp.add(chan1, [&] { ++n1; if (n1 == cycles) throw std::runtime_error("Test"); }));
        TRY(disp.add(chan2, [&] { ++n2; }));
        TRY(rc = disp.run());
        TEST_EQUAL(rc.channel, &chan1);
        TEST(rc.error);
        TEST_THROW(std::rethrow_exception(rc.error), std::runtime_error);
        TEST(chan1.wait_for(1ms));
        TEST_EQUAL(n1, cycles);
        TRY(disp.stop());
    }
    TEST(chan2.wait_for(1ms));
    TEST(chan2.is_closed());

}

void test_channel_dispatch_heterogeneous_sync_exception() {

    SyncTimerChannel chan1(time_interval);
    TimerChannel chan2(time_interval);
    Dispatch disp;
    Dispatch::result rc;
    std::atomic<int> n1(0), n2(0);
    {
        TRY(disp.add(chan1, [&] { ++n1; if (n1 == cycles) throw std::runtime_error("Test"); }));
        TRY(disp.add(chan2, [&] { ++n2; }));
        TRY(rc = disp.run());
        TEST_EQUAL(rc.channel, &chan1);
        TEST(rc.error);
        TEST_THROW(std::rethrow_exception(rc.error), std::runtime_error);
        TEST(chan1.wait_for(1ms));
        TEST_EQUAL(n1.load(), cycles);
        TRY(disp.stop());
    }
    TEST(chan2.wait_for(1ms));
    TEST(chan2.is_closed());

}

void test_channel_dispatch_heterogeneous_async_exception() {

    SyncTimerChannel chan1(time_interval);
    TimerChannel chan2(time_interval);
    Dispatch disp;
    Dispatch::result rc;
    std::atomic<int> n1(0), n2(0);
    {
        TRY(disp.add(chan1, [&] { ++n1; }));
        TRY(disp.add(chan2, [&] { ++n2; if (n2 == cycles) throw std::runtime_error("Test"); }));
        TRY(rc = disp.run());
        TEST_EQUAL(rc.channel, &chan2);
        TEST(rc.error);
        TEST_THROW(std::rethrow_exception(rc.error), std::runtime_error);
        TEST(chan2.wait_for(1ms));
        TEST_EQUAL(n2.load(), cycles);
        TRY(disp.stop());
    }
    TEST(chan1.wait_for(1ms));
    TEST(chan1.is_closed());

}

void test_channel_dispatch_async_message_channel() {

    QueueChannel<int> chan;
    Dispatch disp;
    Dispatch::result rc;
    std::string s;
    std::vector<int> v;
    for (int i = 1; i <= 10; ++i)
        TRY(chan.write(i));
    TRY(disp.add(chan, [&] (int i) {
        v.push_back(i);
        if (i >= 5)
            chan.close();
    }));
    TRY(rc = disp.run());
    TEST_EQUAL(rc.channel, &chan);
    TEST(! rc.error);
    TEST(chan.wait_for(1ms));
    TEST(chan.is_closed());
    s = RS::UnitTest::format_range(v);
    TEST_EQUAL(s, "[1,2,3,4,5]");
    TRY(disp.stop());

}

void test_channel_dispatch_async_stream_channel() {

    BufferChannel chan;
    Dispatch disp;
    Dispatch::result rc;
    std::string s;
    TRY(chan.write("Hello world\n"));
    TRY(chan.set_block_size(5));
    TRY(disp.add(chan, [&] (std::string& t) {
        s += t;
        t.clear();
        if (s.find('\n') != std::string::npos)
            chan.close();
    }));
    TRY(rc = disp.run());
    TEST_EQUAL(rc.channel, &chan);
    TEST(! rc.error);
    TEST(chan.wait_for(1ms));
    TEST(chan.is_closed());
    TEST_EQUAL(s, "Hello world\n");
    TRY(disp.stop());

}
