#pragma once

#include "rs-io/utility.hpp"
#include "rs-tl/time.hpp"
#include <chrono>
#include <ctime>

namespace RS::IO {

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

        void make_timeval(int64_t us, void* tvptr) noexcept;
        void read_timeval(const void* tvptr, std::chrono::seconds& s, std::chrono::microseconds& us) noexcept;

    }

    template <typename C1, typename D1, typename C2, typename D2>
    void convert_time_point(std::chrono::time_point<C1, D1> src, std::chrono::time_point<C2, D2>& dst) {
        dst = Detail::ConvertTimePoint<C1, D1, C2, D2>()(src);
    }

    template <typename R, typename P>
    void duration_to_timespec(const std::chrono::duration<R, P>& d, std::timespec& ts) noexcept {
        using namespace std::chrono;
        static constexpr int64_t G = 1'000'000'000ll;
        int64_t ns = duration_cast<nanoseconds>(d).count();
        ts = {time_t(ns / G), long(ns % G)};
    }

    template <typename R, typename P, typename TV>
    void duration_to_timeval(const std::chrono::duration<R, P>& d, TV& tv) noexcept {
        using namespace std::chrono;
        int64_t us = duration_cast<microseconds>(d).count();
        Detail::make_timeval(us, &tv);
    }

    template <typename R, typename P>
    void timespec_to_duration(const std::timespec& ts, std::chrono::duration<R, P>& d) noexcept {
        using namespace std::chrono;
        using D = duration<R, P>;
        d = duration_cast<D>(seconds(ts.tv_sec)) + duration_cast<D>(nanoseconds(ts.tv_nsec));
    }

    template <typename R, typename P, typename TV>
    void timeval_to_duration(const TV& tv, std::chrono::duration<R, P>& d) noexcept {
        using namespace std::chrono;
        using D = duration<R, P>;
        seconds s;
        microseconds us;
        Detail::read_timeval(&tv, s, us);
        d = duration_cast<D>(s) + duration_cast<D>(us);
    }

    inline void timepoint_to_timespec(const std::chrono::system_clock::time_point& tp, std::timespec& ts) noexcept {
        using namespace std::chrono;
        duration_to_timespec(tp - system_clock::time_point(), ts);
    }

    template <typename TV>
    void timepoint_to_timeval(const std::chrono::system_clock::time_point& tp, TV& tv) noexcept {
        using namespace std::chrono;
        duration_to_timeval(tp - system_clock::time_point(), tv);
    }

    inline void timespec_to_timepoint(const std::timespec& ts, std::chrono::system_clock::time_point& tp) noexcept {
        using namespace std::chrono;
        system_clock::duration d;
        timespec_to_duration(ts, d);
        tp = system_clock::time_point() + d;
    }

    template <typename TV>
    void timeval_to_timepoint(const TV& tv, std::chrono::system_clock::time_point& tp) noexcept {
        using namespace std::chrono;
        system_clock::duration d;
        timeval_to_duration(tv, d);
        tp = system_clock::time_point() + d;
    }

    #ifdef _WIN32

        namespace Detail {

            void filetime_to_timepoint_helper(const void* ftptr, std::chrono::system_clock::time_point& tp) noexcept;
            void timepoint_to_filetime_helper(const std::chrono::system_clock::time_point& tp, void* ftptr) noexcept;

        }

        template <typename FT>
        void filetime_to_timepoint(const FT& ft, std::chrono::system_clock::time_point& tp) noexcept {
            Detail::filetime_to_timepoint_helper(&ft, tp);
        }

        template <typename FT>
        void timepoint_to_filetime(const std::chrono::system_clock::time_point& tp, FT& ft) noexcept {
            Detail::timepoint_to_filetime_helper(tp, &ft);
        }

    #endif

}
