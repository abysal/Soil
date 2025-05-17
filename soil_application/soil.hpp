#pragma once
#include "./rml_ui_backend/RmlUi_Backend.h"
#include "document/document_manager.hpp"
#include "fs_provider.hpp"
#include "graphics_api/dx12.hpp"
#include "graphics_api/ogl.hpp"
#include "rml_extra/ogl_instancer.hpp"
#include "settings.hpp"
#include "shim/fs_shim.hpp"
#include "side_bar/side_bar.hpp"
#include <RmlUi/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Types.h>

namespace Rml {
    class Event;
}

namespace soil {
    class OglInstancer;
    class OGL;
    void run();

    class Application {
    public:
        Application() { this->init(); }
        Application(const Application&)            = delete;
        Application(Application&&)                 = delete;
        Application& operator=(const Application&) = delete;
        Application& operator=(Application&&)      = delete;
        ~Application() {
            Rml::Shutdown();
            Backend::Shutdown();
        }

    private:
        void init();

        void bind_core();

        void main_loop();

        void process();

        // using KeyDownCallback = bool (*)(Rml::Context* context, Rml::Input::KeyIdentifier
        // key, int key_modifier, float native_dp_ratio, bool priority);

        static bool process_key(
            Rml::Context* context, Rml::Input::KeyIdentifier key, int key_modifier,
            float native_dp_ratio, bool priority
        );

        void process_project(
            Rml::DataModelHandle handle, class Rml::Event& event, const Rml::VariantList& args
        );

        void update_side_bar();

    private:
        SimpleShim                        shim        = {};
        Rml::Context*                     context     = nullptr;
        SoilSettings                      settings    = {};
        std::vector<Rml::DataModelHandle> handle_list = {};
        bool                              running     = true;
        DocumentManager                   manager     = {};
        FsProviderPtr                     fs          = {};
        SideBar                           side_bar    = {};
        std::unique_ptr<D3D12>            dx_12       = {};
        OGL                               open_gl     = {};
        std::unique_ptr<OglInstancer>     instancer   = {};

        friend struct RmlBinder;
        friend struct SoilSettings;
    };
} // namespace soil