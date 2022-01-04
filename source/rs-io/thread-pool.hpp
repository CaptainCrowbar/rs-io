#pragma once

#include "rs-io/utility.hpp"
#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace RS::IO {

    class ThreadPool {
    public:
        using callback = std::function<void()>;
        using clock = std::chrono::system_clock;
        ThreadPool(): ThreadPool(0) {}
        explicit ThreadPool(size_t threads);
        ~ThreadPool() noexcept;
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;
        void clear() noexcept;
        void insert(const callback& call);
        void insert(callback&& call);
        bool poll() { return ! unfinished_jobs_; }
        size_t threads() const noexcept { return workers_.size(); }
        void wait() noexcept;
        template <typename R, typename P> bool wait_for(std::chrono::duration<R, P> t) noexcept { return wait_until(clock::now() + t); }
        bool wait_until(clock::time_point t) noexcept;
    private:
        struct worker {
            std::mutex mutex;
            std::deque<callback> queue;
            std::thread thread;
        };
        std::atomic<size_t> clear_count_;
        std::atomic<size_t> next_worker_;
        std::atomic<size_t> unfinished_jobs_;
        std::atomic<bool> shutting_down_;
        std::vector<worker> workers_;
        void thread_payload(worker& work) noexcept;
        static size_t adjust_threads(size_t threads) noexcept;
    };

}
