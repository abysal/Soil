#pragma once
#include "rml_ui_backend/RmlUi_Platform_GLFW.h"

#include <windows.h>
#include <winrt/base.h>

#include <Unknwn.h>     // IUnknown, required by COM interfaces
#include <WebView2.h>   // WebView2 interfaces
#include <combaseapi.h> // CoInitializeEx, etc.
#include <dcomp.h>
#include <objbase.h> // CoCreateInstance
#include <ole2.h>    // COM essentials

namespace soil {
    class WebView {
    public:
        WebView(
            HWND dx_11_hwnd, winrt::com_ptr<IDCompositionVisual> root_visual,
            winrt::com_ptr<IDCompositionDevice> composition_device
        );

        void update() const;

    private:
        void init_web_view(ICoreWebView2CompositionController* controller);

    private:
        winrt::com_ptr<ICoreWebView2CompositionController> composition_controller = {};
        winrt::com_ptr<ICoreWebView2Controller>            controller             = {};
        winrt::com_ptr<ICoreWebView2>                      view                   = {};
        winrt::com_ptr<IDCompositionVisual>                root_visual            = {};
        winrt::com_ptr<IDCompositionDevice>                composition_device     = {};
        HWND                                               hwnd                   = nullptr;
    };
} // namespace soil