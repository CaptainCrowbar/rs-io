#include "rs-io/stdio.hpp"
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <random>

#ifdef _XOPEN_SOURCE

    #include <sys/stat.h>
    #include <unistd.h>

    #ifdef __APPLE__
        #include <sys/syslimits.h>
    #endif

    #define IO_FUNCTION(f) ::f

#else

    #include <io.h>
    #include <windows.h>

    #define IO_FUNCTION(f) _##f

#endif

namespace RS::IO {

    namespace {

        constexpr size_t block_size = 65'536;
        constexpr size_t small_block = 256;

    }

    // Class IoBase

    int IoBase::getc() noexcept {
        unsigned char b = 0;
        return read(&b, 1) ? int(b) : EOF;
    }

    std::string IoBase::read_line() {
        std::string s;
        for (;;) {
            int c = getc();
            if (c == EOF)
                break;
            s += char(c);
            if (c == '\n')
                break;
        }
        return s;
    }

    void IoBase::check(const std::string& detail) const {
        if (status_) {
            if (detail.empty())
                throw std::system_error(status_);
            else
                throw std::system_error(status_, std::string(detail));
        }
    }

    std::string IoBase::read_all() {
        std::string buf;
        while (read_some(buf, block_size)) {}
        return buf;
    }

    size_t IoBase::read_some(std::string& buf, size_t maxlen) {
        size_t offset = buf.size();
        buf.resize(offset + maxlen);
        size_t n = read(&buf[0] + offset, maxlen);
        buf.resize(offset + n);
        return n;
    }

    std::string IoBase::reads(size_t maxlen) {
        std::string s(maxlen, 0);
        s.resize(read(&s[0], maxlen));
        return s;
    }

    void IoBase::write_chars(size_t n, char c) {
        std::string buf(std::min(n, block_size), c);
        size_t quo = n / block_size;
        size_t rem = n % block_size;
        for (size_t i = 0; i < quo; ++i)
            write(buf.data(), block_size);
        if (rem != 0)
            write(buf.data(), rem);
    }

    void IoBase::write_line(const std::string& str) {
        if (str.empty())
            putc('\n');
        else if (str.back() == '\n')
            writes(str);
        else
            writes(std::string(str) + '\n');
    }

    size_t IoBase::writes(const std::string& str) {
        static constexpr char dummy = '\0';
        const char* ptr = str.empty() ? &dummy : str.data();
        size_t ofs = 0, len = str.size();
        do ofs += write(ptr + ofs, len - ofs);
            while (ofs < len && ok());
        return ofs;
    }

    void IoBase::set_error(int err, const std::error_category& cat) noexcept {
        if (err)
            status_ = {err, cat};
        else
            status_.clear();
    }

    IoBase::line_iterator& IoBase::line_iterator::operator++() {
        if (iop) {
            line = iop->read_line();
            if (line.empty())
                iop = nullptr;
        }
        return *this;
    }

    // Class Cstdio

    void Cstdio::deleter::operator()(FILE* fp) const noexcept {
        if (fp != stdin && fp != stdout && fp != stderr)
            std::fclose(fp);
    }

    Cstdio::Cstdio(const Path& f, mode m) {
        if (f.empty() || f.name() == "-") {
            if (m == mode::read_only) {
                *this = std_input();
                return;
            } else if (m == mode::write_only || m == mode::append) {
                *this = std_output();
                return;
            }
        } else if (m == mode::open_always) {
            // There is no C stdio mode corresponding directly to the
            // behaviour we want from IoBase::mode::open_always. Try "rb+"
            // first(open for R/W if the file exists, otherwise fail), and if
            // that fails, try "wb+" (open for R/W regardless, destroying any
            // existing file). There's a race condition here but I don't
            // think there's any way around it.
            std::string iomode = "rb+";
            *this = Cstdio(f, "rb+");
            if (! fp_)
                *this = Cstdio(f, "wb+");
        } else {
            std::string cmode;
            switch (m) {
                case mode::read_only:      cmode = "rb"; break;
                case mode::write_only:     cmode = "wb"; break;
                case mode::append:         cmode = "ab"; break;
                case mode::create_always:  cmode = "wb+"; break;
                case mode::open_existing:  cmode = "rb+"; break;
                default:                   break;
            }
            *this = Cstdio(f, cmode);
        }
    }

