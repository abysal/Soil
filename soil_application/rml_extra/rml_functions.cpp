#include "./rml_functions.hpp"
#include "../soil.hpp"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataTypes.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Variant.h>
#include <numeric>
#include <ranges>
#include <stdexcept>
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

        // This capture is safe, since application never changes location
        rml_funcs.push_back(RML_FUNC_C(settings_prop, {
            if (arguments.size() == 0) {
                return {};
            }

            try {
                std::string out;
                if (!owner.settings.get_member(arguments[0].Get<std::string>()).GetInto(out)) {
                    throw std::logic_error("How?");
                }

                auto string_value = Rml::Variant(out);

                if (arguments.size() == 1) {
                    return string_value;
                }

                auto new_args = arguments;

                new_args[0] = std::move(out);

                return rml_funcs[0].second(new_args);
            } catch (std::invalid_argument error) {
                Rml::Log::Message(
                    Rml::Log::LT_ERROR, "Failed to pull setting. With error: %s", error.what()
                );

                return {};
            }
        }));

        for (size_t i = 0; i < rml_funcs.size(); i++) {
            auto [name, func] = std::move(rml_funcs[i]);

            context.RegisterTransformFunc(name, func);
        }
    }
} // namespace soil