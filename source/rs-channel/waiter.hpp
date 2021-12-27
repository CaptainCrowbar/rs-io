#pragma once

#include <chrono>

namespace RS::Channel {

    class Waiter {
    public:
        using clock = std::chrono::system_clock;
        using duration = clock::duration;
        using time_point = clock::time_point;
        virtual ~Waiter() noexcept = 0;
        virtual bool poll() { return wait_for({}); }
        virtual void wait() { while (! wait_for(std::chrono::seconds(1))) {} }
        virtual bool wait_for(duration t) { return wait_until(clock::now() + t); }
        virtual bool wait_until(time_point t) { return wait_for(t - clock::now()); }
    };

        inline Waiter::~Waiter() noexcept {}

}
