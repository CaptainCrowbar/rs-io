#pragma once

#include "rs-channel/channel.hpp"
#include <atomic>
#include <string>

namespace RS::Channel {

    // Spawned process channels

    class StreamProcess:
    public StreamChannel {
    public:
        explicit StreamProcess(const std::string& cmd);
        ~StreamProcess() noexcept override { do_close(); }
        StreamProcess(const StreamProcess&) = delete;
        StreamProcess(StreamProcess&&) = delete;
        StreamProcess& operator=(const StreamProcess&) = delete;
        StreamProcess& operator=(StreamProcess&&) = delete;
        void close() noexcept override { do_close(); }
        bool is_closed() const noexcept override { return ! fp_; }
        size_t read(void* dst, size_t maxlen) override;
        bool wait_for(duration t) override;
        int status() const noexcept { return status_; }
    private:
        std::atomic<FILE*> fp_;
        int status_ = -1;
        void do_close() noexcept;
    };

    class TextProcess:
    public MessageChannel<std::string> {
    public:
        explicit TextProcess(const std::string& cmd): stream_(cmd), buf_() {}
        TextProcess(const TextProcess&) = delete;
        TextProcess(TextProcess&&) = delete;
        TextProcess& operator=(const TextProcess&) = delete;
        TextProcess& operator=(TextProcess&&) = delete;
        void close() noexcept override;
        bool is_closed() const noexcept override { return stream_.is_closed() && buf_.empty(); }
        bool read(std::string& t) override;
        bool wait_for(duration t) override;
        std::string read_all() { return buf_ + stream_.read_all(); }
        int status() const noexcept { return stream_.status(); }
    private:
        StreamProcess stream_;
        std::string buf_;
    };

    // Shell commands

    std::string shell(const std::string& cmd);

}
