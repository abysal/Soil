#pragma once

#include <format>
#include <stdexcept>

#include <guiddef.h>

#include <strsafe.h>
#include <winrt/base.h>

namespace soil {

    class HResultError : std::runtime_error {
    public:
        HResultError(const winrt::hresult result)
            : std::runtime_error(hr_as_string(result)), result(result) {}

        static std::string hr_as_string(winrt::hresult result) {
            return std::format("0x{:x}", result.value);
        }

        winrt::hresult value() const noexcept { return this->result; }

    private:
        winrt::hresult result = {};
    };

    static void throw_on_fail(const winrt::hresult result) {
        if (!SUCCEEDED(result.value)) {
            throw HResultError(result);
        }
    }
} // namespace soil