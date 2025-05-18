#pragma once
#include "glad/glad.h"
#include "texture.hpp"

#include <glm/vec2.hpp>

namespace soil {

    struct OglTexture : BaseTexture {
        uint32_t open_gl_handle = {};

        OglTexture(
            uint32_t width = 0, uint32_t height = 0,
            TextureFormat format = TextureFormat::RGBA8, uint32_t open_gl_handle = {}
        )
            : BaseTexture(width, height, format), open_gl_handle(open_gl_handle) {}
    };

    class OglTextureHandle : public TextureHandle {
    public:
        using TextureHandle::TextureHandle;
    };

    using OglTextureLookup = TextureLookup<OglTextureHandle, OglTexture>;

    struct Mesh {
        uint32_t vertex_array_object   = {std::numeric_limits<uint32_t>::max()};
        uint32_t vertex_buffer_object  = {std::numeric_limits<uint32_t>::max()};
        uint32_t element_buffer_object = {std::numeric_limits<uint32_t>::max()};
        int32_t  vertex_render_count   = {-1};

        void unload() const {
            glDeleteVertexArrays(1, &this->vertex_array_object);
            glDeleteBuffers(1, &this->vertex_buffer_object);
            glDeleteBuffers(1, &this->element_buffer_object);
        }
    };

} // namespace soil

template <> struct std::hash<soil::OglTextureHandle> {
    size_t operator()(const soil::OglTextureHandle& handle) const noexcept {
        return static_cast<size_t>(handle.get_handle());
    }
}; // namespace std

namespace soil {
    class OGL {
    public:
        void draw_texture(const Mesh& mesh, const OglTextureHandle handle) const;

        static OglTexture load_texture(const ApiTexture& texture);

        static Mesh build_mesh_rect(const glm::vec2 top_left, const glm::vec2 size);
        static void
        build_mesh_rect(const Mesh& mesh, const glm::vec2 top_left, const glm::vec2 size);

        static glm::vec2 to_ndc(const glm::vec2 position);

    public:
        OglTextureLookup lookup{};
    };
} // namespace soil