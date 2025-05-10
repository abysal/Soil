#pragma once
#include "./rml_ui_backend/RmlUi_Backend.h"
#include "document/document_manager.hpp"
#include "rml_ui_backend/RmlUi_Backend.h"
#include "settings.hpp"
#include "shim/fs_shim.hpp"
#include <RmlUi/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Types.h>

namespace soil {
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
        // using KeyDownCallback = bool (*)(Rml::Context* context, Rml::Input::KeyIdentifier
        // key, int key_modifier, float native_dp_ratio, bool priority);

        static bool process_key(
            Rml::Context* context, Rml::Input::KeyIdentifier key, int key_modifier,
            float native_dp_ratio, bool priority
        );

    private:
        SimpleShim                        shim        = {};
        Rml::Context*                     context     = nullptr;
        SoilSettings                      settings    = {};
        std::vector<Rml::DataModelHandle> handle_list = {};
        bool                              running     = true;
        DocumentManager                   manager     = {};

        friend struct RmlBinder;
        friend struct SoilSettings;
    };
} // namespace soil