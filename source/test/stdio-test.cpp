#include "rs-io/stdio.hpp"
#include "rs-io/path.hpp"
#include "rs-io/utility.hpp"
#include "rs-unit-test.hpp"
#include <algorithm>
#include <iterator>
#include <memory>
#include <system_error>
#include <utility>

using namespace RS::IO;

#ifdef _XOPEN_SOURCE
    #define SLASH "/"
#else
    #define SLASH "\\"
#endif

namespace {

    void strip_unc(Path& path) {
        #ifdef _XOPEN_SOURCE
            (void)path;
        #else
            auto name = path.name();
            if (name.substr(0, 4) == "\\\\?\\")
                path = name.substr(4, npos);
        #endif
    }

}

void test_rs_io_stdio_cstdio() {

    Cstdio io;
    Path file = "__cstdio_test__", no_file = "__no_such_file__", path;
    std::string text;
    std::vector<std::string> vec;
    ptrdiff_t offset = 0;
    size_t n = 0;
    ScopeGuard guard([=] { file.remove(); });

    TRY(file.remove());
    TEST(! file.exists());
    TEST(! io.is_open());
    TEST(! io.ok());

    TRY(io = Cstdio(file, IoBase::mode::write_only));
    TEST(io.is_open());
    TEST(io.ok());
    TRY(io.write_str("Hello world\n"));
    TEST(io.ok());
    TRY(io.write_str("Goodbye\n"));
    TEST(io.ok());
    TRY(io.close());
    TEST(! io.is_open());
    TEST(file.exists());

    text.clear();
    TRY(io = Cstdio(file));
    TRY(n = io.read_n(text, 12));  TEST(io.ok());  TEST_EQUAL(n, 12u);  TEST_EQUAL(text, "Hello world\n");
    TRY(n = io.read_n(text, 8));   TEST(io.ok());  TEST_EQUAL(n, 8u);   TEST_EQUAL(text, "Hello world\nGoodbye\n");
    TRY(n = io.read_n(text, 1));   TEST(io.ok());  TEST_EQUAL(n, 0u);   TEST_EQUAL(text, "Hello world\nGoodbye\n");
    TRY(io.close());

    TRY(io = Cstdio(file));
    TRY(text = io.read_str(12));  TEST(io.ok());  TEST_EQUAL(text, "Hello world\n");
    TRY(text = io.read_str(8));   TEST(io.ok());  TEST_EQUAL(text, "Goodbye\n");
    TRY(text = io.read_str(1));   TEST(io.ok());  TEST_EQUAL(text, "");
    TRY(io.close());

    TRY(io = Cstdio(file));
    TRY(text = io.read_str(20));  TEST(io.ok());  TEST_EQUAL(text, "Hello world\nGoodbye\n");
    TRY(text = io.read_str(1));   TEST(io.ok());  TEST_EQUAL(text, "");
    TRY(io.close());

    TRY(io = Cstdio(file));
    TRY(text = io.read_str(100));  TEST(io.ok());  TEST_EQUAL(text, "Hello world\nGoodbye\n");
    TRY(io.close());

    TRY(io = Cstdio(file));
    TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "Hello world\n");
    TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "Goodbye\n");
    TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "");
    TRY(io.close());

    TRY(io = Cstdio(file));
    TRY(text = io.read_all());  TEST(io.ok());  TEST_EQUAL(text, "Hello world\nGoodbye\n");
    TRY(io.close());

    TRY(io = Cstdio(file));
    auto lines = io.lines();
    vec.clear();
    TRY(std::copy(lines.begin(), lines.end(), std::back_inserter(vec)));
    TEST_EQUAL(vec.size(), 2u);
    TEST_EQUAL(vec.at(0), "Hello world\n");
    TEST_EQUAL(vec.at(1), "Goodbye\n");
    TRY(io.close());

    TRY(io = Cstdio(file));
    TRY(offset = io.tell());     TEST(io.ok());  TEST_EQUAL(offset, 0);
    TRY(io.seek(12));            TEST(io.ok());
    TRY(offset = io.tell());     TEST(io.ok());  TEST_EQUAL(offset, 12);
    TRY(text = io.read_str(7));  TEST(io.ok());  TEST_EQUAL(text, "Goodbye");
    TRY(offset = io.tell());     TEST(io.ok());  TEST_EQUAL(offset, 19);
    TRY(io.seek(-13));           TEST(io.ok());
    TRY(offset = io.tell());     TEST(io.ok());  TEST_EQUAL(offset, 6);
    TRY(text = io.read_str(5));  TEST(io.ok());  TEST_EQUAL(text, "world");
    TRY(offset = io.tell());     TEST(io.ok());  TEST_EQUAL(offset, 11);
    TRY(io.close());

    TRY(io = Cstdio(file));
    TRY(path = io.get_path());
    TEST(path.is_absolute());
    TEST(path.id() == file.id());
    TRY(strip_unc(path));
    TEST_EQUAL(path, file.resolve());
    TRY(io.close());

    TRY(io = Cstdio(no_file, IoBase::mode::read_only));
    TEST_THROW(io.check(), std::system_error);

    TRY(io = Cstdio(file, IoBase::mode::write_only));
    TRY(io.format("Agent {0}\n", 86));
    TRY(io.print("Agent", 99));
    TRY(io.close());

    TRY(io = Cstdio(file));
    TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "Agent 86\n");
    TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "Agent 99\n");
    TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "");
    TRY(io.close());

}

