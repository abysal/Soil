#pragma once
#include "./rml_ui_backend/RmlUi_Backend.h"
#include "rml_ui_backend/RmlUi_Backend.h"
#include "shim/fs_shim.hpp"
#include <RmlUi/Core.h>
#include <RmlUi/Core/Context.h>
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

        void main_loop();

    private:
        SimpleShim            shim            = {};
        Rml::Context*         context         = nullptr;
        Rml::ElementDocument* active_document = nullptr;
        bool                  running         = true;
    };
} // namespace soil