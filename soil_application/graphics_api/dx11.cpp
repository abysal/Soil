
#include "dx11.hpp"

#include <dxgi.h>
#include <dxgitype.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "windows.hpp"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <GLFW/glfw3native.h>
#include <WICTextureLoader.h>
#include <glm/vec4.hpp>

constexpr auto* shader_source = R"(
    Texture2D ObjTexture;
    SamplerState ObjSamplerState;

    struct VS_OUTPUT
    {
        float4 position : SV_POSITION;   // clip space position, float4
        float2 TexCoord : TEXCOORD0;     // texture coordinates
    };

    VS_OUTPUT VS(float2 position : POSITION, float2 uv : TEXCOORD0)
    {
        VS_OUTPUT output;
        output.position = float4(position, 0.0f, 1.0f);
        output.TexCoord = uv;
        return output;
    }

    float4 PS(VS_OUTPUT input) : SV_TARGET
    {
        return ObjTexture.Sample(ObjSamplerState, input.TexCoord);
    }
)";

namespace soil {
    // TODO: Make this actually free memory!
    D11Texture::~D11Texture() {}

    D3D11::D3D11(const bool init_window) {
        if (init_window) {
            this->create_debug_window();
        } else {
            return;
        }
    }

    D3D11::~D3D11() {
        if (this->device) {
            this->device->Release();
        }

        if (this->device_context) {
            this->device_context->Release();
        }
    }

    void D3D11::update() {
        if (this->our_window.has_value() && !this->our_window->should_be_open()) {
            this->off_window_storage = OffWindowStorage{
                .error_blob            = this->our_window->error_blob,
                .vertex_blob           = this->our_window->vertex_blob,
                .pixel_shader          = this->our_window->pixel_shader,
                .fragment_blob         = this->our_window->fragment_blob,
                .vertex_shader         = this->our_window->vertex_shader,
                .vertex_layout         = this->our_window->vertex_layout,
                .active_mesh_buffer    = this->our_window->active_mesh_buffer,
                .active_indices_buffer = this->our_window->active_indices_buffer,
                .active_texture_handle = this->our_window->active_texture_handle,
            };

            this->our_window = std::nullopt;
            return;
        }

        if (!this->device_context || !this->our_window) {
            return;
        }

        constexpr glm::vec4 bg_color = {0.1f, 0.9f, 0.4f, 1.f};

        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX       = 0;
        viewport.TopLeftY       = 0;
        viewport.Width          = static_cast<float>(this->our_window->size.x);
        viewport.Height         = static_cast<float>(this->our_window->size.y);
        viewport.MinDepth       = 0.0f;
        viewport.MaxDepth       = 1.0f;
        this->device_context->RSSetViewports(1, &viewport);

        this->device_context->ClearRenderTargetView(
            this->our_window->render_target_view.get(),
            reinterpret_cast<const float*>(&bg_color)
        );

        // Set all necessary pipeline state
        auto*              buffer = this->our_window->active_mesh_buffer.get();
        constexpr uint32_t stride = sizeof(Vertex);
        constexpr uint32_t offset = 0;
        this->device_context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);

        this->device_context->IASetIndexBuffer(
            this->our_window->active_indices_buffer.get(), DXGI_FORMAT_R32_UINT, 0
        );

        this->device_context->IASetInputLayout(this->our_window->vertex_layout.get());
        this->device_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        this->device_context->VSSetShader(this->our_window->vertex_shader.get(), nullptr, 0);
        this->device_context->PSSetShader(this->our_window->pixel_shader.get(), nullptr, 0);

        if (const auto& texture =
                this->texture_lookup.find(this->our_window->active_texture_handle);
            texture.has_value()) {

            auto* tp    = texture.value().get().srv.get();
            auto* state = texture.value().get().sampler.get();

            this->device_context->PSSetShaderResources(0, 1, &tp);
            this->device_context->PSSetSamplers(0, 1, &state);
        }

        this->device_context->DrawIndexed(6, 0, 0);