void test_rs_io_stdio_fdio() {

    Fdio io;
    Path file = "__fdio_test__", no_file = "__no_such_file__", path;
    std::string text;
    std::vector<std::string> vec;
    ptrdiff_t offset = 0;
    size_t n = 0;
    ScopeGuard guard([=] { file.remove(); });

    TRY(file.remove());
    TEST(! file.exists());
    TEST(! io.is_open());
    TEST(! io.ok());

    TRY(io = Fdio(file, IoBase::mode::write_only));
    TEST(io.is_open());
    TEST(io.ok());
    TRY(io.write_str("Hello world\n"));
    TEST(io.ok());
    TRY(io.write_str("Goodbye\n"));
    TEST(io.ok());
    TRY(io.close());
    TEST(! io.is_open());
    TEST(file.exists());

    text.clear();
    TRY(io = Fdio(file));
    TRY(n = io.read_n(text, 12));  TEST(io.ok());  TEST_EQUAL(n, 12u);  TEST_EQUAL(text, "Hello world\n");
    TRY(n = io.read_n(text, 8));   TEST(io.ok());  TEST_EQUAL(n, 8u);   TEST_EQUAL(text, "Hello world\nGoodbye\n");
    TRY(n = io.read_n(text, 1));   TEST(io.ok());  TEST_EQUAL(n, 0u);   TEST_EQUAL(text, "Hello world\nGoodbye\n");
    TRY(io.close());

    TRY(io = Fdio(file));
    TRY(text = io.read_str(12));  TEST(io.ok());  TEST_EQUAL(text, "Hello world\n");
    TRY(text = io.read_str(8));   TEST(io.ok());  TEST_EQUAL(text, "Goodbye\n");
    TRY(text = io.read_str(1));   TEST(io.ok());  TEST_EQUAL(text, "");
    TRY(io.close());

    TRY(io = Fdio(file));
    TRY(text = io.read_str(100));  TEST(io.ok());  TEST_EQUAL(text, "Hello world\nGoodbye\n");
    TRY(io.close());

    TRY(io = Fdio(file));
    TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "Hello world\n");
    TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "Goodbye\n");
    TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "");
    TRY(io.close());

    TRY(io = Fdio(file));
    TRY(text = io.read_all());  TEST(io.ok());  TEST_EQUAL(text, "Hello world\nGoodbye\n");
    TRY(io.close());

    TRY(io = Fdio(file));
    auto lines = io.lines();
    vec.clear();
    TRY(std::copy(lines.begin(), lines.end(), std::back_inserter(vec)));
    TEST_EQUAL(vec.size(), 2u);
    TEST_EQUAL(vec.at(0), "Hello world\n");
    TEST_EQUAL(vec.at(1), "Goodbye\n");
    TRY(io.close());

    TRY(io = Fdio(file));
    TRY(offset = io.tell());     TEST(io.ok());  TEST_EQUAL(offset, 0);
    TRY(io.seek(12));            TEST(io.ok());
    TRY(offset = io.tell());     TEST(io.ok());  TEST_EQUAL(offset, 12);
    TRY(text = io.read_str(7));  TEST(io.ok());  TEST_EQUAL(text, "Goodbye");
    TRY(offset = io.tell());     TEST(io.ok());  TEST_EQUAL(offset, 19);
    TRY(io.seek(-13));           TEST(io.ok());
    TRY(offset = io.tell());     TEST(io.ok());  TEST_EQUAL(offset, 6);
    TRY(text = io.read_str(5));  TEST(io.ok());  TEST_EQUAL(text, "world");
    TRY(offset = io.tell());     TEST(io.ok());  TEST_EQUAL(offset, 11);
    TRY(io.close());

    TRY(io = Fdio(file));
    TRY(path = io.get_path());
    TEST(path.is_absolute());
    TEST(path.id() == file.id());
    TRY(strip_unc(path));
    TEST_EQUAL(path, file.resolve());
    TRY(io.close());

    TRY(io = Fdio(no_file, IoBase::mode::read_only));
    TEST_THROW(io.check(), std::system_error);

    TRY(io = Fdio(file, IoBase::mode::write_only));
    TRY(io.format("Agent {0}\n", 86));
    TRY(io.print("Agent", 99));
    TRY(io.close());

    TRY(io = Fdio(file));
    TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "Agent 86\n");
    TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "Agent 99\n");
    TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "");
    TRY(io.close());

}

