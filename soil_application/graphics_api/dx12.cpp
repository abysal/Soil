

#include "./dx12.hpp"
#include "./windows.hpp"
#include <bit>
#include <combaseapi.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include <winrt/base.h>

#include <directx/d3d12.h>
#include <directx/d3dcommon.h>

#include <RmlUi/Core/Log.h>
#include <combaseapi.h>
#include <directx/d3dx12.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
namespace soil {

    // Stolen from the MS repo
    void get_hardware_adapter(
        IDXGIFactory1* factory, IDXGIAdapter1** out_adapter, bool high_performance
    ) {
        *out_adapter = nullptr;

        winrt::com_ptr<IDXGIAdapter1> adapter;

        winrt::com_ptr<IDXGIFactory6> factory6;
        if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
            for (UINT adapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                     adapterIndex,
                     high_performance == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
                                              : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                     IID_PPV_ARGS(&adapter)
                 ));
                 ++adapterIndex) {
                DXGI_ADAPTER_DESC1 desc;
                throw_on_fail(adapter->GetDesc1(&desc));

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(
                        adapter.get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr
                    ))) {
                    break;
                }
            }
        }

        if (adapter.get() == nullptr) {
            for (UINT adapterIndex = 0;
                 SUCCEEDED(factory->EnumAdapters1(adapterIndex, adapter.put()));
                 ++adapterIndex) {
                DXGI_ADAPTER_DESC1 desc;
                throw_on_fail(adapter->GetDesc1(&desc));

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(
                        adapter.get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr
                    ))) {
                    break;
                }
            }
        }

        *out_adapter = adapter.detach();
    }

    size_t D3D12::align(size_t memory, size_t alignment) {
        assert(!((0 == alignment) || (alignment & (alignment - 1))));
        // Non pow-2 alignment

        return ((memory + (alignment + 1)) & ~(alignment - 1));
    }

    size_t D3D12::allocate(size_t size, size_t alignment) {
        this->gpu_memory.current = this->align(this->gpu_memory.current, alignment);

        const auto offset         = this->gpu_memory.current - this->gpu_memory.base;
        this->gpu_memory.current += size;

        if (this->gpu_memory.current > this->gpu_memory.end) {
            throw std::bad_alloc();
        }

        return offset;
    }

    DXGI_FORMAT D3D12::Dx12TextureInfo::current_format() const {
        switch (this->format) {
        case DxTextureFormat::RGBA8: {
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        }
        default:
            assert(false);
        }
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    size_t D3D12::Dx12TextureInfo::bpp() const {
        // TODO: Make this actually do work
        return 4;
    }

    winrt::com_ptr<ID3D12Resource> D3D12::cpu_upload(const Dx12TextureInfo info) {
        D3D12_SUBRESOURCE_FOOTPRINT texture_info = {
            .Format   = info.current_format(),
            .Width    = info.width,
            .Height   = info.height,
            .Depth    = 1,
            .RowPitch = static_cast<uint32_t>(
                this->align(info.width * info.bpp(), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
            )
        };

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT placed_footprint = {
            .Offset =
                this->allocate(info.width * info.bpp(), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT),
            .Footprint = texture_info
        };

        const std::byte* memory = info.data;

        for (uint32_t y = 0; y < info.height; ++y) {
            auto* current_line = std::bit_cast<std::byte*>(
                this->gpu_memory.base + placed_footprint.Offset + y * texture_info.RowPitch
            );

            for (uint32_t x = 0; x < info.width; ++x) {
                memcpy(
                    current_line + x * info.bpp(), &memory[y * info.width + x * info.bpp()],
                    info.bpp()
                );
            }
        }

        D3D12_RESOURCE_DESC text_desc = {
            text_desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            text_desc.Alignment          = 0,           // Default alignment
            text_desc.Width              = info.width,  // Texture width
            text_desc.Height             = info.height, // Texture height
            text_desc.DepthOrArraySize   = 1,           // Texture depth (1 for 2D textures)
            text_desc.MipLevels          = 1,           // Number of mipmap levels
            text_desc.Format             = info.current_format(), // Texture format
            text_desc.SampleDesc.Count   = 1,                     // No multisampling
            text_desc.SampleDesc.Quality = 0,
            text_desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN, // Default layout
            text_desc.Flags              = D3D12_RESOURCE_FLAG_NONE,
        };

        winrt::com_ptr<ID3D12Resource> texture;

        const auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        throw_on_fail(this->device->CreateCommittedResource(
            &properties, D3D12_HEAP_FLAG_SHARED, &text_desc, D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr, IID_PPV_ARGS(&texture)
        ));

        const auto dest = CD3DX12_TEXTURE_COPY_LOCATION(texture.get(), 0);
        const auto source =
            CD3DX12_TEXTURE_COPY_LOCATION(this->gpu_upload_buffer.get(), placed_footprint);

        this->command_list->CopyTextureRegion(
            &dest,   // Destination (GPU texture)
            0, 0, 0, // Destination position (0,0,0)
            &source, // Source (upload buffer)
            nullptr  // Optional, source box (can be nullptr)
        );

        const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            texture.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON
        );

        this->command_list->ResourceBarrier(1, &barrier);
        return texture;

    } // namespace soil

    void D3D12::flush_cq() {
        throw_on_fail(this->command_list->Close());

        auto*              command_queue = this->command_queue.get();
        ID3D12CommandList* command_lists = this->command_list.get();

        ID3D12Fence* fence = nullptr;

        throw_on_fail(this->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))
        );

        const auto     fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        const uint64_t fence_value = 1;

        command_queue->ExecuteCommandLists(1, &command_lists);
        command_queue->Signal(fence, fence_value);

        if (fence->GetCompletedValue() != fence_value) {
            fence->SetEventOnCompletion(fence_value, fence_event);
            WaitForSingleObject(fence, INFINITE);
        }

        throw_on_fail(this->command_allocator->Reset());
        throw_on_fail(
            this->command_list->Reset(this->command_allocator.get(), this->pipeline_state.get())
        );
    }

    void D3D12::init_buffers() {

        constexpr static size_t alloc_size = 1024 * 1024 * 50;
        const auto              buff       = CD3DX12_RESOURCE_DESC::Buffer(alloc_size);
        const auto              heap_prop  = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        throw_on_fail(this->device->CreateCommittedResource(
            &heap_prop, D3D12_HEAP_FLAG_NONE, &buff, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&this->gpu_upload_buffer)
        ));

        void* data = nullptr;

        const CD3DX12_RANGE read_range(0, 0);

        throw_on_fail(this->gpu_upload_buffer->Map(0, &read_range, &data));

        this->gpu_memory.current = this->gpu_memory.base = std::bit_cast<uintptr_t>(data);

        this->gpu_memory.end = this->gpu_memory.base + alloc_size;
    }

    D3D12::D3D12() {
        winrt::com_ptr<IDXGIFactory4> factory;
        throw_on_fail(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

        winrt::com_ptr<IDXGIAdapter1> hardware;

        get_hardware_adapter(factory.get(), hardware.put(), true);

        throw_on_fail(D3D12CreateDevice(
            hardware.get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&this->device)
        ));

        // Build command queue
        D3D12_COMMAND_QUEUE_DESC queue_desc = {};
        queue_desc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queue_desc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;

        throw_on_fail(this->device->CreateCommandQueue(
            &queue_desc, IID_PPV_ARGS(this->command_queue.put())
        ));
        throw_on_fail(this->device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->command_allocator)
        ));
        throw_on_fail(this->device->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->command_allocator.get(), nullptr,
            IID_PPV_ARGS(&this->command_list)
        ));

        throw_on_fail(this->command_list->Close());
        throw_on_fail(this->command_allocator->Reset());
        throw_on_fail(
            this->command_list->Reset(this->command_allocator.get(), this->pipeline_state.get())
        );
        this->init_buffers();
    }
} // namespace soil