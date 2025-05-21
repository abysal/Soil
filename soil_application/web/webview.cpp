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
    WebView::WebView(
        const HWND dx_11_hwnd, winrt::com_ptr<IDCompositionVisual> root_visual,
        winrt::com_ptr<IDCompositionDevice> composition_device
    ) {
        this->hwnd               = dx_11_hwnd;
        this->root_visual        = root_visual;
        this->composition_device = composition_device;

        throw_on_fail(CreateCoreWebView2EnvironmentWithOptions(
            nullptr, nullptr, nullptr,
            Microsoft::WRL::Callback<
                ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                [&](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                    throw_on_fail(result);

                    winrt::com_ptr<ICoreWebView2Environment3> env3;
                    throw_on_fail(env->QueryInterface(IID_PPV_ARGS(env3.put())));

                    return env3->CreateCoreWebView2CompositionController(
                        this->hwnd,
                        Microsoft::WRL::Callback<
                            ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler>(
                            [&](HRESULT                             result,
                                ICoreWebView2CompositionController* controller) -> HRESULT {
                                throw_on_fail(result);
                                this->init_web_view(controller);
                                return S_OK;
                            }
                        ).Get()
                    );
                }
            ).Get()
        ));
    }

    void WebView::update() const {

        if (this->composition_controller) {
            RECT bounds;
            if (!GetClientRect(this->hwnd, &bounds)) return;

            throw_on_fail(this->controller->put_Bounds(bounds));
        }
    }

    void WebView::init_web_view(ICoreWebView2CompositionController* controller) {
        if (!controller) {
            return;
        }

        controller->AddRef();
        this->composition_controller.attach(controller);
        (void)this->composition_controller->QueryInterface(IID_PPV_ARGS(this->controller.put())
        );
        throw_on_fail(this->controller->get_CoreWebView2(this->view.put()));

        throw_on_fail(this->composition_controller->put_RootVisualTarget(this->root_visual.get()
        ));

        throw_on_fail(this->composition_device->Commit());

        RECT bounds;
        throw_on_fail(GetClientRect(this->hwnd, &bounds));
        throw_on_fail(this->controller->put_Bounds(bounds));
        throw_on_fail(this->view->Navigate(L"https://www.google.com"));
    }
} // namespace soil