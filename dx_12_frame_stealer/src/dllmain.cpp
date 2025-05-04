#include "./ipc.hpp"
#include "directx/d3d12.h"
#include "safetyhook/easy.hpp"
#include <combaseapi.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <errhandlingapi.h>
#include <handleapi.h>
#include <kiero.hpp>
#include <safetyhook.hpp>
#include <stdexcept>
#include <winbase.h>
#include <windows.h>
#include <winerror.h>
#include <winnt.h>
#include <winrt/base.h>
#include <winuser.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
constexpr size_t NOT_BUFFER = 2910380291;

class Buffers {
private:
    auto current_buff() { return this->buffers[write_front]; }

public:
    Buffers() = default;

    void flip() {
        this->write_front++;

        if (write_front > 2) {
            write_front = 0;
        }
    }

    size_t is_in_buffer(ID3D12Resource* ptr) {
        for (int x = 0; x < buffers.size(); x++) {
            if (this->buffers[x].first == ptr) {
                return x;
            }
        }
        return NOT_BUFFER;
    }

    dx_ipc::Dx12TextureDesc handle() { return this->current_buff().second; }
    auto*                   ptr() { return this->current_buff().first; }

    void clear() {
        CloseHandle((HANDLE)this->buffers[0].second.render_handle);
        CloseHandle((HANDLE)this->buffers[1].second.render_handle);
        CloseHandle((HANDLE)this->buffers[2].second.render_handle);
        buffers[0]  = {0, {}};
        buffers[1]  = {0, {}};
        buffers[2]  = {0, {}};
        write_front = 0;
    }

    bool populated() {
        return this->buffers[0].first != 0 && this->buffers[1].first != 0 &&
               this->buffers[2].first != 0;
    }

    void update(ID3D12Resource* buff, dx_ipc::Dx12TextureDesc handle) {
        this->buffers[write_front] = {buff, handle};
    }

private:
    std::array<std::pair<ID3D12Resource*, dx_ipc::Dx12TextureDesc>, 3> buffers{};
    int                                                                write_front{};
};

SafetyHookInline present_hook{};
SafetyHookInline create_committed_resource_hook{};
SafetyHookInline execute_command_lists{};

volatile dx_ipc::Dx12Ipc* ipc_state;
ID3D12CommandQueue*       command_queue;
Buffers                   buffers;

void wait_on_frame() {
    while (!ipc_state->consumed_frame) {}

    ipc_state->consumed_frame = false;
}

void per_frame_logic(IDXGISwapChain3* swap) {
    winrt::com_ptr<ID3D12Device> device;
    swap->GetDevice(IID_PPV_ARGS(&device));

    winrt::com_ptr<ID3D12CommandAllocator> command_allocator;

    if (const auto hr = device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator)
        );
        !SUCCEEDED(hr)) {
        ipc_state->last_error = hr;
        return;
    }

    winrt::com_ptr<ID3D12GraphicsCommandList> command_list;

    if (const auto hr = device->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator.get(), nullptr,
            IID_PPV_ARGS(&command_list)
        );
        !SUCCEEDED(hr)) {
        ipc_state->last_error = hr;
        return;
    }

    D3D12_RESOURCE_BARRIER transition{
        .Type  = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .Transition =
            D3D12_RESOURCE_TRANSITION_BARRIER{
                .pResource   = buffers.ptr(),
                .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                .StateBefore = D3D12_RESOURCE_STATE_PRESENT,
                .StateAfter  = D3D12_RESOURCE_STATE_GENERIC_READ
            }
    };

    command_list->Reset(command_allocator.get(), nullptr);
    command_list->ResourceBarrier(1, &transition);
    command_list->Close();

    const auto* list = command_list.get();

    command_queue->ExecuteCommandLists(1, (ID3D12CommandList**)&list);

    wait_on_frame();
    while (true) {};
}

