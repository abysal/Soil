#pragma once
#include <array>
#include <atomic>

namespace dx_ipc {

    constexpr int INIT_MAGIC = 0xDEADBEEF;

    struct Dx12TextureDesc {
        uint64_t render_handle{0xDEADC0DE};
        uint64_t total_size{};
        uint32_t width{};
        uint32_t height{};
        uint32_t format{};
    };

    struct Dx12Ipc {
        std::atomic_bool                  hooks_init{false};
        std::atomic_bool                  consumed_frame{true};
        std::atomic_bool                  supports_v3_swapchain{true};
        std::atomic_bool                  clear_handled{true};
        std::atomic_long                  last_error{0};
        std::atomic_uint                  init_magic{0xDEADC0DE};
        std::atomic<Dx12TextureDesc>      render_texture_info{};
        std::atomic_uint64_t              back_buffer_pointer{0};
        std::atomic_uint64_t              soil_handle{};
        std::array<std::atomic_char, 512> string_buffer{};
    };
} // namespace dx_ipc