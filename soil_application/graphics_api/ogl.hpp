#pragma once
#include "texture.hpp"

namespace soil {

    struct OglTexture : BaseTexture {
        uint32_t open_gl_handle = {};
    };

    class OglTextureHandle : public TextureHandle {
    public:
        using TextureHandle::TextureHandle;
        using TextureHandle::operator<=>;
    };

    using OglTextureLookup = TextureLookup<OglTextureHandle, OglTexture>;

    class OGL {};

} // namespace soil

template <> struct std::hash<soil::OglTextureHandle> {
    size_t operator()(const soil::OglTextureHandle& handle) const noexcept {
        return handle.get_handle();
    }
}; // namespace std