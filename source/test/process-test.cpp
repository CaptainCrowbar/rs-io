#include "rs-io/process.hpp"
#include "rs-io/channel.hpp"
#include "rs-unit-test.hpp"
#include <chrono>
#include <memory>
#include <set>
#include <string>

using namespace RS::IO;
using namespace std::chrono;
using namespace std::literals;

namespace {

    #ifdef _XOPEN_SOURCE
        const std::string list_command = "ls /";
        const std::string file1 = "bin";
        const std::string file2 = "usr";
    #else
        const std::string list_command = "dir /b \"c:\\\"";
        const std::string file1 = "Program Files";
        const std::string file2 = "Windows";
    #endif

    std::multiset<std::string> split(const std::string& str, const std::string& del = " \n\r\t") {
        std::multiset<std::string> set;
        size_t i = 0, j = 0, n = str.size();
        while (j < n) {
            i = str.find_first_not_of(del, j);
            if (i == std::string::npos)
                break;
            j = str.find_first_of(del, i);
            if (j == std::string::npos)
                j = n;
            set.insert(str.substr(i, j - i));
        }
        return set;
    }

}

void test_rs_io_process_stream() {

    std::unique_ptr<StreamProcess> chan;
    std::string s;
    std::multiset<std::string> set;
    int st = -1;
    size_t n = 0;

    TRY(chan = std::make_unique<StreamProcess>(list_command));
    TEST(chan->wait_for(100ms));
    TEST(! chan->is_closed());
    TRY(n = chan->append(s));
    TEST(n > 0u);
    TEST_MATCH(s, "[\\r\\n A-Za-z0-9._-]+");

    TRY(set = split(s, "\r\n"));
    TEST(! set.empty());
    TEST_EQUAL(set.count(file1), 1u);
    TEST_EQUAL(set.count(file2), 1u);

    TEST(chan->wait_for(10ms));
    TEST(! chan->is_closed());
    TRY(n = chan->append(s));
    TEST_EQUAL(n, 0u);
    TEST(chan->wait_for(10ms));
    TEST(chan->is_closed());
    TRY(st = chan->status());
    TEST_EQUAL(st, 0);

    chan.reset();
    TRY(chan = std::make_unique<StreamProcess>(list_command));
    TRY(s = chan->read_all());
    TRY(set = split(s, "\r\n"));
    TEST(! set.empty());
    TEST_EQUAL(set.count(file1), 1u);
    TEST_EQUAL(set.count(file2), 1u);

}

void test_rs_io_process_text() {

    std::unique_ptr<TextProcess> chan;
    std::string s;
    std::multiset<std::string> set;
    int st = -1;
    bool rc = false;

    TRY(chan = std::make_unique<TextProcess>(list_command));

    for (int i = 1; i <= 1000; ++i) {
        TRY(rc = bool(chan->wait_for(10ms)));
        if (chan->is_closed())
            break;
        if (rc) {
            TEST(chan->read(s));
            set.insert(s);
        }
    }

    TEST(! set.empty());
    TEST_EQUAL(set.count(file1), 1u);
    TEST_EQUAL(set.count(file2), 1u);

    TRY(chan->close());
    TRY(st = chan->status());
    TEST_EQUAL(st, 0);

    chan.reset();
    TRY(chan = std::make_unique<TextProcess>(list_command));
    TRY(s = chan->read_all());
    TRY(set = split(s, "\r\n"));
    TEST(! set.empty());
    TEST_EQUAL(set.count(file1), 1u);
    TEST_EQUAL(set.count(file2), 1u);

}

void test_rs_io_process_shell_command() {

    std::string s;
    std::multiset<std::string> set;

    TRY(s = shell(list_command));
    TEST(! s.empty());
    TRY(set = split(s, "\r\n"));
    TEST(! set.empty());
    TEST_EQUAL(set.count(file1), 1u);
    TEST_EQUAL(set.count(file2), 1u);

}
