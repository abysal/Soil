#include "webview.hpp"
#include "../rml_ui_backend/RmlUi_Backend.h"
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <winrt/base.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "graphics_api/windows.hpp"

#include <GLFW/glfw3native.h>

#include <wrl.h>

namespace soil {
    WebView::WebView(const HWND dx_11_hwnd) {
        this->hwnd = dx_11_hwnd;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        this->window = glfwCreateWindow(800, 600, "Web View", nullptr, nullptr);
        this->hwnd   = glfwGetWin32Window(this->window);
        glfwShowWindow(this->window);

        throw_on_fail(CreateCoreWebView2EnvironmentWithOptions(
            nullptr, nullptr, nullptr,
            Microsoft::WRL::Callback<
                ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                [&](const HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                    throw_on_fail(result);

                    return env->CreateCoreWebView2Controller(
                        this->hwnd,
                        Microsoft::WRL::Callback<
                            ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                            [&](HRESULT, ICoreWebView2Controller* controller) -> HRESULT {
                                this->init_web_view(controller);
                                return S_OK;
                            }
                        ).Get()
                    );
                }
            ).Get()
        ));
    }

    void WebView::update() {

        if (this->controller.get()) {
            RECT bounds;
            GetClientRect(this->hwnd, &bounds);
            this->controller->put_Bounds(bounds);
        }

        // MSG msg = {};
        // while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        //     TranslateMessage(&msg);
        //     DispatchMessage(&msg);
        // }
    }

    void WebView::init_web_view(ICoreWebView2Controller* controller) {
        if (!controller) {
            return;
        }

        controller->AddRef();
        this->controller.attach(controller);
        throw_on_fail(this->controller->get_CoreWebView2(this->view.put()));

        RECT bounds;
        throw_on_fail(GetClientRect(this->hwnd, &bounds));
        throw_on_fail(this->controller->put_Bounds(bounds));
        throw_on_fail(this->view->Navigate(L"https://www.google.com"));
    }
} // namespace soil