HRESULT idxgi_swap_chain_present_hook(IDXGISwapChain3* self, UINT sync_interval, UINT flags) {
    if (!command_queue) {
        return present_hook.stdcall<HRESULT>(self, sync_interval, flags);
    }

    winrt::com_ptr<ID3D12Resource> buffer;

    IDXGISwapChain3* swap_chain3 = self;

    const auto idx = swap_chain3->GetCurrentBackBufferIndex();
    swap_chain3->GetBuffer(idx, IID_PPV_ARGS(&buffer));

    auto* buff = buffer.get();

    if (buffers.is_in_buffer(buff) != NOT_BUFFER && buffers.populated()) {
        wait_on_frame();
        ipc_state->render_texture_info = buffers.handle();
        ipc_state->back_buffer_pointer = (uint64_t)buffers.ptr();

        per_frame_logic(swap_chain3);

        buffers.flip();
        return present_hook.stdcall<HRESULT>(self, sync_interval, flags);
    }

    if (buffers.populated()) {
        buffers.clear();
    }

    ipc_state->back_buffer_pointer = (uint64_t)buff;

    HANDLE buffer_handle = nullptr;

    winrt::com_ptr<ID3D12Device> device;

    swap_chain3->GetDevice(IID_PPV_ARGS(&device));

    HRESULT hr =
        device->CreateSharedHandle(buff, nullptr, GENERIC_ALL, nullptr, &buffer_handle);

    if (!SUCCEEDED(hr)) {
        ipc_state->last_error = hr;

        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&ipc_state->string_buffer, 512,
            nullptr
        );

        return present_hook.stdcall<HRESULT>(self, sync_interval, flags);
    }

    hr = DuplicateHandle(
        GetCurrentProcess(), buffer_handle, (HANDLE)ipc_state->soil_handle.load(),
        &buffer_handle, GENERIC_ALL, true, 0
    );

    if (!SUCCEEDED(hr)) {
        ipc_state->last_error = hr;
        return present_hook.stdcall<HRESULT>(self, sync_interval, flags);
    }

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
    UINT64                             total_size;
    D3D12_RESOURCE_DESC                resource_disc = buffer->GetDesc();
    uint32_t                           row_count;

    device->GetCopyableFootprints(
        &resource_disc, 0, 1, 0, &layout, &row_count, nullptr, &total_size
    );

    auto handle_info = dx_ipc::Dx12TextureDesc{
        .render_handle = (uint64_t)buffer_handle,
        .total_size    = total_size,
        .width         = layout.Footprint.Width,
        .height        = layout.Footprint.Height,
        .format        = (uint32_t)layout.Footprint.Format,
    };

    buffers.update(buff, handle_info);
    buffers.flip();
    ipc_state->clear_handled = false;

    return present_hook.stdcall<HRESULT>(self, sync_interval, flags);
}

HRESULT i3d12Device_create_committed_resource(
    ID3D12Device* self, D3D12_HEAP_PROPERTIES* props, D3D12_HEAP_FLAGS heap_flags,
    D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES init_state, D3D12_CLEAR_VALUE* clear_value,
    REFIID riid, void** resource
) {
    if (props->Type == D3D12_HEAP_TYPE_DEFAULT &&
        (desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)) {
        heap_flags  |= D3D12_HEAP_FLAG_SHARED;
        desc->Flags |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
    }

    const auto result = create_committed_resource_hook.call<HRESULT>(
        self, props, heap_flags, desc, init_state, clear_value, riid, resource
    );

    if (!SUCCEEDED(result)) {
        DebugBreak();
    }

    return result;
}

void execute_command_lists_hook(
    ID3D12CommandQueue* self, uint32_t count, ID3D12CommandList* const* lists
) {
    command_queue = self;
    execute_command_lists.call(self, count, lists);
}

extern "C" __declspec(dllexport) void init(void* ipc_handle) {

    const auto mapped_memory = (volatile dx_ipc::Dx12Ipc*)MapViewOfFile(
        ipc_handle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(dx_ipc::Dx12Ipc) * 2
    );

    if (!mapped_memory) {
        MessageBoxA(nullptr, "AAAAAAAAAA", nullptr, 0);
        throw std::runtime_error("failed to map file");
    }

    if (const auto status = kiero::init(kiero::RenderType::D3D12);
        status != kiero::Status::Success) {
        throw std::logic_error("Kiero failed to hook");
    };

    auto p = reinterpret_cast<void*>(kiero::getMethodsTable()[140]);

    present_hook = safetyhook::create_inline(p, (void*)&idxgi_swap_chain_present_hook);
    p            = (void*)kiero::getMethodsTable()[27];

    create_committed_resource_hook =
        safetyhook::create_inline(p, (void*)&i3d12Device_create_committed_resource);
    p = (void*)kiero::getMethodsTable()[54];

    execute_command_lists = safetyhook::create_inline(p, (void*)&execute_command_lists_hook);

    mapped_memory->hooks_init = true;
    mapped_memory->init_magic = dx_ipc::INIT_MAGIC;

    ipc_state = mapped_memory;
}

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL, // handle to DLL module
    DWORD     reason,   // reason for calling function
    LPVOID    lpvReserved
) // reserved
{

    return TRUE; // Successful DLL_PROCESS_ATTACH.
}