#include "rs-channel/named-mutex.hpp"
#include "rs-unit-test.hpp"
#include <chrono>
#include <memory>
#include <string>

using namespace RS::Channel;
using namespace std::chrono;
using namespace std::literals;

void test_channel_named_mutex() {

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
