#include "soil.hpp"
#include "document/document_loader.hpp"
#include "document/document_manager.hpp"
#include "rml_extra/rml_functions.hpp"
#include "rml_ui_backend/RmlUi_Backend.h"
#include <GLFW/glfw3.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/StyleTypes.h>
#include <RmlUi/Debugger.h>
#include <RmlUi/Debugger/Debugger.h>
#include <cassert>
#include <cstdio>
#include <print>

#include <RmlUi/Core.h>
#include <RmlUi/Core/RenderInterface.h>

namespace soil {

    void run() { Application app{}; }

    void Application::init() {
        std::println("Starting Soil!");

        constexpr auto width  = 1280;
        constexpr auto height = 720;

        Backend::Initialize("Soil Editor", width, height, true);

        Rml::SetFileInterface(&this->shim);
        Rml::SetRenderInterface(Backend::GetRenderInterface());
        Rml::SetSystemInterface(Backend::GetSystemInterface());

        if (!Rml::Initialise()) {
            return; // Dies;
        }

        this->context = Rml::CreateContext("main", {width, height});

        const auto handle = this->shim.Open("./resources/fonts/JetBrainsMonoNL-Regular.ttf");
        assert(handle != 0);

        this->shim.Seek(handle, 0, SEEK_END);

        const auto size = this->shim.Tell(handle);

        this->shim.Seek(handle, 0, SEEK_SET);

        std::vector<uint8_t> memory;
        memory.resize(size);

        const auto read_count = this->shim.Read(memory.data(), size, handle);
        assert(read_count == size);
        (void)read_count;

        Rml::LoadFontFace(
            memory, "JetBrainsMonoNL-Regular", Rml::Style::FontStyle::Normal,
            Rml::Style::FontWeight::Normal, true
        );

        this->bind_core();

        DocumentLoader{}.load_inital_documents(*this->context, this->shim);

        this->manager = DocumentManager{this->context};

        this->settings.bind_callbacks(*this->context, this->manager);

        Rml::Debugger::Initialise(this->context);

        Rml::Debugger::SetVisible(true);

        this->main_loop();
    }

    void Application::bind_core() {

        auto model_builder = this->context->CreateDataModel("core_ui");
        RmlBinder{}.bind_simple(model_builder, *this);

        this->settings.bind_settings(model_builder);

        this->handle_list.push_back(model_builder.GetModelHandle());
    }

    static bool reload_ui = false;

    void Application::main_loop() {
        while (this->running) {
            this->running = Backend::ProcessEvents(this->context, &Application::process_key);

            // Call into our code

            // Tell the context it happened
            this->context->Update();

            Backend::BeginFrame();
            this->context->Render();
            Backend::PresentFrame();

            if (reload_ui) {
                DocumentLoader{}.load_inital_documents(*this->context, this->shim);
                reload_ui = false;
            }
        }
    }

    bool Application::process_key(
        Rml::Context* context, Rml::Input::KeyIdentifier key, int key_modifier,
        float native_dp_ratio, bool priority
    ) {

        // This handles hot reloading of our ui!
        if (key == Rml::Input::KI_R && key_modifier & Rml::Input::KM_CTRL) {
            Rml::Log::Message(Rml::Log::LT_INFO, "Reloading Documents!");

            for (int i = 0; i < context->GetNumDocuments(); i++) {
                Rml::ElementDocument* document = context->GetDocument(i);

                const Rml::String& src = document->GetSourceURL();

                if (src.size() > 4 && src.substr(src.size() - 4) == ".rml") {
                    document->Close();
                    document->ReloadStyleSheet();
                    context->UnloadDocument(document);
                }
            }

            reload_ui = true;

            return false;
        }

        if (key == Rml::Input::KI_F8) {
            Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());

            return false;
        }

        return true;
    }

} // namespace soil