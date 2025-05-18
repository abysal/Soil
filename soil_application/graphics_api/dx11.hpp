#pragma once
#include "rml_ui_backend/RmlUi_Platform_GLFW.h"
#include "texture.hpp"

#include <d3d11.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <winrt/base.h>

namespace soil {
    struct D11Texture : public BaseTexture {
        winrt::com_ptr<ID3D11ShaderResourceView> srv{};
        winrt::com_ptr<ID3D11SamplerState>       sampler{};
        winrt::com_ptr<ID3D11Texture2D>          texture_resource{};
        D11Texture() = default;

        D11Texture(
            const uint32_t width, const uint32_t height,
            const winrt::com_ptr<ID3D11ShaderResourceView>& shader_resource_view,
            const winrt::com_ptr<ID3D11SamplerState>&       sampler,
            const winrt::com_ptr<ID3D11Texture2D>&          texture_resource,
            const TextureFormat                             format = TextureFormat::RGBA8
        )
            : BaseTexture(width, height, format), srv(shader_resource_view), sampler(sampler),
              texture_resource(texture_resource) {}

        D11Texture(D11Texture&& other) noexcept {
            this->sampler.attach(other.sampler.detach());
            this->srv.attach(other.srv.detach());
            this->texture_resource.attach(other.texture_resource.detach());
            this->width  = other.width;
            this->height = other.height;
            this->format = other.format;
        }
        D11Texture& operator=(D11Texture&& other) noexcept {

            this->sampler.attach(other.sampler.detach());
            this->srv.attach(other.srv.detach());
            this->texture_resource.attach(other.texture_resource.detach());
            this->width  = other.width;
            this->height = other.height;
            this->format = other.format;
            return *this;
        }
        D11Texture(const D11Texture&)            = delete;
        D11Texture& operator=(const D11Texture&) = delete;

        ~D11Texture();
    };

    class D11TextureHandle : public TextureHandle {
    public:
        using TextureHandle::TextureHandle;
    };

    using D11TextureLookup = TextureLookup<D11TextureHandle, D11Texture>;
} // namespace soil

template <> struct std::hash<soil::D11TextureHandle> {
    size_t operator()(const soil::D11TextureHandle& handle) const noexcept {
        return handle.get_handle();
    }
}; // namespace std

namespace soil {
    class D3D11 {
    public:
        explicit D3D11(bool init_window);
        D3D11(const D3D11&) = delete;
        D3D11(D3D11&&)      = default;

        ~D3D11();
        void update();

        void upload_texture_from_cpu(const ApiTexture& texture, const std::string& name);

        HWND window_hwnd() const;

    public:
        D11TextureLookup texture_lookup{};

        void set_as_render_texture(const D11TextureHandle handle);

        void new_window();

    private:
        struct Vertex {
            glm::vec2 position{};
            glm::vec2 uv{};

            constexpr static std::array<D3D11_INPUT_ELEMENT_DESC, 2> elements = {
                {{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0
                 },
                 {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(glm::vec2),
                  D3D11_INPUT_PER_VERTEX_DATA, 0}}
            };
        };

        static_assert(sizeof(Vertex) == 16);

    private:
        void create_debug_window();

        void load_shaders();

        void setup_vertex_data();

        void handle_resize();

    private:
        /**
         * Storage for information that is only created when a window is generated but can be
         * reused
         */
        struct OffWindowStorage {
            winrt::com_ptr<ID3D10Blob>         error_blob{};
            winrt::com_ptr<ID3D10Blob>         vertex_blob{};
            winrt::com_ptr<ID3D11PixelShader>  pixel_shader{};
            winrt::com_ptr<ID3D10Blob>         fragment_blob{};
            winrt::com_ptr<ID3D11VertexShader> vertex_shader{};
            winrt::com_ptr<ID3D11InputLayout>  vertex_layout{};
            winrt::com_ptr<ID3D11Buffer>       active_mesh_buffer{};
            winrt::com_ptr<ID3D11Buffer>       active_indices_buffer{};
            D11TextureHandle                   active_texture_handle{};
        };

        struct Window {
            GLFWwindow* window = nullptr;
            glm::ivec2  size{800, 600};

            winrt::com_ptr<IDXGISwapChain>         swap_chain{};
            winrt::com_ptr<ID3D10Blob>             error_blob{};
            winrt::com_ptr<ID3D10Blob>             vertex_blob{};
            winrt::com_ptr<ID3D11PixelShader>      pixel_shader{};
            winrt::com_ptr<ID3D10Blob>             fragment_blob{};
            winrt::com_ptr<ID3D11VertexShader>     vertex_shader{};
            winrt::com_ptr<ID3D11InputLayout>      vertex_layout{};
            winrt::com_ptr<ID3D11Buffer>           active_mesh_buffer{};
            winrt::com_ptr<ID3D11RenderTargetView> render_target_view{};
            winrt::com_ptr<ID3D11Buffer>           active_indices_buffer{};
            D11TextureHandle                       active_texture_handle{};

            [[nodiscard]] bool should_be_open() const;

            explicit Window(GLFWwindow* window) : window(window) {}
            Window(const Window&)            = delete;
            Window& operator=(const Window&) = delete;
            Window(Window&& other) noexcept {
                this->window = std::exchange(other.window, nullptr);
            }
            Window& operator=(Window&& other) noexcept {
                this->window = std::exchange(other.window, nullptr);
                return *this;
            }

            ~Window();
        };

        winrt::com_ptr<ID3D11Device>        device{};
        winrt::com_ptr<ID3D11DeviceContext> device_context{};
        std::optional<Window>               our_window         = std::nullopt;
        std::optional<OffWindowStorage>     off_window_storage = std::nullopt;
    };

} // namespace soil
