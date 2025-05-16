#pragma once
#include <directx/d3d12.h>
#include <winrt/base.h>

namespace soil {

    class D3D12 {
    public:
        D3D12();

        enum DxTextureFormat { RGBA8 };

        struct Dx12TextureInfo {
            uint32_t        width  = 0;
            uint32_t        height = 0;
            std::byte*      data   = nullptr;
            DxTextureFormat format = DxTextureFormat::RGBA8;
            DXGI_FORMAT     current_format() const;
            size_t          bpp() const;
        };

        winrt::com_ptr<ID3D12Resource> cpu_upload(const Dx12TextureInfo info);

        void flush_cq();

    private:
        void init_buffers();

        size_t align(size_t value, size_t boundary);
        size_t allocate(size_t size, size_t alignment);

    private:
        winrt::com_ptr<ID3D12Device>                  device;
        winrt::com_ptr<ID3D12CommandAllocator>        command_allocator;
        winrt::com_ptr<ID3D12CommandQueue>            command_queue;
        winrt::com_ptr<ID3D12RootSignature>           root_signature;
        winrt::com_ptr<ID3D12DescriptorHeap>          rtv_heap;
        winrt::com_ptr<ID3D12DescriptorHeap>          srv_heap;
        winrt::com_ptr<ID3D12PipelineState>           pipeline_state;
        winrt::com_ptr<ID3D12GraphicsCommandList>     command_list;
        std::array<winrt::com_ptr<ID3D12Resource>, 2> render_targets;
        winrt::com_ptr<ID3D12Resource>                gpu_upload_buffer;

        struct GpuAllocation {
            uintptr_t base    = 0x0;
            uintptr_t current = 0x0;
            uintptr_t end     = 0x0;
        };

        // TODO: make a proper, but simple, gpu memory allocator. Currently this is just a bump
        // allocator
        GpuAllocation gpu_memory{};
    };
} // namespace soil