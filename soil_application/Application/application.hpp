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
#include <memory>

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
        using HeaderType = HeaderBar<EditorVisualConfig, BasicButton, BasicButton>;

        EditorVisualConfig                  visual_config{};
        std::unique_ptr<ApplicationSidebar> sidebar{nullptr}; // Allows delayed initialization
        std::unique_ptr<Project>            project{nullptr};
        HeaderType                          header;

        friend class BasicButton;
    };

} // namespace soil

#endif // APPLICATION_HPP
