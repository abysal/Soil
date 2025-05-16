#pragma once
#include <winrt/base.h>
#include <windows.h>


#include <combaseapi.h>     // CoInitializeEx, etc.
#include <Unknwn.h>         // IUnknown, required by COM interfaces
#include <objbase.h>        // CoCreateInstance
#include <ole2.h>           // COM essentials
#include <WebView2.h>       // WebView2 interfaces

namespace soil {
    class WebView {
    public:
        WebView();

    private:
        winrt::com_ptr<ICoreWebView2Controller> controller = {};
        winrt::com_ptr<ICoreWebView2>           view       = {};
    };
} // namespace soil