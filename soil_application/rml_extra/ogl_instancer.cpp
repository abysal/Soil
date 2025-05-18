
#include "ogl_instancer.hpp"

#include "gl_texture_element.hpp"

Rml::ElementPtr soil::OglInstancer::InstanceElement(
    Rml::Element* parent, const Rml::String& tag, const Rml::XMLAttributes& attributes
) {
    (void)parent;
    (void)attributes;

    if (tag == "opengl") {
        return Rml::ElementPtr(new GLTextureElement(this->ogl_renderer));
    }

    Rml::Log::Message(Rml::Log::LT_ERROR, "Invalid element: %s", tag);
    return nullptr;
}

void soil::OglInstancer::ReleaseElement(Rml::Element* element) { delete element; }