#include "open_dx_11.hpp"

#include "windows.hpp"

namespace soil {
    std::optional<OglTexture> OpenDx11::bind(const D11TextureHandle texture) {
        auto txt = this->direct_x11->texture_lookup.find(texture);
        if (!txt) {
            return std::nullopt;
        }

        return this->bind(txt.value());
    }

    OglTexture OpenDx11::bind(const D11Texture& texture) {
        HANDLE                         share_handle{};
        winrt::com_ptr<IDXGIResource1> dxgi_handle{};

        throw_on_fail(texture.texture_resource->QueryInterface(dxgi_handle.put()));

        throw_on_fail(dxgi_handle->CreateSharedHandle(
            nullptr, DXGI_SHARED_RESOURCE_READ, nullptr, &share_handle
        ));

        uint32_t texture_handle = 0;
        uint32_t texture_obj    = 0;

        GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &texture_handle));
        GLCall(glCreateMemoryObjectsEXT(1, &texture_obj));

        const auto size = texture.width * texture.height * texture.bytes_per_pixel() *
                          2; // This *2 is some magic number bullshit
        GLCall(glImportMemoryWin32HandleEXT(
            texture_obj, size, GL_HANDLE_TYPE_D3D11_IMAGE_EXT, share_handle
        ));

        GLCall(glBindTexture(GL_TEXTURE_2D, texture_handle));

        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));

        constexpr float border_color[] = {
            1.0f, 1.0f, 1.0f, 1.0f
        }; // Bright white to signify its fucked.
        GLCall(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color));

        GLCall(glTexStorageMem2DEXT(
            texture_handle, 1, GL_RGBA8, texture.width, texture.height, texture_obj, 0
        ));

        return OglTexture{texture.width, texture.height, TextureFormat::RGBA8, texture_handle};
    }

} // namespace soil