        throw_on_fail(this->our_window->swap_chain->Present(0, 0));
    }

    void D3D11::upload_texture_from_cpu(const ApiTexture& texture, const std::string& name) {
        winrt::com_ptr<ID3D11ShaderResourceView> srv{};
        winrt::com_ptr<ID3D11Texture2D>          texture_resource{};
        winrt::com_ptr<ID3D11SamplerState>       sampler_state{};

        D3D11_TEXTURE2D_DESC texture_desc{};
        texture_desc.Usage              = D3D11_USAGE_DEFAULT;
        texture_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
        texture_desc.CPUAccessFlags     = 0;
        texture_desc.MiscFlags          = 0;
        texture_desc.SampleDesc.Count   = 1;
        texture_desc.SampleDesc.Quality = 0;
        texture_desc.ArraySize          = 1;
        texture_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
        texture_desc.Width              = texture.width;
        texture_desc.Height             = texture.height;
        texture_desc.MipLevels          = 1;

        D3D11_SUBRESOURCE_DATA texture_data{};
        texture_data.SysMemPitch = texture.width * texture.bytes_per_pixel();
        texture_data.pSysMem     = texture.data;

        throw_on_fail(
            this->device->CreateTexture2D(&texture_desc, &texture_data, texture_resource.put())
        );

        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format                          = texture_desc.Format;
        srv_desc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MostDetailedMip       = 0;
        srv_desc.Texture2D.MipLevels             = 1;

        throw_on_fail(
            device->CreateShaderResourceView(texture_resource.get(), &srv_desc, srv.put())
        );

        D3D11_SAMPLER_DESC sampler_desc{};

        sampler_desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampler_desc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampler_desc.MinLOD         = 0;
        sampler_desc.MaxLOD         = D3D11_FLOAT32_MAX;

        throw_on_fail(this->device->CreateSamplerState(&sampler_desc, sampler_state.put()));

        auto gpu_texture =
            D11Texture(texture.width, texture.height, srv, sampler_state, texture_resource);

        this->texture_lookup.upload_texture(D11TextureHandle(name), std::move(gpu_texture));
    }

    HWND D3D11::window_hwnd() const {
        if (this->our_window) {
            return glfwGetWin32Window(this->our_window->window);
        }
        return nullptr;
    }

    void D3D11::set_as_render_texture(const D11TextureHandle handle) {
        if (this->our_window) {
            this->our_window->active_texture_handle = handle;
        }
    }

    void D3D11::new_window() {
        if (!this->our_window) {
            this->create_debug_window();
        }
    }

    static D3D11* our_ptr = nullptr;

    void D3D11::create_debug_window() {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        this->our_window = std::move(
            Window{
                glfwCreateWindow(800, 600, "Direct X 11 Debug Window", nullptr, nullptr),
            }
        );

        if (this->our_window->window == nullptr) {
            throw std::runtime_error("Failed to create GLFW window");
        }

        our_ptr = this;

        glfwSetWindowSizeCallback(
            this->our_window->window,
            [](GLFWwindow* /*dont care*/, const int width, const int height) {
                our_ptr->our_window->size = {
                    static_cast<float>(width), static_cast<float>(height)
                };

                our_ptr->handle_resize();
            }
        );

        DXGI_MODE_DESC buffer_desc = {};

        buffer_desc.Width            = this->our_window->size.x;
        buffer_desc.Height           = this->our_window->size.y;
        constexpr DXGI_RATIONAL rate = {
            .Numerator = static_cast<uint32_t>(60), .Denominator = 1
        };
        buffer_desc.RefreshRate      = rate;
        buffer_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        buffer_desc.Scaling          = DXGI_MODE_SCALING_UNSPECIFIED;
        buffer_desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;

        DXGI_SWAP_CHAIN_DESC swap_chain_description = {};
        swap_chain_description.BufferDesc           = buffer_desc;
        swap_chain_description.SampleDesc.Count     = 1;
        swap_chain_description.SampleDesc.Quality   = 0;
        swap_chain_description.BufferUsage          = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_description.BufferCount          = 1;
        swap_chain_description.OutputWindow = glfwGetWin32Window(this->our_window->window);
        swap_chain_description.Windowed     = TRUE;
        swap_chain_description.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;

        constexpr auto level = D3D_FEATURE_LEVEL_11_0;

        if (!this->device) {
            throw_on_fail(D3D11CreateDeviceAndSwapChain(
                nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, &level,
                1, D3D11_SDK_VERSION, &swap_chain_description,
                this->our_window->swap_chain.put(), this->device.put(), nullptr,
                this->device_context.put()
            ));
        } else {
            winrt::com_ptr<IDXGIDevice> dxgi_device{};
            throw_on_fail(this->device->QueryInterface(dxgi_device.put()));

            winrt::com_ptr<IDXGIAdapter> adapter{};
            throw_on_fail(dxgi_device->GetAdapter(adapter.put()));

            winrt::com_ptr<IDXGIFactory> factory{};
            throw_on_fail(adapter->GetParent(IID_PPV_ARGS(&factory)));

            throw_on_fail(factory->CreateSwapChain(
                this->device.get(), &swap_chain_description, this->our_window->swap_chain.put()
            ));
        }

        ID3D11Texture2D* back_buffer = nullptr;

        throw_on_fail(this->our_window->swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer)));

        throw_on_fail(this->device->CreateRenderTargetView(
            back_buffer, nullptr, this->our_window->render_target_view.put()
        ));

        back_buffer->Release();
        auto* rtv = this->our_window->render_target_view.get();

        this->device_context->OMSetRenderTargets(1, &rtv, nullptr);

        if (!this->off_window_storage) {
            this->load_shaders();
            this->setup_vertex_data();
        } else {
            auto& [error_blob, vertex_blob, pixel_shader, fragment_blob, vertex_shader, vertex_layout, active_mesh_buffer, active_indices_buffer, active_texture_handle] =
                this->off_window_storage.value();

            this->our_window->error_blob            = std::move(error_blob);
            this->our_window->vertex_blob           = std::move(vertex_blob);
            this->our_window->pixel_shader          = std::move(pixel_shader);
            this->our_window->fragment_blob         = std::move(fragment_blob);
            this->our_window->vertex_shader         = std::move(vertex_shader);
            this->our_window->vertex_layout         = std::move(vertex_layout);
            this->our_window->active_mesh_buffer    = std::move(active_mesh_buffer);
            this->our_window->active_indices_buffer = std::move(active_indices_buffer);
            this->our_window->active_texture_handle = std::move(active_texture_handle);

            this->off_window_storage = std::nullopt;
        }
    }

    void D3D11::load_shaders() {
        try {
            throw_on_fail(D3DCompile(
                shader_source, strlen(shader_source), nullptr, nullptr, nullptr, "VS", "vs_5_0",
                0, 0, this->our_window->vertex_blob.put(), this->our_window->error_blob.put()
            ));

            throw_on_fail(this->device->CreateVertexShader(
                this->our_window->vertex_blob->GetBufferPointer(),
                this->our_window->vertex_blob->GetBufferSize(), nullptr,
                this->our_window->vertex_shader.put()
            ));

            this->device_context->VSSetShader(
                this->our_window->vertex_shader.get(), nullptr, 0
            );

        } catch (HResultError& err) {
            throw std::runtime_error(
                std::format(
                    "Failed to build vertex shader: {}. Shader Error: {}", err.what(),
                    static_cast<const char*>(this->our_window->error_blob->GetBufferPointer())
                )
            );
        }

        try {
            throw_on_fail(D3DCompile(
                shader_source, strlen(shader_source), nullptr, nullptr, nullptr, "PS", "ps_5_0",
                0, 0, this->our_window->fragment_blob.put(), this->our_window->error_blob.put()
            ));

            throw_on_fail(this->device->CreatePixelShader(
                this->our_window->fragment_blob->GetBufferPointer(),
                this->our_window->fragment_blob->GetBufferSize(), nullptr,
                this->our_window->pixel_shader.put()
            ));

            this->device_context->PSSetShader(this->our_window->pixel_shader.get(), nullptr, 0);
        } catch (HResultError& err) {
            throw std::runtime_error(
                std::format(
                    "Failed to build fragment: {}. Shader Error: {}", err.what(),
                    static_cast<const char*>(this->our_window->error_blob->GetBufferPointer())
                )
            );
        }
    }

    void D3D11::setup_vertex_data() {
        constexpr auto vertex_list = std::array{
            Vertex{{-1.0f, -1.f}, {0.f, 0.f}},
            Vertex{{-1.f, 1.f}, {0.f, 1.f}},
            Vertex{{1.f, 1.f}, {1.f, 1.f}},
            Vertex{{1.f, -1.f}, {1.f, 0.0f}},
        };

        D3D11_BUFFER_DESC vertex_buffer_desc = {};

        vertex_buffer_desc.Usage          = D3D11_USAGE_DEFAULT;
        vertex_buffer_desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
        vertex_buffer_desc.ByteWidth      = sizeof(vertex_list);
        vertex_buffer_desc.CPUAccessFlags = 0;
        vertex_buffer_desc.MiscFlags      = 0;

        D3D11_SUBRESOURCE_DATA vertex_data = {};
        vertex_data.pSysMem                = vertex_list.data();

        throw_on_fail(this->device->CreateBuffer(
            &vertex_buffer_desc, &vertex_data, this->our_window->active_mesh_buffer.put()
        ));

        constexpr auto indices_list = std::array{0, 1, 2, 0, 2, 3};

        D3D11_BUFFER_DESC indices_buffer_desc = {};

        indices_buffer_desc.Usage          = D3D11_USAGE_DEFAULT;
        indices_buffer_desc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
        indices_buffer_desc.ByteWidth      = sizeof(indices_list);
        indices_buffer_desc.CPUAccessFlags = 0;
        indices_buffer_desc.MiscFlags      = 0;

        D3D11_SUBRESOURCE_DATA indices_data = {};
        indices_data.pSysMem                = indices_list.data();

        throw_on_fail(this->device->CreateBuffer(
            &indices_buffer_desc, &indices_data, this->our_window->active_indices_buffer.put()
        ));

        auto*              vertex_buffer = this->our_window->active_mesh_buffer.get();
        constexpr uint32_t vertex_stride = sizeof(Vertex);
        constexpr uint32_t vertex_offset = 0;
        this->device_context->IASetVertexBuffers(
            0, 1, &vertex_buffer, &vertex_stride, &vertex_offset
        );

        this->device_context->IASetIndexBuffer(
            this->our_window->active_indices_buffer.get(), DXGI_FORMAT_R32_UINT, 0
        );

        throw_on_fail(this->device->CreateInputLayout(
            Vertex::elements.data(), Vertex::elements.size(),
            this->our_window->vertex_blob->GetBufferPointer(),
            this->our_window->vertex_blob->GetBufferSize(),
            this->our_window->vertex_layout.put()
        ));

        this->device_context->IASetInputLayout(this->our_window->vertex_layout.get());

        this->device_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void D3D11::handle_resize() {
        this->our_window->render_target_view->Release();
        this->our_window->render_target_view.detach();

        throw_on_fail(this->our_window->swap_chain->ResizeBuffers(
            1, static_cast<uint32_t>(this->our_window->size.x),
            static_cast<uint32_t>(this->our_window->size.y), DXGI_FORMAT_UNKNOWN, 0
        ));

        ID3D11Texture2D* back_buffer = nullptr;
        throw_on_fail(this->our_window->swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer)));

        throw_on_fail(this->device->CreateRenderTargetView(
            back_buffer, nullptr, this->our_window->render_target_view.put()
        ));

        back_buffer->Release();
        auto* target = this->our_window->render_target_view.get();

        this->device_context->OMSetRenderTargets(1, &target, nullptr);
    }

    bool D3D11::Window::should_be_open() const { return !glfwWindowShouldClose(this->window); }

    D3D11::Window::~Window() {
        if (this->window) {
            glfwDestroyWindow(this->window);
        }

        if (this->swap_chain) {
            this->swap_chain->Release();
        }
    }
} // namespace soil