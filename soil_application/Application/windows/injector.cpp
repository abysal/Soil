#include "./injector.hpp"
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <print>
#include <stdexcept>
#include <string_view>
#include <thread>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <aclapi.h>
#include <handleapi.h>
#include <libloaderapi.h>
#include <memoryapi.h>
#include <minwinbase.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <psapi.h>
#include <sddl.h>
#include <shellapi.h>
#include <stdio.h>
#include <synchapi.h>
#include <tchar.h>
#include <winbase.h>
#include <winnt.h>
#include <winuser.h>


namespace fs = std::filesystem;

using namespace std::chrono_literals;

namespace soil {

    Injector::~Injector() { CloseHandle(this->mc_handle); }

    void Injector::attach() {
        if (!fs::exists(this->dll_path)) {
            throw std::runtime_error("Attempted to inject a missing DLL (Did Akashic forget to "
                                     "change the path from a hardcoded one?)");
        }
        this->update_security();
        const auto target_mc_pid = this->get_mc();

        const auto mc_handle = OpenProcess(PROCESS_ALL_ACCESS, false, target_mc_pid);

        if (!mc_handle) {
            throw std::runtime_error("Failed to open a handle into Minecraft");
        }

        const auto allocation_address = VirtualAllocEx(
            mc_handle, nullptr, this->dll_path.native().length(), MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE
        );

        if (!allocation_address) {
            throw std::runtime_error("Failed to allocate memory in Minecraft!");
        }

        const auto path = this->dll_path.string();

        usize wrote_bytes;

        const usize allocation_size = path.length() + 1;

        if (!WriteProcessMemory(
                mc_handle, allocation_address, path.c_str(), allocation_size, &wrote_bytes
            )) {
            throw std::runtime_error("Failed to write path");
        }

        const auto lla_address =
            GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

        if (!lla_address) {
            throw std::runtime_error("Failed to locate LLA");
        }

        DWORD thread_id;

        auto handle = CreateRemoteThread(
            mc_handle, nullptr, 0, (LPTHREAD_START_ROUTINE)lla_address,
            (void*)allocation_address, 0, &thread_id
        );

        if (!handle) {
            std::runtime_error("Faield to invoke load library!");
        }

        WaitForSingleObject(handle, INFINITE);
        CloseHandle(handle);
        VirtualFreeEx(mc_handle, (void*)allocation_address, allocation_size, MEM_RELEASE);

        this->mc_handle = mc_handle;
    }

    u32 Injector::get_mc() {

        auto targets = this->iter_proc_list("Minecraft.Windows.exe");

        if (targets.size() != 1) {

            if (!targets.empty()) {
                throw std::runtime_error("Not Implemented");
            }

            constexpr usize MAX_RETRY_COUNT = 50;
            usize           execution_count = 0;

            if ((uintptr_t)ShellExecuteA(
                    nullptr, "open", "powershell", "-Command \"Start-Process minecraft://\"",
                    nullptr, IS_DEBUG
                ) < 32) {
                throw std::runtime_error("Failed to start Minecraft!");
            }

            while (targets.empty() && execution_count++ < MAX_RETRY_COUNT) {
                targets = this->iter_proc_list("Minecraft.Windows.exe");
                std::this_thread::sleep_for(200ms);
            }

            if (targets.empty()) {
                throw std::runtime_error("Minecraft took too long to start");
            }
        }

        auto handle = this->get_process_module(targets[0], "SoilDx12.dll");

        if (handle) {
            TerminateProcess(OpenProcess(PROCESS_TERMINATE, false, targets[0]), 0);
            return get_mc();
        }

        return targets[0];
    }

