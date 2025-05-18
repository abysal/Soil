#pragma once
#include "rml_ui_backend/RmlUi_Platform_GLFW.h"

#include <windows.h>
#include <winrt/base.h>

#include <Unknwn.h>     // IUnknown, required by COM interfaces
#include <WebView2.h>   // WebView2 interfaces
#include <combaseapi.h> // CoInitializeEx, etc.
#include <objbase.h>    // CoCreateInstance
#include <ole2.h>       // COM essentials

namespace soil {
    class WebView {
    public:
        WebView(HWND dx_11_hwnd);

        void update();

    private:
        void init_web_view(ICoreWebView2Controller* controller);

    private:
        winrt::com_ptr<ICoreWebView2Controller> controller = {};
        winrt::com_ptr<ICoreWebView2>           view       = {};
        HWND                                    hwnd       = nullptr;
        GLFWwindow*                             window;
    };
} // namespace soil