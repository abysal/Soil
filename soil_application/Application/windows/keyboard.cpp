#include "./keyboard.hpp"

#define NOMINMAX
#include <windows.h>

namespace soil {
    void KeyboardManager::update() noexcept {
        const auto layout = GetKeyboardLayout(0);

        constexpr auto VK_RANGE_LOW  = 0x01;
        constexpr auto VK_RANGE_HIGH = 0xFF;

        for (auto vk = VK_RANGE_LOW; vk < VK_RANGE_HIGH; vk++) {}
    }
} // namespace soil