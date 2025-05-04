#include "soil.hpp"
#include "rml_ui_backend/RmlUi_Backend.h"
#include <GLFW/glfw3.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/StyleTypes.h>
#include <RmlUi/Debugger.h>
#include <RmlUi/Debugger/Debugger.h>
#include <cassert>
#include <cstdio>
#include <filesystem>
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

        Rml::LoadFontFace(
            memory, "JetBrainsMonoNL-Regular", Rml::Style::FontStyle::Normal,
            Rml::Style::FontWeight::Normal, true
        );

        this->active_document = this->context->LoadDocument("resources/ui/main.rml");

        assert(active_document);

        this->active_document->Show();

        Rml::Debugger::Initialise(this->context);

        Rml::Debugger::SetVisible(true);

        this->main_loop();
    }

    void Application::main_loop() {
        while (this->running) {
            this->running = Backend::ProcessEvents(this->context);

            // Call into our code

            // Tell the context it happened
            this->context->Update();

            Backend::BeginFrame();
            this->context->Render();
            Backend::PresentFrame();
        }
    }

} // namespace soil