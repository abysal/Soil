#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <clay/raywrapped.hpp> // Ok if this moves, the world ends
#define DrawTextEx Win_DrawTextEx
#include <directx/d3dx12.h> // MUST go here

#include "../types.hpp"
#include <assert.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <winnt.h>
#include <wrl/client.h>
#undef DrawTextEx

namespace ogl {
    class OpenGl;
}

namespace D3D {

    void throw_on_fail(HRESULT code);

    namespace WRL = Microsoft::WRL;

    class D3D12 {
    public:
        D3D12();

        D3D12(const D3D12&) = delete;
        D3D12(D3D12&&)      = default;

        void upload_debug_texture() noexcept;

        static inline usize align(usize location, u32 alignment) noexcept {
            assert(!((0 == alignment) || (alignment & (alignment - 1))));
            // Non pow-2 alignement

            return ((location + (alignment + 1)) & ~(alignment - 1));
        }

    private:
        HRESULT init_buffers() noexcept;

        usize allocate(usize size, usize align) noexcept;

        void execute_command_list() noexcept;

        void setup_shared_present() noexcept;

        void copy_to_present(ID3D12Resource* source) noexcept;

    private:
        DEBUG_ONLY(WRL::ComPtr<ID3D12Debug> debug_controller{nullptr};)
        WRL::ComPtr<ID3D12Device>              device{nullptr};
        WRL::ComPtr<IDXGIFactory4>             factory{nullptr};
        WRL::ComPtr<IDXGIAdapter1>             hardware{nullptr};
        WRL::ComPtr<ID3D12CommandQueue>        queue{nullptr};
        WRL::ComPtr<ID3D12CommandAllocator>    alloc{nullptr};
        WRL::ComPtr<ID3D12GraphicsCommandList> list{nullptr};
        WRL::ComPtr<ID3D12Resource>            upload_buffer{nullptr};
        u8*                                    buffer_start{nullptr};
        u8*                                    buffer_cur{nullptr};
        u8*                                    buffer_end{nullptr};
        WRL::ComPtr<ID3D12Resource>            debug_texture{nullptr};
        WRL::ComPtr<ID3D12Resource>            staging_resource{nullptr};
        D3D12_RESOURCE_DESC                    present_texture_description{};
        HANDLE                                 present_texture_handle;
        WRL::ComPtr<ID3D12Resource>            present_texture{nullptr};
        usize                                  present_width{};
        usize                                  present_height{};

        friend ogl::OpenGl;
    };
} // namespace D3D