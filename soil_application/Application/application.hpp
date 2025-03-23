//
// Created by Akashi on 02/03/25.
//

#ifndef APPLICATION_HPP
#define APPLICATION_HPP
#include "../clay/components.hpp"
#include "./components/simple_button.hpp"
#include "Application/application_sidebar.hpp"
#include "Application/project/project.hpp"
#include "editor_config.hpp"
#include <DirectXMagic/dx_creation.hpp>
#include <memory>
#include <open_gl_magic/ogl.hpp>

namespace soil {
    using namespace clay_extension;
    class Application {
    public:
        Application() noexcept;

        void render() noexcept;

    private:
        void render_head_bar() noexcept;

        void handle_project_change(const char *new_path) noexcept;

    private:
        using HeaderType = HeaderBar<
            EditorVisualConfig, BasicButton, BasicButton, BasicButton, BasicButton,
            BasicButton>;

        EditorVisualConfig                  visual_config{};
        std::unique_ptr<ApplicationSidebar> sidebar{nullptr}; // Allows delayed initialization
        std::unique_ptr<Project>            project{nullptr};
        std::unique_ptr<D3D::D3D12>         d3d12_ctx{nullptr};
        std::unique_ptr<ogl::OpenGl>        ogl_ctx{nullptr};
        HeaderType                          header;

        friend class BasicButton;
    };

} // namespace soil

#endif // APPLICATION_HPP
