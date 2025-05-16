#include "webview.hpp"
#include "../rml_ui_backend/RmlUi_Backend.h"
#include <GLFW/glfw3.h>
#include <winrt/base.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


namespace soil {
    WebView::WebView() {
        assert(Backend::get_window());

        // const auto win_handle = glfwGetWin32Window(Backend::get_window());
    }

} // namespace soil