
#define SOIL_GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>

#include "./ogl.hpp"
#include "GLFW/glfw3.h"
#include "raylib.h"

// pulls in the version of glad raylib uses
#include <clay/clay_binding.hpp>
#include <print>
#include <rlgl.h>

#include <DirectXMagic/dx_creation.hpp>
#include <stdexcept>

using namespace clay_extension;
void CheckOpenGLError(const char* stmt, const char* fname, int line) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
        abort();
    }
}

#define GL_CHECK(stmt)                                                                         \
    do {                                                                                       \
        stmt;                                                                                  \
        CheckOpenGLError(#stmt, __FILE__, __LINE__);                                           \
    } while (0)

namespace ogl {
    OpenGl::OpenGl() {
        if (!soil_gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
            throw std::logic_error("Failed to load gl");
        }
    }

    void OpenGl::load_test_texture() {
        Image debug_texture =
            GenImageGradientLinear(500, 500, 90, {0, 121, 241, 255}, {255, 161, 0, 255});

        u32 id;

        GL_CHECK(glGenTextures(1, &id));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));

        GL_CHECK(glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, 500, 500, 0, GL_RGBA, GL_UNSIGNED_BYTE,
            debug_texture.data
        ));
        GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));

        UnloadImage(debug_texture);

        this->textures.emplace_back(Texture2D{
            .id      = id,
            .width   = 500,
            .height  = 500,
            .mipmaps = 1,
            .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        });

        this->texture_index_lookup["debug_texture"] = this->textures.size() - 1;
    }

    void OpenGl::display_debug_texture() noexcept {
        if (!this->texture_index_lookup.contains("debug_texture")) {
            return;
        }

        bool  dx      = this->texture_index_lookup.contains("direct_x");
        auto& texture = (dx) ? this->texture_from_name("direct_x")
                             : this->texture_from_name("debug_texture");
        new_element(
            Clay_ElementDeclaration{
                .layout.sizing = {CLAY_SIZING_FIXED(500), CLAY_SIZING_FIXED(500)},
                .image =
                    {.imageData        = &texture,
                     .sourceDimensions = {(f32)texture.width, (f32)texture.height}}
            },
            [&] {}
        );
    }

    void OpenGl::unload_texture_index(usize index) {
        GL_CHECK(glDeleteTextures(1, &this->texture_from_idx(index).id));
    }

    void OpenGl::dx_to_opengl_ipc(volatile dx_ipc::Dx12Ipc& texture) {
        const u32 mem_obj = this->new_memory_object();

        const auto info = texture.render_texture_info.load();

        GL_CHECK(glImportMemoryWin32HandleEXT(
            mem_obj, info.total_size * 2, GL_HANDLE_TYPE_D3D12_RESOURCE_EXT,
            (void*)info.render_handle
        ));

        u32 texture_id;

        assert(info.format == 28);

        GL_CHECK(glGenTextures(1, &texture_id));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture_id));
        GL_CHECK(glTexStorageMem2DEXT(
            GL_TEXTURE_2D, 1, GL_RGBA8, info.width, info.height, mem_obj, 0
        ));

        this->textures.emplace_back(Texture2D{
            .id      = texture_id,
            .width   = (i32)info.width,
            .height  = (i32)info.height,
            .mipmaps = 1,
            .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        });

        this->texture_index_lookup["direct_x"] = this->textures.size() - 1;
    }

    u32 OpenGl::new_memory_object() {
        u32 mem;
        GL_CHECK(glCreateMemoryObjectsEXT(1, &mem));
        this->memory_objects.push_back(mem);
        return mem;
    }

    void OpenGl::dx_to_ogl_test(std::observer_ptr<D3D::D3D12> dx) noexcept {
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
        UINT64                             total_size;
        D3D12_RESOURCE_DESC                resource_disc;
        dx->device->GetCopyableFootprints(
            &resource_disc, 0, 1, 0, &layout, nullptr, nullptr, &total_size
        );

        const u32 mem_obj = this->new_memory_object();

        GL_CHECK(glImportMemoryWin32HandleEXT(
            mem_obj, total_size * 2, GL_HANDLE_TYPE_D3D12_RESOURCE_EXT,
            dx->present_texture_handle
        ));

        u32 texture_id;

        GL_CHECK(glGenTextures(1, &texture_id));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture_id));
        GL_CHECK(glTexStorageMem2DEXT(
            GL_TEXTURE_2D, 1, GL_RGBA8, dx->present_width, dx->present_height, mem_obj, 0
        ));

        this->textures.emplace_back(Texture2D{
            .id      = texture_id,
            .width   = (i32)dx->present_width,
            .height  = (i32)dx->present_height,
            .mipmaps = 1,
            .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        });

        this->texture_index_lookup["direct_x"] = this->textures.size() - 1;
    }
} // namespace ogl