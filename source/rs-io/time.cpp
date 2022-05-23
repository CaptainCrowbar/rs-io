#include "rs-io/time.hpp"

#ifdef _WIN32
    #include <windows.h>
#endif

#ifdef _XOPEN_SOURCE
    #include <sys/time.h>
#endif

using namespace std::chrono;

namespace RS::IO {

    namespace Detail {

        #ifdef _XOPEN_SOURCE
            using TvSec = time_t;
            using TvUsec = suseconds_t;
        #else
            using TvSec = long;
            using TvUsec = long;
        #endif

        void make_timeval(int64_t us, void* tvptr) noexcept {
            static constexpr int64_t M = 1'000'000;
            auto& tv = *static_cast<timeval*>(tvptr);
            tv = {TvSec(us / M), TvUsec(us % M)};
        }

        void read_timeval(const void* tvptr, seconds& s, microseconds& us) noexcept {
            auto& tv = *static_cast<const timeval*>(tvptr);
            s = seconds(tv.tv_sec);
            us = microseconds(tv.tv_usec);
        }

        #ifdef _WIN32

            void filetime_to_timepoint_helper(const void* ftptr, system_clock::time_point& tp) noexcept {
                static constexpr int64_t filetime_freq = 10'000'000;        // FILETIME ticks (100 ns) per second
                static constexpr int64_t windows_epoch = 11'644'473'600ll;  // Windows epoch (1601) to Unix epoch (1970) in seconds
                auto& ft = *static_cast<const FILETIME*>(ftptr);
                int64_t ticks = (int64_t(ft.dwHighDateTime) << 32) + int64_t(ft.dwLowDateTime);
                int64_t sec = ticks / filetime_freq - windows_epoch;
                int64_t nsec = 100ll * (ticks % filetime_freq);
                tp = system_clock::from_time_t(time_t(sec)) + duration_cast<system_clock::duration>(nanoseconds(nsec));
            }

            void timepoint_to_filetime_helper(const system_clock::time_point& tp, void* ftptr) noexcept {
                static constexpr uint64_t filetime_freq = 10'000'000;        // FILETIME ticks (100 ns) per second
                static constexpr uint64_t windows_epoch = 11'644'473'600ll;  // Windows epoch (1601) to Unix epoch (1970) in seconds
                auto& ft = *static_cast<FILETIME*>(ftptr);
                auto unix_time = tp - system_clock::from_time_t(0);
                uint64_t nsec = duration_cast<nanoseconds>(unix_time).count();
                uint64_t ticks = nsec / 100ll + filetime_freq * windows_epoch;
                ft = {uint32_t(ticks & 0xfffffffful), uint32_t(ticks >> 32)};
            }

        #endif

    }

}
