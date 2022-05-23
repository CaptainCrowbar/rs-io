#include "rs-io/time.hpp"
#include "rs-unit-test.hpp"
#include <chrono>
#include <ctime>
#include <ratio>

#ifdef _WIN32
    #include <windows.h>
#endif

#ifdef _XOPEN_SOURCE
    #include <sys/time.h>
#endif

using namespace RS::IO;
using namespace std::chrono;
using namespace std::literals;

void test_rs_io_time_point_conversion() {

    using sys_time = system_clock::time_point;
    using hrc_time = high_resolution_clock::time_point;

    sys_time sys1, sys2, sys3;
    hrc_time hrc1, hrc2, hrc3;
    milliseconds ms;

    sys1 = system_clock::now();
    hrc1 = high_resolution_clock::now();
    sys2 = sys1 + 1min;
    hrc2 = hrc1 + 1min;

    TRY(convert_time_point(sys2, sys3));  ms = duration_cast<milliseconds>(sys3 - sys1);  TEST_EQUAL(ms.count(), 60'000);
    TRY(convert_time_point(hrc2, hrc3));  ms = duration_cast<milliseconds>(hrc3 - hrc1);  TEST_EQUAL(ms.count(), 60'000);
    TRY(convert_time_point(hrc2, sys3));  ms = duration_cast<milliseconds>(sys3 - sys1);  TEST_NEAR(ms.count(), 60'000, 50);
    TRY(convert_time_point(sys2, hrc3));  ms = duration_cast<milliseconds>(hrc3 - hrc1);  TEST_NEAR(ms.count(), 60'000, 50);

}

void test_rs_io_time_system_specific_conversions() {

    using fsec = duration<double>;
    using days = duration<int64_t, std::ratio<86400>>;

    milliseconds ms = {};
    seconds s = {};
    days d = {};
    fsec fs = {};
    timespec ts = {};
    timeval tv = {};

    ts = {0, 0};                 TRY(timespec_to_duration(ts, fs));  TEST_EQUAL(fs.count(), 0);
    ts = {0, 0};                 TRY(timespec_to_duration(ts, d));   TEST_EQUAL(d.count(), 0);
    ts = {0, 0};                 TRY(timespec_to_duration(ts, ms));  TEST_EQUAL(ms.count(), 0);
    ts = {0, 0};                 TRY(timespec_to_duration(ts, s));   TEST_EQUAL(s.count(), 0);
    ts = {0, 125'000'000};       TRY(timespec_to_duration(ts, fs));  TEST_EQUAL(fs.count(), 0.125);
    ts = {0, 125'000'000};       TRY(timespec_to_duration(ts, d));   TEST_EQUAL(d.count(), 0);
    ts = {0, 125'000'000};       TRY(timespec_to_duration(ts, ms));  TEST_EQUAL(ms.count(), 125);
    ts = {0, 125'000'000};       TRY(timespec_to_duration(ts, s));   TEST_EQUAL(s.count(), 0);
    ts = {86'400, 0};            TRY(timespec_to_duration(ts, fs));  TEST_EQUAL(fs.count(), 86'400);
    ts = {86'400, 0};            TRY(timespec_to_duration(ts, d));   TEST_EQUAL(d.count(), 1);
    ts = {86'400, 0};            TRY(timespec_to_duration(ts, ms));  TEST_EQUAL(ms.count(), 86'400'000);
    ts = {86'400, 0};            TRY(timespec_to_duration(ts, s));   TEST_EQUAL(s.count(), 86'400);
    ts = {86'400, 125'000'000};  TRY(timespec_to_duration(ts, fs));  TEST_EQUAL(fs.count(), 86'400.125);
    ts = {86'400, 125'000'000};  TRY(timespec_to_duration(ts, d));   TEST_EQUAL(d.count(), 1);
    ts = {86'400, 125'000'000};  TRY(timespec_to_duration(ts, ms));  TEST_EQUAL(ms.count(), 86'400'125);
    ts = {86'400, 125'000'000};  TRY(timespec_to_duration(ts, s));   TEST_EQUAL(s.count(), 86'400);
    tv = {0, 0};                 TRY(timeval_to_duration(tv, fs));   TEST_EQUAL(fs.count(), 0);
    tv = {0, 0};                 TRY(timeval_to_duration(tv, d));    TEST_EQUAL(d.count(), 0);
    tv = {0, 0};                 TRY(timeval_to_duration(tv, ms));   TEST_EQUAL(ms.count(), 0);
    tv = {0, 0};                 TRY(timeval_to_duration(tv, s));    TEST_EQUAL(s.count(), 0);
    tv = {0, 125'000};           TRY(timeval_to_duration(tv, fs));   TEST_EQUAL(fs.count(), 0.125);
    tv = {0, 125'000};           TRY(timeval_to_duration(tv, d));    TEST_EQUAL(d.count(), 0);
    tv = {0, 125'000};           TRY(timeval_to_duration(tv, ms));   TEST_EQUAL(ms.count(), 125);
    tv = {0, 125'000};           TRY(timeval_to_duration(tv, s));    TEST_EQUAL(s.count(), 0);
    tv = {86'400, 0};            TRY(timeval_to_duration(tv, fs));   TEST_EQUAL(fs.count(), 86'400);
    tv = {86'400, 0};            TRY(timeval_to_duration(tv, d));    TEST_EQUAL(d.count(), 1);
    tv = {86'400, 0};            TRY(timeval_to_duration(tv, ms));   TEST_EQUAL(ms.count(), 86'400'000);
    tv = {86'400, 0};            TRY(timeval_to_duration(tv, s));    TEST_EQUAL(s.count(), 86'400);
    tv = {86'400, 125'000};      TRY(timeval_to_duration(tv, fs));   TEST_EQUAL(fs.count(), 86'400.125);
    tv = {86'400, 125'000};      TRY(timeval_to_duration(tv, d));    TEST_EQUAL(d.count(), 1);
    tv = {86'400, 125'000};      TRY(timeval_to_duration(tv, ms));   TEST_EQUAL(ms.count(), 86'400'125);
    tv = {86'400, 125'000};      TRY(timeval_to_duration(tv, s));    TEST_EQUAL(s.count(), 86'400);

    fs = fsec(0);                   TRY(duration_to_timespec(fs, ts));  TEST_EQUAL(ts.tv_sec, 0);       TEST_EQUAL(ts.tv_nsec, 0);
    d = days(0);                    TRY(duration_to_timespec(d, ts));   TEST_EQUAL(ts.tv_sec, 0);       TEST_EQUAL(ts.tv_nsec, 0);
    ms = milliseconds(0);           TRY(duration_to_timespec(ms, ts));  TEST_EQUAL(ts.tv_sec, 0);       TEST_EQUAL(ts.tv_nsec, 0);
    s = seconds(0);                 TRY(duration_to_timespec(s, ts));   TEST_EQUAL(ts.tv_sec, 0);       TEST_EQUAL(ts.tv_nsec, 0);
    fs = fsec(0.125);               TRY(duration_to_timespec(fs, ts));  TEST_EQUAL(ts.tv_sec, 0);       TEST_EQUAL(ts.tv_nsec, 125'000'000);
    ms = milliseconds(125);         TRY(duration_to_timespec(ms, ts));  TEST_EQUAL(ts.tv_sec, 0);       TEST_EQUAL(ts.tv_nsec, 125'000'000);
    fs = fsec(86'400);              TRY(duration_to_timespec(fs, ts));  TEST_EQUAL(ts.tv_sec, 86'400);  TEST_EQUAL(ts.tv_nsec, 0);
    d = days(1);                    TRY(duration_to_timespec(d, ts));   TEST_EQUAL(ts.tv_sec, 86'400);  TEST_EQUAL(ts.tv_nsec, 0);
    ms = milliseconds(86'400'000);  TRY(duration_to_timespec(ms, ts));  TEST_EQUAL(ts.tv_sec, 86'400);  TEST_EQUAL(ts.tv_nsec, 0);
    s = seconds(86'400);            TRY(duration_to_timespec(s, ts));   TEST_EQUAL(ts.tv_sec, 86'400);  TEST_EQUAL(ts.tv_nsec, 0);
    fs = fsec(86'400.125);          TRY(duration_to_timespec(fs, ts));  TEST_EQUAL(ts.tv_sec, 86'400);  TEST_EQUAL(ts.tv_nsec, 125'000'000);
    ms = milliseconds(86'400'125);  TRY(duration_to_timespec(ms, ts));  TEST_EQUAL(ts.tv_sec, 86'400);  TEST_EQUAL(ts.tv_nsec, 125'000'000);
    fs = fsec(0);                   TRY(duration_to_timeval(fs, tv));   TEST_EQUAL(tv.tv_sec, 0);       TEST_EQUAL(tv.tv_usec, 0);
    d = days(0);                    TRY(duration_to_timeval(d, tv));    TEST_EQUAL(tv.tv_sec, 0);       TEST_EQUAL(tv.tv_usec, 0);
    ms = milliseconds(0);           TRY(duration_to_timeval(ms, tv));   TEST_EQUAL(tv.tv_sec, 0);       TEST_EQUAL(tv.tv_usec, 0);
    s = seconds(0);                 TRY(duration_to_timeval(s, tv));    TEST_EQUAL(tv.tv_sec, 0);       TEST_EQUAL(tv.tv_usec, 0);
    fs = fsec(0.125);               TRY(duration_to_timeval(fs, tv));   TEST_EQUAL(tv.tv_sec, 0);       TEST_EQUAL(tv.tv_usec, 125'000);
    ms = milliseconds(125);         TRY(duration_to_timeval(ms, tv));   TEST_EQUAL(tv.tv_sec, 0);       TEST_EQUAL(tv.tv_usec, 125'000);
    fs = fsec(86'400);              TRY(duration_to_timeval(fs, tv));   TEST_EQUAL(tv.tv_sec, 86'400);  TEST_EQUAL(tv.tv_usec, 0);
    d = days(1);                    TRY(duration_to_timeval(d, tv));    TEST_EQUAL(tv.tv_sec, 86'400);  TEST_EQUAL(tv.tv_usec, 0);
    ms = milliseconds(86'400'000);  TRY(duration_to_timeval(ms, tv));   TEST_EQUAL(tv.tv_sec, 86'400);  TEST_EQUAL(tv.tv_usec, 0);
    s = seconds(86'400);            TRY(duration_to_timeval(s, tv));    TEST_EQUAL(tv.tv_sec, 86'400);  TEST_EQUAL(tv.tv_usec, 0);
    fs = fsec(86'400.125);          TRY(duration_to_timeval(fs, tv));   TEST_EQUAL(tv.tv_sec, 86'400);  TEST_EQUAL(tv.tv_usec, 125'000);
    ms = milliseconds(86'400'125);  TRY(duration_to_timeval(ms, tv));   TEST_EQUAL(tv.tv_sec, 86'400);  TEST_EQUAL(tv.tv_usec, 125'000);

    #ifdef _WIN32

        static constexpr int64_t epoch = 11'644'473'600ll;
        static constexpr int64_t freq = 10'000'000ll;

        int64_t ticks = 0;
        FILETIME ft1, ft2;
        system_clock::time_point tp1, tp2;
        system_clock::duration du;

        ticks = epoch * freq;
        ft1 = {uint32_t(ticks), uint32_t(ticks >> 32)};
        TRY(filetime_to_timepoint(ft1, tp1));
        du = tp1 - system_clock::from_time_t(0);
        TEST_EQUAL(du.count(), 0);
        TRY(timepoint_to_filetime(tp1, ft2));
        TEST_EQUAL(ft2.dwHighDateTime, ft1.dwHighDateTime);
        TEST_EQUAL(ft2.dwLowDateTime, ft1.dwLowDateTime);

        ticks += 86'400 * freq;
        ft1 = {uint32_t(ticks), uint32_t(ticks >> 32)};
        TRY(filetime_to_timepoint(ft1, tp1));
        du = tp1 - system_clock::from_time_t(0);
        TEST_EQUAL(duration_cast<milliseconds>(du).count(), 86'400'000);

        tp1 = system_clock::from_time_t(0);
        TRY(timepoint_to_filetime(tp1, ft1));
        TRY(filetime_to_timepoint(ft1, tp2));
        TEST_EQUAL(tp2.time_since_epoch().count(), tp1.time_since_epoch().count());

        tp1 = system_clock::from_time_t(1'234'567'890);
        TRY(timepoint_to_filetime(tp1, ft1));
        TRY(filetime_to_timepoint(ft1, tp2));
        TEST_EQUAL(tp2.time_since_epoch().count(), tp1.time_since_epoch().count());

    #endif

}
