#pragma once
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>

namespace soil {
    struct RmlBinder {
        void bind_simple(Rml::DataModelConstructor& context, class Application& owner);
    };

} // namespace soil