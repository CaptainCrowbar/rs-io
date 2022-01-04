#pragma once

#include <functional>
#include <iterator>
#include <string>
#include <type_traits>
#include <utility>

namespace RS::IO {

    constexpr size_t npos = std::string::npos;

    template <typename Iterator>
    struct Irange {
        Iterator first, second;
        Iterator begin() const { return first; }
        Iterator end() const { return second; }
        bool empty() const noexcept { return first == second; }
    };

    template <typename Iterator>
    Irange<Iterator> irange(Iterator i, Iterator j) {
        return {i, j};
    }

    template <typename T, typename Del, T Null = T()>
    class Resource{
    public:
        static constexpr T null = Null;
        constexpr Resource() noexcept: value_(Null) {}
        constexpr explicit Resource(T t) noexcept: value_(t) {}
        ~Resource() noexcept { reset(); }
        Resource(const Resource&) = delete;
        constexpr Resource(Resource&& r) noexcept: value_(std::exchange(r.value_, Null)) {}
        Resource& operator=(const Resource&) = delete;
        Resource& operator=(Resource&& r) noexcept {
            if (&r != this) {
                reset();
                value_ = std::exchange(r.value_, Null);
            }
            return *this;
        }
        constexpr operator T() const noexcept { return value_; }
        constexpr explicit operator bool() const noexcept { return value_ != Null; }
        constexpr auto& operator*() const noexcept { return *value_; }
        constexpr auto* operator->() const noexcept { return &**this; }
        constexpr T get() const noexcept { return value_; }
        constexpr T release() noexcept { return std::exchange(value_, Null); }
        void reset() noexcept {
            if (value_ != Null) {
                Del()(value_);
                value_ = Null;
            }
        }
        friend constexpr void swap(Resource& a, Resource& b) noexcept {
            T t = a.value_;
            a.value_ = b.value_;
            b.value_ = t;
        }
    private:
        T value_;
    };

    class ScopeGuard {
    public:
        template <typename F> explicit ScopeGuard(F callback)
            { try { call_ = callback; } catch (...) { callback(); throw; } }
        ~ScopeGuard() noexcept { if (call_) { try { call_(); } catch (...) {} } }
        ScopeGuard(const ScopeGuard&) = delete;
        ScopeGuard(ScopeGuard&&) = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;
        ScopeGuard& operator=(ScopeGuard&&) = delete;
        void release() noexcept { call_ = nullptr; }
    private:
        std::function<void()> call_;
    };

    template <typename T>
    struct TotalOrder {
        friend bool operator!=(const T& a, const T& b) noexcept { return ! (a == b); }
        friend bool operator>(const T& a, const T& b) noexcept { return b < a; }
        friend bool operator<=(const T& a, const T& b) noexcept { return ! (b < a); }
        friend bool operator>=(const T& a, const T& b) noexcept { return ! (a < b); }
    };

    // Mixin                  Requires         Defines
    // =====                  ========         =======
    // InputIterator          *t ++t t==u      t-> t++ t!=u
    // OutputIterator         t=v              *t ++t t++
    // ForwardIterator        *t ++t t==u      t-> t++ t!=u
    // BidirectionalIterator  *t ++t --t t==u  t-> t++ t-- t!=u
    // RandomAccessIterator   *t t+=n t-u      t-> t[n] ++t t++ --t t-- t-=n t+n n+t t-n t==u t!=u t<u t>u t<=u t>=u

    template <typename T, typename CV>
    struct InputIterator {
        using difference_type = ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        using pointer = CV*;
        using reference = CV&;
        using value_type = std::remove_const_t<CV>;
        CV* operator->() const noexcept { return &*static_cast<const T&>(*this); }
        friend T operator++(T& t, int) { T rc = t; ++t; return rc; }
        friend bool operator!=(const T& a, const T& b) noexcept { return ! (a == b); }
    };

    template <typename T>
    struct OutputIterator {
        using difference_type = void;
        using iterator_category = std::output_iterator_tag;
        using pointer = void;
        using reference = void;
        using value_type = void;
        T& operator*() noexcept { return static_cast<T&>(*this); }
        friend T& operator++(T& t) noexcept { return t; }
        friend T operator++(T& t, int) noexcept { return t; }
    };

    template <typename T, typename CV>
    struct ForwardIterator:
    InputIterator<T, CV> {
        using iterator_category = std::forward_iterator_tag;
    };

    template <typename T, typename CV>
    struct BidirectionalIterator:
    ForwardIterator<T, CV> {
        using iterator_category = std::bidirectional_iterator_tag;
        friend T operator--(T& t, int) { T rc = t; --t; return rc; }
    };

    template <typename T, typename CV>
    struct RandomAccessIterator:
    BidirectionalIterator<T, CV>,
    TotalOrder<T> {
        using iterator_category = std::random_access_iterator_tag;
        CV& operator[](ptrdiff_t i) const noexcept { T t = static_cast<const T&>(*this); t += i; return *t; }
        friend T& operator++(T& t) { return t += 1; }
        friend T& operator--(T& t) { return t += -1; }
        friend T& operator-=(T& a, ptrdiff_t b) { return a += - b; }
        friend T operator+(const T& a, ptrdiff_t b) { T t = a; return t += b; }
        friend T operator+(ptrdiff_t a, const T& b) { T t = b; return t += a; }
        friend T operator-(const T& a, ptrdiff_t b) { T t = a; return t -= b; }
        friend bool operator==(const T& a, const T& b) noexcept { return a - b == 0; }
        friend bool operator<(const T& a, const T& b) noexcept { return a - b < 0; }
    };

}
