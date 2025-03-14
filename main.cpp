#include "./clay/clay_binding.hpp"
#include "Application/application.hpp"
#include <print>


int main() {
    auto font = setup_basics("Soil");

    auto app = soil::Application();

    render_loop([&] { app.render(); }, std::move(font));
}
