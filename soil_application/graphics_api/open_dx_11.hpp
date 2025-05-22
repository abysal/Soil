#pragma once
#include "dx11.hpp"
#include "ogl.hpp"

#include <gsl/pointers>

namespace soil {

    class OpenDx11 {
    public:
        OpenDx11(gsl::not_null<D3D11*> direct_x11, gsl::not_null<OGL*> open_gl)
            : direct_x11(direct_x11), open_gl(open_gl) {}

        [[nodiscard]] std::optional<OglTexture> bind(const D11TextureHandle texture);
        [[nodiscard]] static OglTexture         bind(const D11Texture& texture);

    private:
        gsl::not_null<D3D11*> direct_x11;
        gsl::not_null<OGL*>   open_gl;
    };

} // namespace soil
