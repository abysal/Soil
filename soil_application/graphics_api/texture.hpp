#pragma once
#include "rml_extra/gl_texture_element.hpp"

#include <RmlUi/Core/Types.h>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <winrt/base.h>

namespace soil {
    enum class TextureFormat { RGBA8 };

    template <typename T1, typename T2, typename T = std::common_type_t<T1, T2>>
    T lerp(const T1& a, const T2& b, float t) {
        return static_cast<T>(a) + (static_cast<T>(b) - static_cast<T>(a)) * t;
    }

    template <typename T1, typename T2, typename T = std::common_type_t<T1, T2>>
    T lerp(const T1& a, const T2& b, double t) {
        return static_cast<T>(a) + (static_cast<T>(b) - static_cast<T>(a)) * t;
    }

    template <typename T>
    std::byte* get_row(std::byte* data, const size_t row, const size_t pixels_per_row) {
        return data + row * pixels_per_row * sizeof(T);
    }

    struct BaseTexture {
        uint32_t      width  = 0;
        uint32_t      height = 0;
        TextureFormat format = TextureFormat::RGBA8;
    };

    class TextureHandle {
    public:
        TextureHandle() = default;
        explicit TextureHandle(const std::string& name) {
            constexpr std::hash<std::string> hash_fn;
            this->handle = hash_fn(name);
        }

        auto operator<=>(const TextureHandle&) const = default;

        bool is_valid() const { return this->handle != 0; }

        size_t get_handle() const { return this->handle; }

    private:
        size_t handle = 0;
    };

    template <typename HandleType, typename TextureType> class TextureLookup {
    public:
        TextureLookup()                                    = default;
        TextureLookup(const TextureLookup&)                = delete;
        TextureLookup& operator=(const TextureLookup&)     = delete;
        TextureLookup(TextureLookup&&) noexcept            = default;
        TextureLookup& operator=(TextureLookup&&) noexcept = default;

        std::optional<std::reference_wrapper<TextureType>>
        find(const OglTextureHandle handle) const {
            return this->operator[](handle);
        }

        std::optional<std::reference_wrapper<TextureType>> operator[](const HandleType handle
        ) const {
            if (this->map.contains(handle)) {
                return this->map[handle];
            }
            return std::nullopt;
        }

        void upload_texture(const HandleType handle, TextureType&& texture) {
            this->map[handle] = std::move(texture);
        }

    private:
        std::unordered_map<HandleType, TextureType> map{};
    };

    struct ApiTexture : public BaseTexture {
        std::byte* data = nullptr;

        explicit ApiTexture(
            const uint32_t width = 0, const uint32_t height = 0, std::byte* data = nullptr,
            const TextureFormat format = TextureFormat::RGBA8
        )
            : width(width), height(height), format(format), data(data) {}

        ApiTexture(const ApiTexture&)            = delete;
        ApiTexture& operator=(const ApiTexture&) = delete;
        ApiTexture(ApiTexture&& other) noexcept {
            this->width  = other.width;
            this->height = other.height;
            this->data   = std::exchange(other.data, nullptr);
            this->format = other.format;
        }
        ApiTexture& operator=(ApiTexture&&) = default;
        ~ApiTexture() {
            if (this->data) delete[] this->data;
        }
    };

    ApiTexture gradient(uint32_t width, uint32_t height, Rml::Colourb start, Rml::Colourb end);

} // namespace soil

template <> struct std::hash<soil::TextureHandle> {
    size_t operator()(const soil::TextureHandle& handle) const noexcept {
        return handle.get_handle();
    }
}; // namespace std