#pragma once
#include <RmlUi/Core/Context.h>

namespace soil {
    struct RmlBinder {
        static void bind_simple(Rml::DataModelConstructor& context, class Application& owner);
    };

} // namespace soil