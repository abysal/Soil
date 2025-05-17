#pragma once
#include <RmlUi/Core/Context.h>

namespace soil {
    struct RmlBinder {
        void bind_simple(Rml::DataModelConstructor& context, class Application& owner);
    };

} // namespace soil