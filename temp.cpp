struct FrameContext {
    ID3D12CommandAllocator*     commandAllocator            = nullptr;
    ID3D12Resource*             main_render_target_resource = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE main_render_target_descriptor;
};
uint64_t      buffersCounts = -1;
FrameContext* frameContext  = nullptr;

HRESULT hookPresentD3D12(IDXGISwapChain3* ppSwapChain, UINT syncInterval, UINT flags) {
    auto deviceType = ID3D_Device_Type::INVALID_DEVICE_TYPE;
    auto window     = (HWND)FindWindowA(nullptr, (LPCSTR) "Minecraft");
    if (window == NULL) {
        goto out;
    };
    if (SUCCEEDED(ppSwapChain->GetDevice(IID_PPV_ARGS(&d3d11Device)))) {
        deviceType = ID3D_Device_Type::D3D11;
    } else if (SUCCEEDED(ppSwapChain->GetDevice(IID_PPV_ARGS(&d3d12Device)))) {
        deviceType = ID3D_Device_Type::D3D12;
    };
    if (deviceType == ID3D_Device_Type::INVALID_DEVICE_TYPE) {
        goto out;
    };
    if (deviceType == ID3D_Device_Type::D3D11) {
    } else if (deviceType == ID3D_Device_Type::D3D12) {
        if (!initContext) ImGui::CreateContext();
        DXGI_SWAP_CHAIN_DESC sdesc;
        ppSwapChain->GetDesc(&sdesc);
        sdesc.Flags        = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sdesc.OutputWindow = window;
        sdesc.Windowed = ((GetWindowLongPtr(window, GWL_STYLE) & WS_POPUP) != 0) ? false : true;
        buffersCounts  = sdesc.BufferCount;
        frameContext   = new FrameContext[buffersCounts];
        D3D12_DESCRIPTOR_HEAP_DESC descriptorImGuiRender = {};
        descriptorImGuiRender.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        descriptorImGuiRender.NumDescriptors = buffersCounts;
        descriptorImGuiRender.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (d3d12DescriptorHeapImGuiRender == nullptr)
            if (FAILED(d3d12Device->CreateDescriptorHeap(
                    &descriptorImGuiRender, IID_PPV_ARGS(&d3d12DescriptorHeapImGuiRender)
                )))
                goto out;
        if (d3d12Device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)
            ) != S_OK)
            return false;
        for (size_t i = 0; i < buffersCounts; i++) {
            frameContext[i].commandAllocator = allocator;
        };
        if (d3d12Device->CreateCommandList(
                0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, NULL,
                IID_PPV_ARGS(&d3d12CommandList)
            ) != S_OK ||
            d3d12CommandList->Close() != S_OK)
            return false;
        D3D12_DESCRIPTOR_HEAP_DESC descriptorBackBuffers;
        descriptorBackBuffers.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        descriptorBackBuffers.NumDescriptors = buffersCounts;
        descriptorBackBuffers.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        descriptorBackBuffers.NodeMask       = 1;
        if (d3d12Device->CreateDescriptorHeap(
                &descriptorBackBuffers, IID_PPV_ARGS(&d3d12DescriptorHeapBackBuffers)
            ) != S_OK)
            return false;
        const auto rtvDescriptorSize =
            d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        rtvHandle = d3d12DescriptorHeapBackBuffers->GetCPUDescriptorHandleForHeapStart();
        for (size_t i = 0; i < buffersCounts; i++) {
            ID3D12Resource* pBackBuffer                   = nullptr;
            frameContext[i].main_render_target_descriptor = rtvHandle;
            ppSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
            d3d12Device->CreateRenderTargetView(pBackBuffer, nullptr, rtvHandle);
            frameContext[i].main_render_target_resource  = pBackBuffer;
            rtvHandle.ptr                               += rtvDescriptorSize;
            pBackBuffer->Release();
        };
        POINT mouse;
        RECT  rc = {0};
        if (d3d12CommandQueue == nullptr) goto out;
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        FrameContext& currentFrameContext =
            frameContext[ppSwapChain->GetCurrentBackBufferIndex()];
        currentFrameContext.commandAllocator->Reset();
        D3D12_RESOURCE_BARRIER barrier;
        barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource   = currentFrameContext.main_render_target_resource;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
        d3d12CommandList->Reset(currentFrameContext.commandAllocator, nullptr);
        d3d12CommandList->ResourceBarrier(1, &barrier);
        d3d12CommandList->OMSetRenderTargets(
            1, &currentFrameContext.main_render_target_descriptor, FALSE, nullptr
        );
        d3d12CommandList->SetDescriptorHeaps(1, &d3d12DescriptorHeapImGuiRender);
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), d3d12CommandList);
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
        d3d12CommandList->ResourceBarrier(1, &barrier);
        d3d12CommandList->Close();
        d3d12CommandQueue->ExecuteCommandLists(
            1, reinterpret_cast<ID3D12CommandList* const*>(&d3d12CommandList)
        );
        d3d12DescriptorHeapBackBuffers->Release();
        d3d12CommandList->Release();
        allocator->Release();
        currentFrameContext.main_render_target_resource->Release();
        currentFrameContext.commandAllocator->Release();
        d3d12Device->Release();
        delete frameContext;
    };
    goto out;
out:
    return oPresentD3D12(ppSwapChain, syncInterval, flags);
};