    usize Injector::get_process_module(u32 PID, const std::string module_name) {
        DWORD                bytes_needed{};
        DWORD                module_count{};
        std::vector<HMODULE> module = std::vector<HMODULE>(128, 0);

        HANDLE proc_handle = OpenProcess(0xFFFF, FALSE, PID);

        if (!EnumProcessModulesEx(
                proc_handle, module.data(), module.size() * sizeof(HMODULE), &bytes_needed,
                LIST_MODULES_ALL
            )) {
            goto end;
        }

        while (bytes_needed >= module.size() * sizeof(HMODULE)) {
            module.resize(module.size() * 2);

            if (!EnumProcessModulesEx(
                    proc_handle, module.data(), module.size() * sizeof(HMODULE), &bytes_needed,
                    LIST_MODULES_ALL
                )) {
                goto end;
            }
        }

        module_count = bytes_needed / sizeof(HMODULE);

        for (usize idx = 0; idx < module_count; idx++) {
            auto name_arr = std::array<char, 2048>();

            auto name_len = GetModuleFileNameA(module[idx], name_arr.data(), name_arr.size());

            if (!name_len) {
                continue;
            }

            const auto name = std::string_view{name_arr.data(), name_len};

            if (name.ends_with(module_name)) {
                return (usize)module[idx];
            }
        }

    end:
        CloseHandle(proc_handle);

        return 0;
    }

    std::vector<u32> Injector::iter_proc_list(const std::string_view process_name) {
        DWORD              process_count{};
        std::vector<DWORD> processes = std::vector<DWORD>(1024, 0);

        auto proc_size = processes.size() * sizeof(DWORD);

        if (!EnumProcesses(processes.data(), proc_size, &process_count)) {
            throw std::runtime_error("Failed to list all processes");
        }

        while (process_count >= proc_size) {
            processes.reserve(proc_size * 2);
            auto proc_size = processes.size() * sizeof(DWORD);

            if (!EnumProcesses(processes.data(), proc_size, &process_count)) {
                throw std::runtime_error("Failed to list all processes");
            }
        }

        process_count /= sizeof(DWORD);

        std::vector<u32> pids{};

        for (usize idx = 0; idx < process_count; idx++) {
            if (processes[idx] == 0) {
                continue;
            }

            const auto id = processes[idx];

            HANDLE process_handle =
                OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, id);

            if (process_handle == 0) {
                continue;
            }

            auto name_arr = std::array<char, 2048>();

            usize length =
                GetProcessImageFileNameA(process_handle, name_arr.data(), name_arr.size());

            if (!length) {
                continue;
            }

            CloseHandle(process_handle);

            const auto name = std::string_view{name_arr.data(), length};

            if (!name.ends_with(process_name)) {
                continue;
            }

            pids.push_back(id);
        }

        return pids;
    }

    void Injector::update_security() {
        PSECURITY_DESCRIPTOR pSD      = NULL;
        PACL                 pOldDACL = NULL;
        PACL                 pNewDACL = NULL;
        EXPLICIT_ACCESS      ea;
        DWORD                dwRes;

        dwRes = GetNamedSecurityInfoW(
            this->dll_path.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL,
            &pOldDACL, NULL, &pSD
        );
        if (dwRes != ERROR_SUCCESS) {
            throw std::runtime_error("Failed to get security info!");
            return;
        }

        ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
        ea.grfAccessPermissions = GENERIC_ALL;    // Full Control
        ea.grfAccessMode        = GRANT_ACCESS;   // Allow access
        ea.grfInheritance       = NO_INHERITANCE; // No inheritance
        ea.Trustee.TrusteeForm  = TRUSTEE_IS_SID;

        PSID pSID = NULL;
        if (!ConvertStringSidToSidA("S-1-15-2-1", &pSID)) {
            throw std::runtime_error("Failed to convert SID!");
            return;
        }

        ea.Trustee.ptstrName = (LPSTR)pSID;

        dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
        if (dwRes != ERROR_SUCCESS) {
            throw std::runtime_error("Failed to set entries in ACL!");
            return;
        }

        dwRes = SetNamedSecurityInfoW(
            (wchar_t*)this->dll_path.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL,
            NULL, pNewDACL, NULL
        );
        if (dwRes != ERROR_SUCCESS) {
            throw std::runtime_error("Failed to change DACL");
        }

        // Clean up
        if (pSD) LocalFree(pSD);
        if (pNewDACL) LocalFree(pNewDACL);
        if (pSID) LocalFree(pSID);
    }
} // namespace soil