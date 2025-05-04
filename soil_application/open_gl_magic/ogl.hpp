#pragma once
#include <cstddef>
#define NOMINMAX
#include "Application/memory/pointer.hpp"
#include "DirectXMagic/dx_creation.hpp"
#include <clay/raywrapped.hpp>
#include <ipc.hpp>
#include <minwindef.h>
#include <types.hpp>
#include <unordered_map>
#include <vector>

namespace D3D {
    class D3D12;
}

#define GL_HANDLE_TYPE_D3D12_TEXTURE_EXT 0x9593

namespace ogl {
    // Provides a way to talk to the raw open gl context that raylib manages, so we can upload
    // textures and call some low level functions
    class OpenGl {
    public:
    public:
        OpenGl();
        OpenGl(const OpenGl&) = delete;
        OpenGl(OpenGl&&)      = default;

        void load_test_texture();

        u32 new_memory_object();

        inline Texture2D& texture_from_name(const std::string& id) {
            return this->textures[this->texture_index_lookup.at(id)];
        }

        inline Texture2D& texture_from_idx(usize index) { return this->textures[index]; }

        inline usize texture_index_from_name(const std::string& id) {
            return this->texture_index_lookup.at(id);
        }

        inline usize last_index() const noexcept { return this->textures.size() - 1; }

        void display_debug_texture() noexcept;

        void dx_to_ogl_test(std::observer_ptr<D3D::D3D12> dx) noexcept;

        void unload_texture_index(usize index);

        void dx_to_opengl_ipc(volatile dx_ipc::Dx12Ipc& texture);

    private:
    public:
        std::vector<Texture2D>                 textures{};
        std::unordered_map<std::string, usize> texture_index_lookup{};
        std::vector<u32>                       memory_objects{};
    };
} // namespace ogl