    Cstdio::Cstdio(const Path& f, const std::string& iomode) {
        #ifdef _XOPEN_SOURCE
            errno = 0;
            auto rc = ::fopen(f.c_name(), iomode.data());
        #else
            auto wmode = to_wstring(iomode);
            errno = 0;
            auto rc = _wfopen(f.c_name(), wmode.data());
        #endif
        int err = errno;
        *this = Cstdio(rc);
        set_error(err);
    }

    void Cstdio::close() noexcept {
        errno = 0;
        fp_.reset();
        set_error(errno);
    }

    void Cstdio::flush() noexcept {
        errno = 0;
        ::fflush(fp_);
        set_error(errno);
    }

    int Cstdio::getc() noexcept {
        errno = 0;
        int c = ::fgetc(fp_);
        set_error(errno);
        return c;
    }

    void Cstdio::putc(char c) {
        errno = 0;
        ::fputc(int(uint8_t(c)), fp_);
        set_error(errno);
    }

    size_t Cstdio::read(void* ptr, size_t maxlen) noexcept {
        errno = 0;
        size_t n = ::fread(ptr, 1, maxlen, fp_);
        set_error(errno);
        return n;
    }

    std::string Cstdio::read_line() {
        std::string buf;
        for (;;) {
            size_t offset = buf.size();
            buf.resize(offset + small_block, '\0');
            errno = 0;
            auto rc = ::fgets(&buf[0] + offset, small_block, fp_);
            set_error(errno);
            if (rc == nullptr)
                return buf.substr(0, offset);
            size_t lfpos = buf.find('\n', offset);
            if (lfpos != npos)
                return buf.substr(0, lfpos + 1);
            size_t ntpos = buf.find_last_not_of('\0') + 1;
            if (ntpos < small_block - 1)
                return buf.substr(0, ntpos);
            buf.pop_back();
        }
    }

    void Cstdio::seek(ptrdiff_t offset, int which) noexcept {
        errno = 0;
        #ifdef _XOPEN_SOURCE
            ::fseeko(fp_, offset, which);
        #else
            _fseeki64(fp_, offset, which);
        #endif
        set_error(errno);
    }

    ptrdiff_t Cstdio::tell() noexcept {
        errno = 0;
        #ifdef _XOPEN_SOURCE
            ptrdiff_t offset = ::ftello(fp_);
        #else
            ptrdiff_t offset = _ftelli64(fp_);
        #endif
        set_error(errno);
        return offset;
    }

    size_t Cstdio::write(const void* ptr, size_t len) {
        errno = 0;
        size_t n = ::fwrite(ptr, 1, len, fp_);
        set_error(errno);
        return n;
    }

    int Cstdio::fd() const noexcept {
        return fp_ ? IO_FUNCTION(fileno)(fp_) : -1;
    }

    void Cstdio::ungetc(char c) {
        errno = 0;
        ::ungetc(int(uint8_t(c)), fp_);
        set_error(errno);
    }

    Cstdio Cstdio::null() noexcept {
        Cstdio io(null_device, "r+");
        ::fgetc(io.fp_); // Clear bogus ioctl error
        return io;
    }

    // Class Fdio

    void Fdio::deleter::operator()(int fd) const noexcept {
        if (fd > 2)
            IO_FUNCTION(close)(fd);
    }

    Fdio::Fdio(const Path& f, mode m) {
        if (f.empty() || f.name() == "-") {
            if (m == mode::read_only) {
                *this = std_input();
                return;
            } else if (m == mode::write_only || m == mode::append) {
                *this = std_output();
                return;
            }
        }
        int fmode = 0;
        switch (m) {
            case mode::read_only:      fmode = O_RDONLY; break;
            case mode::write_only:     fmode = O_WRONLY|O_CREAT|O_TRUNC; break;
            case mode::append:         fmode = O_WRONLY|O_APPEND|O_CREAT; break;
            case mode::create_always:  fmode = O_RDWR|O_CREAT|O_TRUNC; break;
            case mode::open_always:    fmode = O_RDWR|O_CREAT; break;
            case mode::open_existing:  fmode = O_RDWR; break;
            default:                   break;
        }
        *this = Fdio(f, fmode, 0666);
    }

