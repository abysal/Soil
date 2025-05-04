#pragma once
#include <assert.h>
#include <filesystem>
#include <types.hpp>
#include <vector>

namespace soil {

    using Win32Handle = void*;

    class Injector {
    public:
        Injector(std::filesystem::path&& dll_path) noexcept : dll_path(dll_path) {};

        ~Injector();

        Injector(const Injector&) = delete;
        Injector(Injector&&)      = default;

        void attach();

        Win32Handle constexpr inline valid_handle() noexcept {
            assert(this->mc_handle);
            return this->mc_handle;
        }

    private:
        void update_security();

        u32 get_mc();

        usize get_process_module(u32 PID, const std::string module_name);

        std::vector<u32> iter_proc_list(const std::string_view process_name);

    private:
        std::filesystem::path dll_path;
        Win32Handle           mc_handle{nullptr};
    };
} // namespace soil