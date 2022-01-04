#include "rs-io/named-mutex.hpp"
#include "rs-unit-test.hpp"
#include <chrono>
#include <memory>
#include <string>

using namespace RS::IO;
using namespace std::chrono;
using namespace std::literals;

void test_rs_io_named_mutex() {

    std::string name = "rs-channel-named-mutex-test";
    std::unique_ptr<NamedMutex> mutex;

    TRY(mutex = std::make_unique<NamedMutex>(name));
    REQUIRE(mutex);

    TRY(mutex->lock());
    TRY(mutex->unlock());
    TEST(mutex->try_lock());
    TRY(mutex->unlock());
    TEST(mutex->try_lock_for(100ms));
    TRY(mutex->unlock());
    TEST(mutex->try_lock_until(system_clock::now() + 100ms));
    TRY(mutex->unlock());

}
