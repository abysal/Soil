#pragma once
#include <experimental/memory>

namespace std {
    template <typename T>
    using observer_ptr = std::experimental::observer_ptr<T>;

    template <typename PointerType> observer_ptr<PointerType> make_observer(PointerType *pointer) noexcept {
        return observer_ptr<PointerType>(pointer);
    }
} // namespace std