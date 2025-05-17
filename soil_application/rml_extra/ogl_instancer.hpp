#pragma once
#include "graphics_api/ogl.hpp"

#include <RmlUi/Core/ElementInstancer.h>
#include <gsl/pointers>

namespace soil {

    class OglInstancer : public Rml::ElementInstancer {
    public:
        OglInstancer(gsl::not_null<OGL*> ogl_renderer) : ogl_renderer(ogl_renderer) {}

        Rml::ElementPtr InstanceElement(
            Rml::Element* parent, const Rml::String& tag, const Rml::XMLAttributes& attributes
        ) override;

        void ReleaseElement(Rml::Element* element) override;

    private:
        gsl::not_null<OGL*> ogl_renderer;
    };

} // namespace soil
