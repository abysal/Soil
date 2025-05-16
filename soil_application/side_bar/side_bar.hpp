#pragma once
#include <RmlUi/Core/Element.h>
#include <fs_provider.hpp>

namespace soil {
    class SideBar {
    public:
        SideBar()                          = default;
        SideBar(const SideBar&)            = default;
        SideBar(SideBar&&)                 = default;
        SideBar& operator=(const SideBar&) = default;
        SideBar& operator=(SideBar&&)      = default;

        void update_fs(FsProviderPtr ptr) { this->current_fs = ptr; }

        void render(class Rml::Element&);
        void render();

    private:
        FsProviderPtr current_fs = nullptr;
        Rml::Element* fs_parent  = {};
    };
} // namespace soil