#include "./clay/clay_binding.hpp"
#include "./clay/components.hpp"
#include "Application/Application.hpp"

int main() {
    auto font = setup_basics("Soil");

    auto app = soil::Application();

    render_loop([&] { app.render(); }, std::move(font));
}