    Fdio::Fdio(const Path& f, int iomode, int perm) {
        #ifdef _XOPEN_SOURCE
            #ifdef __CYGWIN__
                if (! (iomode & (O_BINARY | O_TEXT)))
                    iomode |= O_BINARY;
            #endif
            errno = 0;
            auto rc = ::open(f.c_name(), iomode, perm);
        #else
            if (! (iomode & (_O_BINARY | _O_TEXT | _O_U8TEXT | _O_U16TEXT | _O_WTEXT)))
                iomode |= _O_BINARY;
            errno = 0;
            auto rc = _wopen(f.c_name(), iomode, perm);
        #endif
        int err = errno;
        *this = Fdio(rc);
        set_error(err);
    }

    void Fdio::close() noexcept {
        errno = 0;
        fd_.reset();
        set_error(errno);
    }

    void Fdio::flush() noexcept {
        #ifdef _XOPEN_SOURCE
            errno = 0;
            ::fsync(fd_);
            set_error(errno);
        #else
            auto h = reinterpret_cast<HANDLE>(_get_osfhandle(fd_));
            if (h == INVALID_HANDLE_VALUE)
                set_error(EBADF);
            else if (! FlushFileBuffers(h))
                set_error(GetLastError(), std::system_category());
        #endif
    }

    size_t Fdio::read(void* ptr, size_t maxlen) noexcept {
        errno = 0;
        auto rc = IO_FUNCTION(read)(fd_, ptr, iosize(maxlen));
        set_error(errno);
        return rc;
    }

    void Fdio::seek(ptrdiff_t offset, int which) noexcept {
        errno = 0;
        IO_FUNCTION(lseek)(fd_, ofsize(offset), which);
        set_error(errno);
    }

    ptrdiff_t Fdio::tell() noexcept {
        errno = 0;
        auto offset = IO_FUNCTION(lseek)(fd_, 0, SEEK_CUR);
        set_error(errno);
        return offset;
    }

    size_t Fdio::write(const void* ptr, size_t len) {
        errno = 0;
        size_t n = IO_FUNCTION(write)(fd_, ptr, iosize(len));
        set_error(errno);
        return n;
    }

    Fdio Fdio::dup() noexcept {
        errno = 0;
        int rc = IO_FUNCTION(dup)(fd_);
        set_error(errno);
        return Fdio(rc);
    }

    Fdio Fdio::dup(int f) noexcept {
        errno = 0;
        IO_FUNCTION(dup2)(fd_, f);
        set_error(errno);
        return Fdio(f);
    }

    Fdio Fdio::null() noexcept {
        int iomode = O_RDWR;
        #ifdef O_CLOEXEC
            iomode |= O_CLOEXEC;
        #endif
        return Fdio(null_device, iomode);
    }

    std::pair<Fdio, Fdio> Fdio::pipe(size_t winmem) {
        int fds[2];
        int rc = 0;
        errno = 0;
        #ifdef _XOPEN_SOURCE
            (void)winmem;
            rc = ::pipe(fds);
        #else
            rc = _pipe(fds, iosize(winmem), O_BINARY);
        #endif
        int err = errno;
        std::pair<Fdio, Fdio> pair;
        pair.first = Fdio(fds[0]);
        pair.second = Fdio(fds[1]);
        if (rc) {
            pair.first.set_error(err);
            pair.second.set_error(err);
        }
        return pair;
    }

    #ifdef _WIN32

        // Class Winio

        void Winio::deleter::operator()(void* fh) const noexcept {
            sattic const auto stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
            sattic const auto stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
            sattic const auto stderr_handle = GetStdHandle(STD_ERROR_HANDLE);
            if (fh != INVALID_HANDLE_VALUE && fh != stdin_handle && fh != stdout_handle && fh != stderr_handle)
                CloseHandle(fh);
        }

        Winio::Winio(const Path& f, mode m) {
            if (f.empty() || f.name() == "-") {
                if (m == mode::read_only) {
                    *this = std_input();
                    return;
                } else if (m == mode::write_only || m == mode::append) {
                    *this = std_output();
                    return;
                }
            }
            uint32_t access = 0, creation = 0;
            switch (m) {
                case mode::read_only:      access = GENERIC_READ; creation = OPEN_EXISTING; break;
                case mode::write_only:     access = GENERIC_WRITE; creation = CREATE_ALWAYS; break;
                case mode::append:         access = GENERIC_WRITE; creation = OPEN_ALWAYS; break;
                case mode::create_always:  access = GENERIC_READ | GENERIC_WRITE; creation = CREATE_ALWAYS; break;
                case mode::open_always:    access = GENERIC_READ | GENERIC_WRITE; creation = OPEN_ALWAYS; break;
                case mode::open_existing:  access = GENERIC_READ | GENERIC_WRITE; creation = OPEN_EXISTING; break;
                default:                   break;
            }
            *this = Winio(f, access, 0, nullptr, creation);
            if (m == mode::append) {
                LARGE_INTEGER distance;
                distance.QuadPart = 0;
                SetLastError(0);
                SetFilePointerEx(fh_, distance, nullptr, FILE_END);
                set_error(GetLastError(), std::system_category());
            }
        }