void test_rs_io_stdio_pipe() {

    std::pair<Fdio, Fdio> pipe;
    std::string text;

    TRY(pipe = Fdio::pipe());
    TRY(pipe.second.write_str("Hello world\n"));
    TRY(pipe.second.write_str("Goodbye\n"));
    TRY(text = pipe.first.read_str(100));
    TEST_EQUAL(text, "Hello world\nGoodbye\n");

}

void test_rs_io_stdio_winio() {

    #ifdef _WIN32

        Winio io;
        Path file = "__winio_test__", no_file = "__no_such_file__", path;
        std::string text;
        std::vector<std::string> vec;
        ptrdiff_t offset = 0;
        size_t n = 0;
        ScopeGuard guard([=] { file.remove(); });

        TRY(file.remove());
        TEST(! file.exists());
        TEST(! io.is_open());
        TEST(! io.ok());

        TRY(io = Winio(file, IoBase::mode::write_only));
        TEST(io.is_open());
        TEST(io.ok());
        TRY(io.write_str("Hello world\n"));
        TEST(io.ok());
        TRY(io.write_str("Goodbye\n"));
        TEST(io.ok());
        TRY(io.close());
        TEST(! io.is_open());
        TEST(file.exists());

        text.clear();
        TRY(io = Winio(file));
        TRY(n = io.read_n(text, 12));  TEST(io.ok());  TEST_EQUAL(n, 12u);  TEST_EQUAL(text, "Hello world\n");
        TRY(n = io.read_n(text, 8));   TEST(io.ok());  TEST_EQUAL(n, 8u);   TEST_EQUAL(text, "Hello world\nGoodbye\n");
        TRY(n = io.read_n(text, 1));   TEST(io.ok());  TEST_EQUAL(n, 0u);   TEST_EQUAL(text, "Hello world\nGoodbye\n");
        TRY(io.close());

        TRY(io = Winio(file));
        TRY(text = io.read_str(12));  TEST(io.ok());  TEST_EQUAL(text, "Hello world\n");
        TRY(text = io.read_str(8));   TEST(io.ok());  TEST_EQUAL(text, "Goodbye\n");
        TRY(text = io.read_str(1));   TEST(io.ok());  TEST_EQUAL(text, "");
        TRY(io.close());

        TRY(io = Winio(file));
        TRY(text = io.read_str(20));  TEST(io.ok());  TEST_EQUAL(text, "Hello world\nGoodbye\n");
        TRY(text = io.read_str(1));   TEST(io.ok());  TEST_EQUAL(text, "");
        TRY(io.close());

        TRY(io = Winio(file));
        TRY(text = io.read_str(100));  TEST(io.ok());  TEST_EQUAL(text, "Hello world\nGoodbye\n");
        TRY(io.close());

        TRY(io = Winio(file));
        TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "Hello world\n");
        TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "Goodbye\n");
        TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "");
        TRY(io.close());

        TRY(io = Winio(file));
        TRY(text = io.read_all());  TEST(io.ok());  TEST_EQUAL(text, "Hello world\nGoodbye\n");
        TRY(io.close());

        TRY(io = Winio(file));
        auto lines = io.lines();
        vec.clear();
        TRY(std::copy(lines.begin(), lines.end(), std::back_inserter(vec)));
        TEST_EQUAL(vec.size(), 2u);
        TEST_EQUAL(vec.at(0), "Hello world\n");
        TEST_EQUAL(vec.at(1), "Goodbye\n");
        TRY(io.close());

        TRY(io = Winio(file));
        TRY(offset = io.tell());     TEST_EQUAL(offset, 0);
        TRY(io.seek(12));
        TRY(offset = io.tell());     TEST_EQUAL(offset, 12);
        TRY(text = io.read_str(7));  TEST_EQUAL(text, "Goodbye");
        TRY(offset = io.tell());     TEST_EQUAL(offset, 19);
        TRY(io.seek(-13));
        TRY(offset = io.tell());     TEST_EQUAL(offset, 6);
        TRY(text = io.read_str(5));  TEST_EQUAL(text, "world");
        TRY(offset = io.tell());     TEST_EQUAL(offset, 11);
        TRY(io.close());

        TRY(io = Winio(file));
        TRY(path = io.get_path());
        TEST(path.is_absolute());
        TEST(path.id() == file.id());
        TRY(strip_unc(path));
        TEST_EQUAL(path, file.resolve());
        TRY(io.close());

        TRY(io = Winio(no_file, IoBase::mode::read_only));
        TEST_THROW(io.check(), std::system_error);

        TRY(io = Winio(file, IoBase::mode::write_only));
        TRY(io.format("Agent {0}\n", 86));
        TRY(io.print("Agent", 99));
        TRY(io.close());

        TRY(io = Winio(file));
        TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "Agent 86\n");
        TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "Agent 99\n");
        TRY(text = io.read_line());  TEST(io.ok());  TEST_EQUAL(text, "");
        TRY(io.close());

    #endif

}

