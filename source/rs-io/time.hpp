#pragma once

#include "rs-io/utility.hpp"
#include <chrono>
#include <ctime>

#ifdef _WIN32
    struct FILETIME;
#endif

#ifdef _XOPEN_SOURCE
    struct timeval;
#endif

namespace RS::IO {

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

    namespace Detail {

        template <typename C1, typename D1, typename C2, typename D2>
        struct ConvertTimePoint {
            std::chrono::time_point<C2, D2> operator()(std::chrono::time_point<C1, D1> t) const {
                auto now1 = C1::now();
                auto now2 = C2::now();
                auto now3 = C1::now();
                now1 += (now3 - now1) / 2;
                return std::chrono::time_point_cast<D2>(now2 + (t - now1));
            }
        };

        template <typename C, typename D1, typename D2>
        struct ConvertTimePoint<C, D1, C, D2> {
            std::chrono::time_point<C, D2> operator()(std::chrono::time_point<C, D1> t) const {
                return std::chrono::time_point_cast<D2>(t);
            }
        };

        void make_timeval(int64_t us, timeval& tv) noexcept;
        void read_timeval(const timeval& tv, std::chrono::seconds& s, std::chrono::microseconds& us) noexcept;

    }

    template <typename C1, typename D1, typename C2, typename D2>
    void convert_time_point(std::chrono::time_point<C1, D1> src, std::chrono::time_point<C2, D2>& dst) {
        dst = Detail::ConvertTimePoint<C1, D1, C2, D2>()(src);
    }

    void timepoint_to_timespec(const std::chrono::system_clock::time_point& tp, std::timespec& ts) noexcept;
    void timepoint_to_timeval(const std::chrono::system_clock::time_point& tp, timeval& tv) noexcept;
    void timespec_to_timepoint(const std::timespec& ts, std::chrono::system_clock::time_point& tp) noexcept;
    void timeval_to_timepoint(const timeval& tv, std::chrono::system_clock::time_point& tp) noexcept;

    template <typename R, typename P>
    void duration_to_timespec(const std::chrono::duration<R, P>& d, std::timespec& ts) noexcept {
        using namespace std::chrono;
        static constexpr int64_t G = 1'000'000'000ll;
        int64_t ns = duration_cast<nanoseconds>(d).count();
        ts = {time_t(ns / G), long(ns % G)};
    }

    template <typename R, typename P>
    void duration_to_timeval(const std::chrono::duration<R, P>& d, timeval& tv) noexcept {
        using namespace std::chrono;
        int64_t us = duration_cast<microseconds>(d).count();
        Detail::make_timeval(us, tv);
    }

    template <typename R, typename P>
    void timespec_to_duration(const std::timespec& ts, std::chrono::duration<R, P>& d) noexcept {
        using namespace std::chrono;
        using D = duration<R, P>;
        d = duration_cast<D>(seconds(ts.tv_sec)) + duration_cast<D>(nanoseconds(ts.tv_nsec));
    }

    template <typename R, typename P>
    void timeval_to_duration(const timeval& tv, std::chrono::duration<R, P>& d) noexcept {
        using namespace std::chrono;
        using D = duration<R, P>;
        seconds s;
        microseconds us;
        Detail::read_timeval(tv, s, us);
        d = duration_cast<D>(s) + duration_cast<D>(us);
    }

    #ifdef _WIN32

        void filetime_to_timepoint(const FILETIME& ft, std::chrono::system_clock::time_point& tp) noexcept;
        void timepoint_to_filetime(const std::chrono::system_clock::time_point& tp, FILETIME& ft) noexcept;

    #endif

}
