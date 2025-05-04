#include <clay/clay_binding.hpp> // Breaks if moved

#include "./dx_talker.hpp"
#include "injector.hpp"

#include "tlhelp32.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <format>
#include <libloaderapi.h>
#include <minwinbase.h>
#include <print>
#include <processthreadsapi.h>
#include <psapi.h>
#include <stdexcept>

namespace soil {
    uintptr_t DxTalkerBase::external_process_locate_func(const std::string& mod_name) {
        assert(this->loaded_module_handle);
        const auto address_in_self =
            (uintptr_t)GetProcAddress(this->loaded_module_handle, mod_name.c_str());

        if (!address_in_self) {
            throw std::runtime_error("Failed to locate function");
        }

        const auto loaded_module_base     = (uintptr_t)this->loaded_module_handle;
        const auto other_application_base = (uintptr_t)this->target_module.modBaseAddr;

        const auto raw_offset = address_in_self - loaded_module_base;

        return raw_offset + other_application_base;
    }

    void DxTalkerBase::open_module_handle(const std::string& module) {
        assert(this->mc_handle);

        const auto pid = GetProcessId(this->mc_handle);

        const auto handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);

        if (INVALID_HANDLE_VALUE == handle) {
            throw std::runtime_error("Failed to snapshot the process");
        }

        MODULEENTRY32 me;
        memset((void*)&me, 0, sizeof(MODULEENTRY32));
        me.dwSize = sizeof(MODULEENTRY32);

        bool ran = false;

        for (bool cont = Module32First(handle, &me); cont; cont = Module32Next(handle, &me)) {
            ran = true;

            if (!strcmp(me.szModule, module.c_str())) {
                this->target_module = me;
            }
        }

        if (!ran) {
            throw std::runtime_error("Failed to iter modules");
        }

        if (this->target_module.dwSize != sizeof(MODULEENTRY32)) {
            throw std::runtime_error("Failed to find module");
        }
    }

    void DxTalkerBase::preload_module() {
        assert(this->target_module.dwSize == sizeof(MODULEENTRY32));

        const auto local_dll_handle = LoadLibrary(this->target_module.szExePath);

        if (!local_dll_handle) {
            throw std::runtime_error("Failed to load dll into self");
        }

        this->loaded_module_handle = local_dll_handle;
    }

    void Dx12Talker::attach(Injector& injector) {
        this->mc_ipc_handle = this->ipc.instance_ipc(injector);
        this->mc_handle     = injector.valid_handle();

        this->open_module_handle("SoilDx12.dll");
        this->preload_module();
    }

    void Dx12Talker::init_injected_dll() {

        auto& ipc = this->get_ipc();

        ipc.soil_handle = (uint64_t)GetCurrentProcess();

        const auto init_func = this->external_process_locate_func("init");
        DWORD      thread_id;
        if (!CreateRemoteThread(
                this->mc_handle, nullptr, 0, (LPTHREAD_START_ROUTINE)init_func,
                (void*)this->mc_ipc_handle, 0, &thread_id
            )) {
            throw std::runtime_error("Failed to init dll");
        }
    }

    void Dx12Talker::visualise_state(const EditorVisualConfig& cfg) noexcept {
        using namespace clay_extension;

        if (!this->ipc_created()) {
            return;
        }

        new_element(
            {
                .layout.layoutDirection = CLAY_TOP_TO_BOTTOM,
                .backgroundColor        = cfg.sidebar_background_color,
            },
            [&] {
                const auto& state = this->get_ipc();
                std::string render_string;

                const auto render = [&] {
                    new_element({}, [&] {
                        Clay__OpenTextElement(
                            to_clay_last(render_string),
                            text_config({
                                            .textColor = cfg.normal_text_color,
                                            .fontSize  = 15,
                                        })
                                .get()
                        );
                    });
                };

                render_string = std::format("has_v3: {}", state.supports_v3_swapchain.load());
                render();

                render_string = std::format("magic: {:x}", state.init_magic.load());
                render();

                if (state.last_error.load() != 0) {
                    render_string =
                        std::format("last error: {}", (const char*)&state.string_buffer);
                    render();
                }

                render_string =
                    std::format("back buffer ptr: {:x}", state.back_buffer_pointer.load());
                render();

                render_string = "Texture Info:";
                render();

                const auto texture_info = state.render_texture_info.load();

                render_string = std::format("handle: {:x}", texture_info.render_handle);
                render();

                render_string = std::format("width: {}", texture_info.width);
                render();

                render_string = std::format("height: {}", texture_info.height);
                render();

                render_string = std::format("total size: {}", texture_info.total_size);
                render();

                render_string = std::format("format: {:x}", texture_info.format);
                render();
            }
        );
    }
} // namespace soil