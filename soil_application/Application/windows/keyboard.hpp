#pragma once
#include <types.hpp>
#include <unordered_map>

namespace soil {

    enum class Key {

    };

    class KeyboardManager {
    public:
        KeyboardManager() noexcept = default;

        void update() noexcept;

    private:
        std::unordered_map<Key, i32> keycode_map{};
    };
} // namespace soil