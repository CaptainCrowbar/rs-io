#pragma once

#include "rs-io/utility.hpp"
#include "rs-format/unicode.hpp"
#include "rs-tl/iterator.hpp"
#include "rs-tl/types.hpp"
#include <chrono>
#include <functional>
#include <iterator>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace RS::IO {

    class Path:
    public TL::TotalOrder<Path> {

    public:

        // Types

        #ifdef _XOPEN_SOURCE
            using character_type = char;
        #else
            using character_type = wchar_t;
        #endif

        class directory_iterator;
        class search_iterator;

        using search_range = TL::Irange<search_iterator>;
        using directory_range = TL::Irange<directory_iterator>;
        using id_type = std::pair<uint64_t, uint64_t>;
        using string_type = std::basic_string<character_type>;
        using time_point = std::chrono::system_clock::time_point;

        enum class form {
            empty,
            absolute,
            drive_absolute,
            drive_relative,
            relative,
        };

        // Constants

        #ifdef _XOPEN_SOURCE
            static constexpr char delimiter = '/';
            static constexpr char native_delimiter = '/';
        #else
            static constexpr char delimiter = '\\';
            static constexpr wchar_t native_delimiter = L'\\';
        #endif

        #if defined(__APPLE__) || defined(_WIN32)
            static constexpr bool native_case = false;
        #else
            static constexpr bool native_case = true;
        #endif

        static constexpr int append       = 1 << 0;   // Append if file exists
        static constexpr int bottom_up    = 1 << 1;   // Bottom up order
        static constexpr int legal_name   = 1 << 2;   // Throw on illegal file name
        static constexpr int may_copy     = 1 << 3;   // Copy if operation not allowed
        static constexpr int may_fail     = 1 << 4;   // Empty string if read fails
        static constexpr int no_follow    = 1 << 5;   // Don't follow symlinks
        static constexpr int no_hidden    = 1 << 6;   // Skip hidden files
        static constexpr int overwrite    = 1 << 7;   // Delete existing file if necessary
        static constexpr int recurse      = 1 << 8;   // Recursive directory operations
        static constexpr int std_default  = 1 << 9;   // Use stdin/out if file is "" or "-"
        static constexpr int unicode      = 1 << 10;  // Skip files with non-Unicode names

        // Comparison objects

        enum class cmp {
            cased,
            icase,
            native = native_case ? cased : icase,
        };

        class equal {
        public:
            equal() = default;
            explicit equal(cmp mode) noexcept: cmode(mode) {}
            bool operator()(const Path& lhs, const Path& rhs) const;
        private:
            cmp cmode = cmp::native;
        };

        class less {
        public:
            less() = default;
            explicit less(cmp mode) noexcept: cmode(mode) {}
            bool operator()(const Path& lhs, const Path& rhs) const;
        private:
            cmp cmode = cmp::native;
        };

        // Life cycle functions

        Path() = default;
        Path(const std::string& file, int flags = 0);
        Path(const char* file, int flags = 0): Path(std::string(file), flags) {}

        #ifdef _WIN32
            Path(const std::wstring& file, int flags = 0);
            Path(const wchar_t* file, int flags = 0): Path(std::wstring(file), flags) {}
        #endif

        // Path name functions

        std::string name() const;
        std::string str() const { return name(); }
        string_type os_name() const { return filename_; }
        const character_type* c_name() const noexcept { return filename_.data(); }

        std::string as_url() const;
        std::vector<std::string> breakdown() const;
        std::vector<string_type> os_breakdown() const;
        Path change_ext(const std::string& new_ext) const;
        bool empty() const noexcept { return filename_.empty(); }
        size_t hash() const noexcept { return std::hash<string_type>()(filename_); }

        bool is_absolute() const noexcept;
        bool is_drive_absolute() const noexcept;
        bool is_drive_relative() const noexcept;
        bool is_relative() const noexcept;
        bool is_leaf() const noexcept;
        bool is_legal() const noexcept;
        bool is_root() const noexcept;
        bool is_unicode() const noexcept { return Format::is_valid_utf(filename_); }

        form path_form() const noexcept;
        Path relative_to(const Path& base) const;
        std::pair<std::string, std::string> split_leaf() const;
        std::pair<string_type, string_type> split_os_leaf() const;
        std::pair<Path, Path> split_path() const;
        std::pair<Path, Path> split_root() const;

        static Path common(const Path& lhs, const Path& rhs);
        template <typename Range> static Path common(const Range& files);
        static Path join(const Path& lhs, const Path& rhs);
        template <typename Range> static Path join(const Range& files);
        friend Path operator/(const Path& lhs, const Path& rhs) { return Path::join(lhs, rhs); }
        friend Path& operator/=(Path& lhs, const Path& rhs) { lhs = Path::join(lhs, rhs); return lhs; }

        friend std::ostream& operator<<(std::ostream& out, const Path& p) { return out << p.name(); }
        friend std::ostream& operator<<(std::ostream& out, Path::form f);
        friend bool operator==(const Path& lhs, const Path& rhs) noexcept { return lhs.filename_ == rhs.filename_; }
        friend bool operator<(const Path& lhs, const Path& rhs) noexcept { return lhs.filename_ < rhs.filename_; }

        // File system query functions

        time_point access_time(int flags = 0) const noexcept;
        time_point create_time(int flags = 0) const noexcept;
        time_point modify_time(int flags = 0) const noexcept;
        time_point status_time(int flags = 0) const noexcept;

        directory_range directory(int flags = 0) const;
        search_range deep_search(int flags = 0) const;
        bool exists(int flags = 0) const noexcept;
        id_type id(int flags = 0) const noexcept;

        bool is_directory(int flags = 0) const noexcept;
        bool is_file(int flags = 0) const noexcept;
        bool is_special(int flags = 0) const noexcept;
        bool is_hidden() const noexcept;
        bool is_symlink() const noexcept;

        Path resolve() const;
        Path resolve_symlink() const;
        uint64_t size(int flags = 0) const;

        // File system update functions

        void copy_to(const Path& dst, int flags = 0) const;
        void create() const;
        void make_directory(int flags = 0) const;
        void make_symlink(const Path& linkname, int flags = 0) const;
        void move_to(const Path& dst, int flags = 0) const;
        void remove(int flags = 0) const;

        void set_access_time(int flags = 0) const { set_access_time(std::chrono::system_clock::now(), flags); }
        void set_access_time(time_point t, int flags = 0) const;
        void set_create_time(int flags = 0) const { set_create_time(std::chrono::system_clock::now(), flags); }
        void set_create_time(time_point t, int flags = 0) const;
        void set_modify_time(int flags = 0) const { set_modify_time(std::chrono::system_clock::now(), flags); }
        void set_modify_time(time_point t, int flags = 0) const;

        // I/O functions

        void load(std::string& str, size_t maxlen = npos, int flags = 0) const;
        void save(const std::string& str, int flags = 0) const;

        // Process state functions

        void change_directory() const;
        static Path current_directory();

    private:

        string_type filename_;

        std::pair<string_type, string_type> get_base_ext() const noexcept;
        string_type get_leaf() const noexcept;
        string_type get_root(bool allow_drive_special) const noexcept;
        void make_canonical(int flags);
        template <typename Range, typename BinaryFunction> static Path do_combine(const Range& range, BinaryFunction f);

        #ifdef _XOPEN_SOURCE
            void set_file_times(time_point atime, time_point mtime, int flags) const;
        #else
            time_point get_file_time(int index) const noexcept;
            void set_file_time(time_point t, int index) const;
        #endif

    };

        template <typename Range>
        Path Path::common(const Range& files) {
            return do_combine(files, [] (const Path& lhs, const Path& rhs) { return common(lhs, rhs); });
        }

        template <typename Range>
        Path Path::join(const Range& files) {
            return do_combine(files, [] (const Path& lhs, const Path& rhs) { return join(lhs, rhs); });
        }

        template <typename Range, typename BinaryFunction>
        Path Path::do_combine(const Range& range, BinaryFunction f) {
            using std::begin;
            using std::end;
            auto i = begin(range), j = end(range);
            if (i == j)
                return {};
            Path p = *i;
            for (++i; i != j; ++i)
                p = f(p, *i);
            return p;
        }

        class Path::search_iterator:
        public TL::InputIterator<search_iterator, const Path> {
        public:
            search_iterator() = default;
            search_iterator(const Path& dir, int flags);
            const Path& operator*() const noexcept;
            search_iterator& operator++();
            bool operator==(const search_iterator& i) const noexcept { return impl == i.impl; }
        private:
            struct impl_type;
            std::shared_ptr<impl_type> impl;
        };

        class Path::directory_iterator:
        public TL::InputIterator<directory_iterator, const Path> {
        public:
            directory_iterator() = default;
            directory_iterator(const Path& dir, int flags);
            const Path& operator*() const noexcept;
            directory_iterator& operator++();
            bool operator==(const directory_iterator& i) const noexcept { return impl == i.impl; }
        private:
            struct impl_type;
            std::shared_ptr<impl_type> impl;
        };

}

namespace std {

    template <>
    class hash<RS::IO::Path> {
        size_t operator()(const RS::IO::Path& path) const {
            return path.hash();
        }
    };

}
