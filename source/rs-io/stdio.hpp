#pragma once

#include "rs-io/path.hpp"
#include "rs-io/utility.hpp"
#include "rs-format/format.hpp"
#include "rs-format/string.hpp"
#include <cstdio>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace RS::IO {

    // I/O abstract base class

    class IoBase {
    public:

        class line_iterator:
        public InputIterator<line_iterator, const std::string> {
        public:
            line_iterator() = default;
            explicit line_iterator(IoBase& io): iop(&io), line() { ++*this; }
            const std::string& operator*() const noexcept { return line; }
            line_iterator& operator++();
            bool operator==(const line_iterator& rhs) const noexcept { return iop == rhs.iop; }
        private:
            IoBase* iop = nullptr;
            std::string line;
        };

        enum class mode {
            read_only = 1,
            write_only,
            append,
            create_always,
            open_always,
            open_existing,
        };

        virtual ~IoBase() = default;
        IoBase(const IoBase&) = delete;
        IoBase(IoBase&&) = default;
        IoBase& operator=(const IoBase&) = delete;
        IoBase& operator=(IoBase&&) = default;

        virtual void close() noexcept = 0;
        virtual void flush() noexcept {}
        virtual int getc() noexcept;
        virtual Path get_path() const { return {}; }
        virtual bool is_open() const noexcept = 0;
        virtual bool is_terminal() const noexcept = 0;
        virtual void putc(char c) { write(&c, 1); }
        virtual size_t read(void* ptr, size_t maxlen) noexcept = 0;
        virtual std::string read_line();
        virtual void seek(ptrdiff_t offset, int which = SEEK_CUR) noexcept = 0;
        virtual ptrdiff_t tell() noexcept = 0;
        virtual size_t write(const void* ptr, size_t len) = 0;
        virtual void write_n(size_t n, char c);

        void check(const std::string& detail = "") const;
        void clear_error() noexcept { set_error(0); }
        std::error_code error() const noexcept { return status_; }
        template <typename... Args> void format(const std::string& pattern, Args&&... args);
        Irange<line_iterator> lines() { return {line_iterator(*this), {}}; }
        bool ok() const noexcept { return ! status_ && is_open(); }
        template <typename... Args> void print(Args&&... args);
        std::string read_all();
        size_t read_n(std::string& s, size_t maxlen = 1024);
        std::string read_str(size_t maxlen);
        void write_line() { putc('\n'); }
        void write_line(std::string_view str);
        size_t write_str(std::string_view str);

    protected:

        static constexpr const char* null_device =
            #ifdef _XOPEN_SOURCE
                "/dev/null";
            #else
                "NUL:";
            #endif

        IoBase() = default;

        void set_error(int err, const std::error_category& cat = std::generic_category()) noexcept;

    private:

        std::error_code status_;

    };

        template <typename... Args>
        void IoBase::format(const std::string& pattern, Args&&... args) {
            write_str(Format::format(pattern, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void IoBase::print(Args&&... args) {
            write_line(Format::prints(std::forward<Args>(args)...));
        }

    // C standard I/O

    class Cstdio:
    public IoBase {

    public:

        using handle_type = FILE*;

        Cstdio() = default;
        explicit Cstdio(FILE* f) noexcept: fp_(f) {}
        explicit Cstdio(const Path& f, mode m = mode::read_only);
        Cstdio(const Path& f, const std::string& iomode);
        Cstdio(const Cstdio&) = delete;
        Cstdio(Cstdio&&) = default;
        Cstdio& operator=(const Cstdio&) = delete;
        Cstdio& operator=(Cstdio&&) = default;

        void close() noexcept override;
        void flush() noexcept override;
        int getc() noexcept override;
        Path get_path() const override;
        bool is_open() const noexcept override { return bool(fp_); }
        bool is_terminal() const noexcept override;
        void putc(char c) override;
        size_t read(void* ptr, size_t maxlen) noexcept override;
        std::string read_line() override;
        void seek(ptrdiff_t offset, int which = SEEK_CUR) noexcept override;
        ptrdiff_t tell() noexcept override;
        size_t write(const void* ptr, size_t len) override;

        int fd() const noexcept;
        FILE* get() const noexcept { return fp_.get(); }
        FILE* release() noexcept { return fp_.release(); }
        void ungetc(char c);

        static Cstdio null() noexcept;
        static Cstdio std_input() noexcept { return Cstdio(stdin); }
        static Cstdio std_output() noexcept { return Cstdio(stdout); }
        static Cstdio std_error() noexcept { return Cstdio(stderr); }

    private:

        struct deleter {
            void operator()(FILE* fp) const noexcept;
        };

        Resource<FILE*, deleter> fp_;

    };

    // File descriptor I/O

    class Fdio:
    public IoBase {

    public:

        using handle_type = int;

        Fdio() = default;
        explicit Fdio(int f) noexcept: fd_(f) {}
        explicit Fdio(const Path& f, mode m = mode::read_only);
        Fdio(const Path& f, int iomode, int perm = 0666);
        Fdio(const Fdio&) = delete;
        Fdio(Fdio&&) = default;
        Fdio& operator=(const Fdio&) = delete;
        Fdio& operator=(Fdio&&) = default;

        void close() noexcept override;
        void flush() noexcept override;
        Path get_path() const override;
        bool is_open() const noexcept override { return bool(fd_); }
        bool is_terminal() const noexcept override;
        size_t read(void* ptr, size_t maxlen) noexcept override;
        void seek(ptrdiff_t offset, int which = SEEK_CUR) noexcept override;
        ptrdiff_t tell() noexcept override;
        size_t write(const void* ptr, size_t len) override;

        Fdio dup() noexcept;
        Fdio dup(int f) noexcept;
        int get() const noexcept { return fd_.get(); }
        int release() noexcept { return fd_.release(); }

        static Fdio null() noexcept;
        static std::pair<Fdio, Fdio> pipe(size_t winmem = 65'536);
        static Fdio std_input() noexcept { return Fdio(0); }
        static Fdio std_output() noexcept { return Fdio(1); }
        static Fdio std_error() noexcept { return Fdio(2); }

    private:

        struct deleter {
            void operator()(int fd) const noexcept;
        };

        Resource<int, deleter, -1> fd_;

        #ifdef _MSC_VER
            using iosize = unsigned;
            using ofsize = long;
        #else
            using iosize = size_t;
            using ofsize = ptrdiff_t;
        #endif

    };

    #ifdef _WIN32

        // Windows file I/O

        class Winio:
        public IoBase {

        public:

            using handle_type = void*;

            Winio() = default;
            explicit Winio(void* f) noexcept: fh_(f) {}
            explicit Winio(const Path& f, mode m = mode::read_only);
            Winio(const Path& f, uint32_t desired_access, uint32_t share_mode, LPSECURITY_ATTRIBUTES security_attributes,
                uint32_t creation_disposition, uint32_t flags_and_attributes = 0, HANDLE template_file = nullptr);
            Winio(const Winio&) = delete;
            Winio(Winio&&) = default;
            Winio& operator=(const Winio&) = delete;
            Winio& operator=(Winio&&) = default;

            void close() noexcept override;
            void flush() noexcept override;
            Path get_path() const override;
            bool is_open() const noexcept override { return bool(fh_); }
            bool is_terminal() const noexcept override;
            size_t read(void* ptr, size_t maxlen) noexcept override;
            void seek(ptrdiff_t offset, int which = SEEK_CUR) noexcept override;
            ptrdiff_t tell() noexcept override;
            size_t write(const void* ptr, size_t len) override;

            void* get() const noexcept { return fh_.get(); }
            void* release() noexcept { return fh_.release(); }

            static Winio null() noexcept;
            static Winio std_input() noexcept { return Winio(GetStdHandle(STD_INPUT_HANDLE), false); }
            static Winio std_output() noexcept { return Winio(GetStdHandle(STD_OUTPUT_HANDLE), false); }
            static Winio std_error() noexcept { return Winio(GetStdHandle(STD_ERROR_HANDLE), false); }

        private:

            struct deleter {
                void operator()(void* fh) const noexcept;
            };

            Resource<void*, deleter> fh_;

        };

    #endif

    // Temporary file

    class TempFile:
    public Cstdio {

    public:

        TempFile();
        TempFile(const Path& dir, const std::string& prefix);
        ~TempFile() noexcept override;
        TempFile(const TempFile&) = delete;
        TempFile(TempFile&&) = default;
        TempFile& operator=(const TempFile&) = delete;
        TempFile& operator=(TempFile&&) = default;

        Path get_path() const override;

    private:

        Path where_;

    };

}
