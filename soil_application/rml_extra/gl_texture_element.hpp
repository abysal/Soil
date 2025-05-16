#pragma once
#include "graphics_api/ogl.hpp"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Geometry.h>
#include <gsl/pointers>

namespace soil {

    class GLTextureElement final : public Rml::Element {
    public:
        explicit GLTextureElement(gsl::not_null<OglTextureLookup*> lookup)
            : Element("gl_texture"), lookup(lookup) {}

        bool GetIntrinsicDimensions(Rml::Vector2f& dimensions, float& ratio) override;

    private:
        void load_handle();

    private:
        OglTextureHandle                 our_texture{};
        Rml::Vector2f                    dimensions{-1, -1};
        float                            scale{1};
        Rml::Geometry                    geometry;
        gsl::not_null<OglTextureLookup*> lookup;
    };

} // namespace soil
