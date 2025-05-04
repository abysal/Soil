#include "./dx_creation.hpp"
#include "directx/d3d12.h"

#include <combaseapi.h>
#include <cstring>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgiformat.h>
#include <minwindef.h>
#include <print>
#include <stdexcept>
#include <winerror.h>
#include <winnt.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace D3D {
    void throw_on_fail(HRESULT code) {
        if (!SUCCEEDED(code)) {
            std::println("{}", code);
            throw std::logic_error("Dx Error");
        }
    }

    static HRESULT GetD3D12Addapter(IDXGIAdapter1** adapter) {

        WRL::ComPtr<IDXGIFactory4> pFactory;
        HRESULT                    hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
        if (FAILED(hr)) return hr;
        // Enumerate through the adapters
        WRL::ComPtr<IDXGIAdapter1> pAdapter;
        for (UINT i = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(i, &pAdapter); ++i) {
            DXGI_ADAPTER_DESC1 desc;
            pAdapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                continue;
            }

            WRL::ComPtr<ID3D12Device> pDevice;
            hr = D3D12CreateDevice(
                pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pDevice)
            );
            if (SUCCEEDED(hr)) {
                *adapter = pAdapter.Detach();
                return S_OK;
            }
        }

        return E_FAIL; // No suitable adapter found
    }

    D3D12::D3D12() {
        DEBUG_ONLY(throw_on_fail(D3D12GetDebugInterface(IID_PPV_ARGS(&this->debug_controller)));
                   this->debug_controller->EnableDebugLayer();)

        throw_on_fail(CreateDXGIFactory1(IID_PPV_ARGS(&this->factory)));

        throw_on_fail(GetD3D12Addapter(&this->hardware));

        throw_on_fail(D3D12CreateDevice(
            this->hardware.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&this->device)
        ));

        D3D12_COMMAND_QUEUE_DESC desk = {
            .Type  = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        };

        throw_on_fail(this->device->CreateCommandQueue(&desk, IID_PPV_ARGS(&this->queue)));
        throw_on_fail(this->device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->alloc)
        ));
        throw_on_fail(this->device->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->alloc.Get(), nullptr,
            IID_PPV_ARGS(&this->list)
        ));

        throw_on_fail(this->init_buffers());
    }

    HRESULT D3D12::init_buffers() noexcept {

        constexpr static usize alloc_size = 1024 * 1024 * 50;
        auto                   buff       = CD3DX12_RESOURCE_DESC::Buffer(alloc_size);
        auto                   heap_prop  = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        if (const auto result = this->device->CreateCommittedResource(
                &heap_prop, D3D12_HEAP_FLAG_NONE, &buff, D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr, IID_PPV_ARGS(&this->upload_buffer)
            );
            !SUCCEEDED(result)) {
            return result;
        }

        void* data;

        CD3DX12_RANGE read_range(0, 0);

        this->upload_buffer->Map(0, &read_range, &data);

        this->buffer_cur = this->buffer_start = reinterpret_cast<u8*>(data);
        this->buffer_end                      = this->buffer_start + alloc_size;
        return 0;
    }

    usize D3D12::allocate(usize size, usize align) noexcept {
        this->buffer_cur = (u8*)this->align((usize)this->buffer_cur, (u32)align);

        usize offset = this->buffer_cur - this->buffer_start;

        if (buffer_cur + size) {
            return 0;
        } else {
            return offset;
        }
    }

    void D3D12::upload_debug_texture() noexcept {

        Image debug_texture =
            GenImageGradientLinear(500, 500, 90, {255, 182, 30, 255}, {255, 182, 30, 255});

        this->present_width = this->present_height = 500;

        D3D12_SUBRESOURCE_FOOTPRINT image_footprint = {
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .Width  = 500,
            .Height = 500,
            .Depth  = 1,
            .RowPitch =
                (u32)D3D12::align(500 * sizeof(DWORD), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
        };

        const auto offset = this->allocate(500 * 500, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT t2d = {
            .Offset = offset, .Footprint = image_footprint
        };

        for (u32 y = 0; y < 500; y++) {
            auto* line     = this->buffer_start + t2d.Offset + y * image_footprint.RowPitch;
            u8*   img_data = (u8*)debug_texture.data;
            for (u32 x = 0; x < 500; x++) {
                memcpy(
                    line + x * sizeof(DWORD), &(img_data[(y * 500 + x) * sizeof(DWORD)]),
                    sizeof(DWORD)
                );
            }
        }

        D3D12_RESOURCE_DESC text_desc = {
            text_desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            text_desc.Alignment          = 0,   // Default alignment
            text_desc.Width              = 500, // Texture width
            text_desc.Height             = 500, // Texture height
            text_desc.DepthOrArraySize   = 1,   // Texture depth (1 for 2D textures)
            text_desc.MipLevels          = 1,   // Number of mipmap levels
            text_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM, // Texture format
            text_desc.SampleDesc.Count   = 1,                          // No multisampling
            text_desc.SampleDesc.Quality = 0,
            text_desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN, // Default layout
            text_desc.Flags              = D3D12_RESOURCE_FLAG_NONE,
        };

        this->present_texture_description = text_desc;

        auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        throw_on_fail(this->device->CreateCommittedResource(
            &properties, D3D12_HEAP_FLAG_SHARED, &text_desc, D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr, IID_PPV_ARGS(&this->present_texture)
        ));

        auto dest   = CD3DX12_TEXTURE_COPY_LOCATION(this->present_texture.Get(), 0);
        auto source = CD3DX12_TEXTURE_COPY_LOCATION(this->upload_buffer.Get(), t2d);

        this->list->CopyTextureRegion(
            &dest,   // Destination (GPU texture)
            0, 0, 0, // Destination position (0,0,0)
            &source, // Source (upload buffer)
            nullptr  // Optional, source box (can be nullptr)
        );

        UnloadImage(debug_texture);

        std::println("Created Command List");

        this->setup_shared_present();
        this->execute_command_list();
    }

    void D3D12::copy_to_present(ID3D12Resource* source) noexcept {
        D3D12_RESOURCE_BARRIER source_bar = CD3DX12_RESOURCE_BARRIER::Transition(
            source, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE
        );

        D3D12_RESOURCE_BARRIER presentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            this->present_texture.Get(), D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_COPY_DEST
        );
    }

    void D3D12::execute_command_list() noexcept {

        std::println("Executing Command List");
        this->list->Close();

        ID3D12CommandQueue* cq    = this->queue.Get();
        ID3D12CommandList*  cls[] = {this->list.Get()};

        ID3D12Fence* fence;

        throw_on_fail(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

        HANDLE fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        u64    fence_value = 1; // Fence value to track GPU completion

        cq->ExecuteCommandLists(1, cls);
        cq->Signal(fence, fence_value);

        if (fence->GetCompletedValue() < fence_value) {
            fence->SetEventOnCompletion(fence_value, fence_event);
            WaitForSingleObject(fence_event, INFINITE);
        }

        std::println("Ran Command List");
    }

    void D3D12::setup_shared_present() noexcept {
        D3D12_HEAP_PROPERTIES heapProps = {.Type = D3D12_HEAP_TYPE_DEFAULT};

        throw_on_fail(this->device->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_SHARED, &this->present_texture_description,
            D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&this->present_texture)
        ));

        throw_on_fail(this->device->CreateSharedHandle(
            this->present_texture.Get(), nullptr, GENERIC_ALL, nullptr,
            &this->present_texture_handle
        ));
    }
} // namespace D3D