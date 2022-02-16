#pragma once

#include "rs-io/utility.hpp"
#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace RS::IO {

    class ThreadPool {

    public:

        using clock = std::chrono::system_clock;

        ThreadPool(): ThreadPool(0) {}
        explicit ThreadPool(int threads);
        ~ThreadPool() noexcept;
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;

        void clear() noexcept;
        template <typename F> void insert(F&& f);
        bool poll() { return ! unfinished_jobs_; }
        int threads() const noexcept { return int(workers_.size()); }
        void wait() noexcept;
        template <typename R, typename P> bool wait_for(std::chrono::duration<R, P> t) noexcept;
        bool wait_until(clock::time_point t) noexcept;

    private:

        using callback = std::function<void()>;

        struct worker {
            std::mutex mutex;
            std::deque<callback> queue;
            std::thread thread;
        };

        std::atomic<int> clear_count_;
        std::atomic<int> next_worker_;
        std::atomic<int> unfinished_jobs_;
        std::atomic<bool> shutting_down_;
        std::vector<worker> workers_;

        static int adjust_threads(int threads) noexcept;
        static void thread_payload(ThreadPool* pool, worker* work) noexcept;

    };

        template <typename F>
        void ThreadPool::insert(F&& f) {
            if (clear_count_)
                return;
            int index = next_worker_;
            next_worker_ = (index + 1) % threads();
            auto& work = workers_[index];
            std::unique_lock lock(work.mutex);
            ++unfinished_jobs_;
            work.queue.emplace_back(std::forward<F>(f));
        }

        template <typename R, typename P>
        bool ThreadPool::wait_for(std::chrono::duration<R, P> t) noexcept {
            return wait_until(clock::now() + t);
        }

}
