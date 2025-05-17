//
// Created by Akashic on 5/16/2025.
//

#include "gl_texture_element.hpp"

#include <RmlUi/Core/ElementUtilities.h>

namespace soil {
    bool GLTextureElement::GetIntrinsicDimensions(Rml::Vector2f& dimensions, float& ratio) {
        load_handle();
        const float dp_ratio = Rml::ElementUtilities::GetDensityIndependentPixelRatio(this);
        const auto& texture  = this->gl->lookup.find(this->handle).value().get();

        this->dimension.x = [&] {
            if (this->HasAttribute("width")) return this->GetAttribute<float>("width", -1);
            return static_cast<float>(texture.width);
        }();

        this->dimension.y = [&] {
            if (this->HasAttribute("height")) return this->GetAttribute<float>("height", -1);
            return static_cast<float>(texture.height);
        }();

        this->dimension *= this->dimension_scale;

        dimensions = std::bit_cast<Rml::Vector2f>(this->dimension);
        ratio      = dp_ratio;

        return true;
    }
    void GLTextureElement::OnAttributeChange(const Rml::ElementAttributes& changed_attributes) {
        Element::OnAttributeChange(changed_attributes);

        bool dirty = false;

        if (changed_attributes.find("texture") != changed_attributes.end()) {
            dirty               = true;
            this->dirty_texture = true;
        }

        if (changed_attributes.find("width") != changed_attributes.end() ||
            changed_attributes.find("height") != changed_attributes.end()) {
            dirty = true;
        }

        if (dirty) {
            DirtyLayout();
            this->dirty_geo = true;
        }
    }

    void GLTextureElement::OnRender() {
        if (this->dirty_geo) this->build_geo();
        this->load_handle();

        this->gl->draw_texture(this->mesh, this->handle);
    }

    void GLTextureElement::load_handle() {
        if (!this->dirty_texture) {
            return;
        }
        this->dirty_texture = false;

        const auto texture_id = this->GetAttribute<std::string>("texture", "");

        if (texture_id.empty()) {
            return;
        }

        const auto handle = this->gl->lookup.find(OglTextureHandle(texture_id));

        if (!handle.has_value()) {
            return;
        }

        const float dp_ratio  = Rml::ElementUtilities::GetDensityIndependentPixelRatio(this);
        this->dimension_scale = dp_ratio;

        this->handle = OglTextureHandle(texture_id);
    }

    // NOLINTNEXTLINE
    void GLTextureElement::free_texture() {}

    void GLTextureElement::build_geo() {
        const auto position =
            std::bit_cast<glm::vec2>(this->GetAbsoluteOffset(Rml::BoxArea::Border));
        const auto size = this->dimension;
        if (this->mesh.vertex_array_object == 0xFFFFFFFF) {
            this->mesh = OGL::build_mesh_rect(position, size);
        } else {
            OGL::build_mesh_rect(this->mesh, position, size);
        }

        this->dirty_geo = false;
    }
} // namespace soil