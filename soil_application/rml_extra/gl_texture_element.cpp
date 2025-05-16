//
// Created by Akashic on 5/16/2025.
//

#include "gl_texture_element.hpp"

#include <RmlUi/Core/ElementUtilities.h>

namespace soil {
    bool GLTextureElement::GetIntrinsicDimensions(Rml::Vector2f& dimensions, float& ratio) {
        load_handle();
        const float dp_ratio = Rml::ElementUtilities::GetDensityIndependentPixelRatio(this);

        this->dimensions.x = [&] {
            if (this->HasAttribute("width")) {
                return this->GetAttribute<float>("width", -1);
            }
            const auto texture = this->lookup->find(this->our_texture);
            assert(texture.has_value());

            return texture->get().width;
        }();
        this->dimensions.y = [&] {
            if (this->HasAttribute("height")) {
                return this->GetAttribute<float>("height", -1);
            }
            const auto texture = this->lookup->find(this->our_texture);
            assert(texture.has_value());

            return texture->get().height;
        }();
        this->scale = dp_ratio;

        // Rescales the data so it works :3
        this->dimensions *= this->scale;

        dimensions = this->dimensions;

        return true;
    }

    void GLTextureElement::load_handle() {
        if (this->our_texture.is_valid()) {
            return;
        }

        const auto texture_id = this->GetAttribute<std::string>("texture", "");

        if (texture_id.empty()) {
            throw std::runtime_error("missing texture attrib");
        }

        this->our_texture = OglTextureHandle(texture_id);
    }
} // namespace soil