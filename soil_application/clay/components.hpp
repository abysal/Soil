#pragma once
#include "./components/header.hpp"
#include "./components/header_button.hpp"

namespace clay_extension {
    enum class ComponentType : i32 { RAYLIB_3D_MODEL, CUSTOM_VIRTUAL };

    class RenderComponent {
    public:
        virtual ~RenderComponent() = 0;
        virtual void on_render() noexcept = 0;
    };

    struct RenderComponentStore {
        ComponentType type;
        RenderComponent* component;
    };
} // namespace clay_extension