        Winio::Winio(const Path& f, uint32_t desired_access, uint32_t share_mode, LPSECURITY_ATTRIBUTES security_attributes,
                uint32_t creation_disposition, uint32_t flags_and_attributes, HANDLE template_file) {
            SetLastError(0);
            auto rc = CreateFileW(f.c_name(), desired_access, share_mode, security_attributes, creation_disposition, flags_and_attributes, template_file);
            int err = GetLastError();
            *this = Winio(rc);
            set_error(err, std::system_category());
        }

        void Winio::close() noexcept {
            SetLastError(0);
            fh_.reset();
            set_error(GetLastError(), std::system_category());
        }

        void Winio::flush() noexcept {
            if (! FlushFileBuffers(fh_))
                set_error(GetLastError(), std::system_category());
        }

        size_t Winio::read(void* ptr, size_t maxlen) noexcept {
            DWORD n = 0;
            SetLastError(0);
            ReadFile(fh_, ptr, uint32_t(maxlen), &n, nullptr);
            set_error(GetLastError(), std::system_category());
            return n;
        }

        void Winio::seek(ptrdiff_t offset, int which) noexcept {
            LARGE_INTEGER distance;
            distance.QuadPart = offset;
            DWORD method = 0;
            switch (which) {
                case SEEK_SET:  method = FILE_BEGIN; break;
                case SEEK_CUR:  method = FILE_CURRENT; break;
                case SEEK_END:  method = FILE_END; break;
                default:        break;
            }
            SetLastError(0);
            SetFilePointerEx(fh_, distance, nullptr, method);
            set_error(GetLastError(), std::system_category());
        }

        ptrdiff_t Winio::tell() noexcept {
            LARGE_INTEGER distance, result;
            distance.QuadPart = result.QuadPart = 0;
            SetLastError(0);
            SetFilePointerEx(fh_, distance, &result, FILE_CURRENT);
            set_error(GetLastError(), std::system_category());
            return result.QuadPart;
        }

        size_t Winio::write(const void* ptr, size_t len) {
            DWORD n = 0;
            SetLastError(0);
            WriteFile(fh_, ptr, uint32_t(len), &n, nullptr);
            set_error(GetLastError(), std::system_category());
            return n;
        }

        Winio Winio::null() noexcept {
            return Winio(null_device, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING);
        }

    #endif

    // Class TempFile

    TempFile::TempFile() {
        errno = 0;
        auto file = std::tmpfile();
        auto err = errno;
        if (! file)
            throw std::system_error(err, std::generic_category());
        Cstdio io(file);
        Cstdio::operator=(std::move(io));
    }

    TempFile::TempFile(const Path& dir, const std::string& prefix) {
        static constexpr int max_tries = 100;
        if (! dir.empty() && ! dir.is_directory())
            throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
        std::random_device dev;
        Fdio fdio;
        for (int i = 0; i < max_tries && ! fdio.ok(); ++i) {
            uint64_t x = dev();
            uint64_t y = dev();
            uint64_t z = (x << 32) + y;
            where_ = dir / (std::string(prefix) + Format::hex(z));
            where_ = where_.resolve();
            fdio = Fdio(where_, O_RDWR | O_CREAT | O_EXCL);
        }
        fdio.check();
        FILE* file = nullptr;
        #if defined(_WIN32) && ! defined(__CYGWIN__)
            file = _wfdopen(fdio.release(), L"rb+");
        #else
            file = fdopen(fdio.release(), "rb+");
        #endif
        Cstdio io(file);
        Cstdio::operator=(std::move(io));
    }

    TempFile::~TempFile() noexcept {
        if (! where_.empty()) {
            try { close(); } catch (...) {}
            try { where_.remove(); } catch (...) {}
        }
    }

}
