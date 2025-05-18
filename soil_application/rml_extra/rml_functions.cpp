#include "./rml_functions.hpp"
#include "../soil.hpp"
#include <RmlUi/Core/DataTypes.h>
#include <RmlUi/Core/Variant.h>
#include <numeric>
#include <ranges>
#include <utility>

#define RML_FUNC(name, ...)                                                                    \
    {#name, [](const Rml::VariantList& arguments) -> Rml::Variant __VA_ARGS__}

#define RML_FUNC_C(name, ...)                                                                  \
    {#name, [&](const Rml::VariantList& arguments) -> Rml::Variant __VA_ARGS__}

#define CASE_CONV(typename, access_name)                                                       \
    case other.access_name: {                                                                  \
        return Rml::ToString(other.GetReference<typename>(), "");                              \
    }

namespace soil {

    static std::vector<std::pair<std::string, Rml::DataTransformFunc>> rml_funcs =
        {RML_FUNC(str_cat, {
            if (arguments.empty()) {
                return {};
            }

            const auto arg_count = arguments.size();

            if (arg_count == 1) {
                return arguments[0];
            }

            auto view =
                arguments | std::views::transform([](const Rml::Variant& other) -> std::string {
                    std::string out;
                    if (!other.GetInto(out)) {
                        return "";
                    } else {
                        return out;
                    }
                });

            auto output = std::accumulate(view.begin(), view.end(), std::string{});

            return Rml::Variant(std::move(output));
        })};

    void RmlBinder::bind_simple(Rml::DataModelConstructor& context, class Application& owner) {
        for (auto& rml_func : rml_funcs) {
            auto [name, func] = std::move(rml_func);

            context.RegisterTransformFunc(name, func);
        }
    }
} // namespace soil