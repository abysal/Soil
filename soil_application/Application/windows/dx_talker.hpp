#pragma once
#include "./communication_manager.hpp"
#include "injector.hpp"
#include "tlhelp32.h"
#include <Application/editor_config.hpp>
#include <ipc.hpp>

namespace soil {

    class DxTalkerBase {
    protected:
        void open_module_handle(const std::string& module_name);
        void preload_module();

    public:
        uintptr_t external_process_locate_func(const std::string& mod_name);

    protected:
        DxTalkerBase()                    = default;
        DxTalkerBase(const DxTalkerBase&) = delete;
        DxTalkerBase(DxTalkerBase&&)      = default;

    protected:
        Win32Handle   mc_ipc_handle{};
        Win32Handle   mc_handle{};
        MODULEENTRY32 target_module{0};
        HMODULE       loaded_module_handle{};
    };

    class Dx12Talker : public DxTalkerBase {
    public:
        Dx12Talker() = default;

        void attach(Injector& injector);
        void init_injected_dll();

        volatile dx_ipc::Dx12Ipc& get_ipc() { return this->ipc.get_struct(); }

        bool ipc_created() const noexcept { return this->ipc.created(); }

        void visualise_state(const EditorVisualConfig& cfg) noexcept;

    private:
        Dx12Talker(const Dx12Talker&) = delete;
        Dx12Talker(Dx12Talker&&)      = default;

    private:
        IPCManager<dx_ipc::Dx12Ipc> ipc{};
    };
} // namespace soil