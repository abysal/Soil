#pragma once
#include "graphics_api/ogl.hpp"

#include <RmlUi/Core/Element.h>
#include <gsl/pointers>

namespace soil {

    class GLTextureElement final : public Rml::Element {
    public:
        explicit GLTextureElement(gsl::not_null<OGL*> gl) : Element("gl_texture"), gl(gl) {}

        bool GetIntrinsicDimensions(Rml::Vector2f& dimensions, float& ratio) override;

        ~GLTextureElement() override {
            if (this->mesh.vertex_array_object != 0xFFFFFFFF) {
                this->mesh.unload();
            }
        }

    protected:
        void OnAttributeChange(const Rml::ElementAttributes& changed_attributes) override;

        void OnDpRatioChange() override { this->DirtyLayout(); }

        void OnResize() override { this->dirty_geo = true; }

        void OnRender() override;

    private:
        void load_handle();

        void free_texture();

        void build_geo();

    private:
        gsl::not_null<OGL*> gl;
        OglTextureHandle    handle;
        float               dimension_scale{0};
        bool                dirty_geo{false};
        bool                dirty_texture{false};
        glm::vec2           dimension{-1, -1};
        Mesh                mesh{.vertex_array_object = 0xFFFFFFFF};
    };

} // namespace soil
