#include "./side_bar.hpp"
#include <RmlUi/Core/Element.h>

namespace soil {

    void SideBar::render(Rml::Element& element) {
        this->fs_parent = &element;
        this->render();
    }

    void SideBar::render() {
        if (!this->fs_parent) {
            return; // Must mean the element we wanted to use is gone. This will happen if the
                    // bar is hidden for example, or state is reloaded
        }

        auto& root = *this->fs_parent;

        root.SetInnerRML(""); // Slightly hacky method to delete all children

        this->current_fs->render_tree(root);
    }

} // namespace soil