void test_rs_io_stdio_null_device() {

    std::string text;

    {
        Cstdio io;
        TRY(io = Cstdio::null());
        TRY(text = io.read_str(20));
        TRY(text = io.read_str(20));
        TEST_EQUAL(text, "");
    }

    {
        Cstdio io;
        TRY(io = Cstdio::null());
        TRY(io.write_str("Hello world\n"));
        TRY(io.write_str("Hello world\n"));
        TRY(text = io.read_str(20));
        TRY(text = io.read_str(20));
        #ifdef _XOPEN_SOURCE
            TEST_EQUAL(text, "");
        #endif
    }

    {
        Fdio io;
        TRY(io = Fdio::null());
        TRY(text = io.read_str(20));
        TRY(text = io.read_str(20));
        TEST_EQUAL(text, "");
    }

    {
        Fdio io;
        TRY(io = Fdio::null());
        TRY(io.write_str("Hello world\n"));
        TRY(io.write_str("Hello world\n"));
        TRY(text = io.read_str(20));
        TRY(text = io.read_str(20));
        TEST_EQUAL(text, "");
    }

    #ifdef _WIN32

        {
            Winio io;
            TRY(io = Winio::null());
            TRY(text = io.read_str(20));
            TRY(text = io.read_str(20));
            TEST_EQUAL(text, "");
        }

        {
            Winio io;
            TRY(io = Winio::null());
            TRY(io.write_str("Hello world\n"));
            TRY(io.write_str("Hello world\n"));
            TRY(text = io.read_str(20));
            TRY(text = io.read_str(20));
            TEST_EQUAL(text, "");
        }

    #endif

}

void test_rs_io_stdio_anonymous_temporary_file() {

    std::unique_ptr<TempFile> tf;
    Path path;

    TRY(tf = std::make_unique<TempFile>());
    REQUIRE(tf);
    TEST(tf->get());
    TRY(path = tf->get_path());
    TEST(! path.empty());
    TRY(tf.reset());
    TEST(! path.exists());

}

void test_rs_io_stdio_named_temporary_file() {

    std::unique_ptr<TempFile> tf;
    Path path;

    TRY(tf = std::make_unique<TempFile>("", "__test_tempfile_"));
    REQUIRE(tf);
    TEST(tf->get());
    TRY(path = tf->get_path());
    TEST(! path.empty());
    TEST_MATCH(path.name(), ".+" SLASH "__test_tempfile_[[:xdigit:]]{16}$");
    TEST(path.exists());
    TEST(path.is_file());
    TRY(tf.reset());
    TEST(! path.exists());

}
