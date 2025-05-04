#pragma once
#include "./injector.hpp"
#include <stdexcept>
#include <type_traits>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <handleapi.h>
#include <memoryapi.h>
#include <processthreadsapi.h>
#include <winbase.h>
#include <winnt.h>

namespace soil {
    template <typename T>
        requires std::is_default_constructible_v<T> && std::is_trivially_destructible_v<T>
    class IPCManager {
    public:
        IPCManager() noexcept = default;

        ~IPCManager() noexcept {
            this->structure->~T();
            UnmapViewOfFile((const void*)this->structure);
            CloseHandle(this->local_file_handle);
        }

        bool created() const noexcept { return this->structure; }

        volatile T& get_struct() noexcept {
            assert(this->structure);
            return *this->structure;
        }

        Win32Handle instance_ipc(Injector& injector) {
            const Win32Handle file_handle =
                CreateFileMapping(nullptr, nullptr, PAGE_READWRITE, 0, sizeof(T) * 2, nullptr);

            if (!file_handle) {
                throw std::runtime_error("Failed to create map file");
            }

            const Win32Handle mc_handle = injector.valid_handle();

            const Win32Handle own_handle = GetCurrentProcess();

            Win32Handle mc_file_handle{nullptr};

            if (!DuplicateHandle(
                    own_handle, file_handle, mc_handle, &mc_file_handle,
                    FILE_MAP_READ | FILE_MAP_WRITE, true, 0
                )) {
                throw std::runtime_error("Failed to open a handle into the game");
            }

            this->local_file_handle = file_handle;

            LPVOID mapped_memory =
                MapViewOfFile(file_handle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(T) * 2);

            if (!mapped_memory) {
                throw std::runtime_error("Failed to map memory");
            }

            this->structure = new (mapped_memory) T();

            return mc_file_handle;
        }

    private:
        volatile T* structure{nullptr};
        Win32Handle local_file_handle{};
    };
} // namespace soil