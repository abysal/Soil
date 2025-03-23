#pragma once
#include <compare> // MSVC requires this

namespace std_ext {

    template <typename T> class ObserverPtr {
    public:
        constexpr ObserverPtr() noexcept = default;
        constexpr ObserverPtr(T *p) noexcept : ptr(p) {}

        constexpr T       *get() noexcept { return ptr; }
        constexpr const T *get() const noexcept { return ptr; }

        constexpr T       *operator->() noexcept { return ptr; }
        constexpr const T *operator->() const noexcept { return ptr; }

        constexpr T       &operator*() noexcept { return *ptr; }
        constexpr const T &operator*() const noexcept { return *ptr; }

        constexpr explicit operator bool() const noexcept { return ptr != nullptr; }

        constexpr auto operator<=>(const ObserverPtr<T> &) const noexcept = default;

    private:
        T *ptr{nullptr};
    };

} // namespace std_ext

namespace std {
    template <typename T> using observer_ptr = std_ext::ObserverPtr<T>;

    template <typename PointerType>
    constexpr observer_ptr<PointerType> make_observer(PointerType *pointer) noexcept {
        return observer_ptr<PointerType>(pointer);
    }
} // namespace std