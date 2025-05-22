//
// Created by Akashic on 5/16/2025.
//

#include "ogl.hpp"
#include "glad/glad.h"

#include "rml_ui_backend/RmlUi_Backend.h"
#include "rml_ui_backend/RmlUi_Renderer_GL3.h"

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

namespace soil {
    void OGL::draw_texture(const Mesh& mesh, const OglTextureHandle handle) const {
        static_cast<RenderInterface_GL3*>(Backend::GetRenderInterface())
            ->UseProgram(ProgramId::Passthrough);

        const auto texture_optional = this->lookup.find(handle);
        assert(texture_optional.has_value());
        auto& texture = texture_optional.value().get();

        GLCall(glBindTexture(GL_TEXTURE_2D, texture.open_gl_handle));
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.element_buffer_object));
        GLCall(glBindVertexArray(mesh.vertex_array_object));
        GLCall(glDrawElements(GL_TRIANGLES, mesh.vertex_render_count, GL_UNSIGNED_INT, nullptr)
        );
    }

    OglTexture OGL::load_texture(const ApiTexture& texture) {
        const auto  width  = texture.width;
        const auto  height = texture.height;
        const auto* data   = texture.data;

        uint32_t texture_id{0};
        GLCall(glGenTextures(1, &texture_id));
        GLCall(glBindTexture(GL_TEXTURE_2D, texture_id));

        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));

        constexpr float border_color[] = {
            1.0f, 1.0f, 1.0f, 1.0f
        }; // Bright white to signify its fucked
        GLCall(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color));

        GLCall(glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
        ));

        return OglTexture{width, height, TextureFormat::RGBA8, texture_id};
    }

    Mesh OGL::build_mesh_rect(const glm::vec2 top_left, const glm::vec2 size) {
        const auto tl = to_ndc(top_left);
        const auto tr = to_ndc(top_left + glm::vec2{size.x, 0});
        const auto bl = to_ndc(top_left + glm::vec2{0, size.y});
        const auto br = to_ndc(top_left + glm::vec2{size.x, size.y});

        const auto vertices = std::array<float, 16>{
            tr.x, tr.y, 1.f, 1.f, // Top right
            br.x, br.y, 1.f, 0.f, // Bottom Right
            bl.x, bl.y, 0.f, 0.f, // Bottom Left
            tl.x, tl.y, 0.f, 1.f, // Top Left
        };

        constexpr auto indices = std::array{0, 1, 3, 1, 2, 3};

        Mesh mesh = {};

        GLCall(glGenVertexArrays(1, &mesh.vertex_array_object));
        GLCall(glGenBuffers(1, &mesh.vertex_buffer_object));
        GLCall(glGenBuffers(1, &mesh.element_buffer_object));

        GLCall(glBindVertexArray(mesh.vertex_array_object));

        GLCall(glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_buffer_object));
        GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW)
        );

        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.element_buffer_object));
        GLCall(glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW
        ));

        const auto program = static_cast<RenderInterface_GL3*>(Backend::GetRenderInterface())
                                 ->GetProgram(ProgramId::Passthrough);

        int32_t pos_location       = glGetAttribLocation(program, "inPosition");
        int32_t tex_coord_location = glGetAttribLocation(
            program, "inTexCoord0"
        ); // FOR SOME FUCKING REASON THIS IS 2 NOT 1?????????????

        GLCall(glEnableVertexAttribArray(pos_location));

        GLCall(glEnableVertexAttribArray(tex_coord_location));

        GLCall(glVertexAttribPointer(
            pos_location, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0)
        )); // position
        GLCall(glVertexAttribPointer(
            tex_coord_location, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
            reinterpret_cast<void*>((2 * sizeof(float)))
        )); // texcoord

        mesh.vertex_render_count = indices.size();

        return mesh;
    }

    void
    OGL::build_mesh_rect(const Mesh& mesh, const glm::vec2 top_left, const glm::vec2 size) {
        const auto tl = to_ndc(top_left);
        const auto tr = to_ndc(top_left + glm::vec2{size.x, 0});
        const auto bl = to_ndc(top_left + glm::vec2{0, size.y});
        const auto br = to_ndc(top_left + glm::vec2{size.x, size.y});

        const auto vertices = std::array<glm::vec2, 8>{
            tr, {1.f, 1.f}, // Top right
            br, {1.f, 0.f}, // Bottom Right
            bl, {0.f, 0.f}, // Bottom Left
            tl, {0.f, 1.f}, // Top Left
        };

        GLCall(glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_buffer_object));
        GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW)
        );
    }

    glm::vec2 OGL::to_ndc(const glm::vec2 position) {
        int width, height;
        glfwGetWindowSize(Backend::get_window(), &width, &height);

        const auto x = (position.x / width) * 2.0f - 1.0f;
        const auto y = 1.0 - (position.y / height) * 2.0f;

        return glm::vec2{x, static_cast<float>(y)};
    }
} // namespace soil