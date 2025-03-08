
#pragma once
#include "../clay_binding.hpp"
#include "../templates.hpp"
#include <clay.h>
#include <concepts>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace clay_extension {

    template <typename T>
    concept HasExpandMember = requires {
        { T::allow_multiple } -> std::same_as<bool>;
    };

    struct RenderInformation {
        bool is_hovered;
    };

    template <typename T, typename ConfigType>
    concept HeaderBarButton =
        requires(T a, ConfigType &x, const RenderInformation &y) {
            { a.id() } -> std::same_as<std::string>;
            { a.render(x, y) } -> std::same_as<void>;
        };

    COMPLEX_OPTIONAL_ARGS_FUNCTION(custom_declaration, CustomDeclaration, std::optional<ClayElementDeclarationPartial>, std::nullopt, const RenderInformation &)

    // Dark magic below!
    // This basically a wrapper over a tuple with a little more magic to support
    // multiple of the same type being added at runtime
    template <typename ConfigType, HeaderBarButton<ConfigType>... ButtonTypes>
    class HeaderBar {
    private:
        template <typename T, bool expand>
        using ConditionalMore = std::conditional_t<expand, std::vector<T>, T>;

        template <typename T> struct ExpandedType {
            using type = ConditionalMore<T, false>;
        };

        template <HasExpandMember T> struct ExpandedType<T> {
            using type = ConditionalMore<T, T::allow_multiple>;
        };

        template <typename T> struct More {
            static constexpr bool allows_more = false;
        };

        template <HasExpandMember T> struct More<T> {
            static constexpr bool allows_more = T::allow_multiple;
        };

        template <HeaderBarButton<ConfigType> Button> struct ButtonElement {
            using ButtonType = Button;
            Button         store;
            Clay_ElementId id;
            std::string    id_string;

            ButtonElement(Button &&button) noexcept : store(std::move(button)) {

                this->id_string = this->store.id();

                this->id = hash_string(this->id_string);
            }
        };

        template <typename T>
        using SingleButtonType = ButtonElement<typename ExpandedType<T>::type>;

        using StoreType = std::tuple<SingleButtonType<ButtonTypes>...>;

    public:
        struct ElementConfig {
            soil::Color background_color;
            f32         hover_light_amount = 0.0;
            f32         rounding           = 10;
            soil::Vec2i min_size           = {0, 0};
        };

    public:
        HeaderBar(
            Clay_ElementDeclaration element_data, ElementConfig button_config,
            typename ExpandedType<ButtonTypes>::type &&...types
        ) noexcept
            : button_store(
                  std::make_tuple(std::move(ButtonElement{std::move(types)})...)
              ),
              button_config(button_config), host_config(element_data) {}

        void render(ConfigType &color_config) noexcept {
            new_element(this->host_config, [&] {
                this->for_each_element([&](auto &element) {
                    Clay_ElementDeclaration decl =
                        this->build_full_decl(element);

                    RenderInformation render = {Clay_PointerOver(decl.id)};

                    new_element(decl, [&] {
                        element.store.render(color_config, render);
                    });
                });
            });
        }

        template <HeaderBarButton<ConfigType> T>
            requires std::same_as<
                BoolToConstant<More<T>::allows_more>, std::true_type>
        void add_element(T &&element) noexcept {
            ButtonElement<std::vector<T>> &elements =
                std::get<SingleButtonType<T>>(this->button_store);

            elements.store.emplace_back(std::move(element));
        }

    private:
        template <HeaderBarButton<ConfigType> T>
        Clay_ElementDeclaration build_full_decl(ButtonElement<T> &t) noexcept {
            Clay_ElementDeclaration decl = {
                .id              = t.id,
                .backgroundColor = this->button_config.background_color,
                .cornerRadius =
                    {this->button_config.rounding, this->button_config.rounding,
                     this->button_config.rounding, this->button_config.rounding}
            };

            RenderInformation render = {
                .is_hovered = Clay_PointerOver(decl.id)
            };

            std::optional<ClayElementDeclarationPartial> base =
                CustomDeclarationCaller<T>::custom_declaration(t.store, render);

            return base
                .and_then(
                    [&](ClayElementDeclarationPartial &p
                    ) -> std::optional<Clay_ElementDeclaration> {
                        decl.layout          = p.layout;
                        decl.backgroundColor = p.backgroundColor;
                        decl.cornerRadius    = p.cornerRadius;
                        decl.image           = p.image;
                        decl.floating        = p.floating;
                        decl.custom          = p.custom;
                        decl.scroll          = p.scroll;
                        decl.border          = p.border;
                        decl.userData        = p.userData;

                        return decl;
                    }
                )
                .value_or(decl);
        }

        template <typename T> void for_each_element(T &&callback) noexcept {
            this->for_each_element_disptacher(
                tuple_list(this->button_store), callback
            );
        }

        template <typename T, std::size_t... Indices>
        void for_each_element_disptacher(
            std::index_sequence<Indices...>, T &func
        ) noexcept {
            (this->for_each_element_impl<T, Indices>(func), ...);
        }

        template <typename T, size_t index>
        void for_each_element_impl(T &func) noexcept {
            auto &element = std::get<index>(this->button_store);

            this->for_each_element_call(element, func);
        }

        template <typename T, typename ButtonType>
            requires std::same_as<
                std::true_type, is_vector<typename ButtonType::ButtonType>>
        void for_each_element_call(ButtonType &elements, T &func) noexcept {
            for (ButtonType &element : elements.store) {
                this->for_each_element_call(element, func);
            }
        }

        template <typename T, typename ButtonType>
        void for_each_element_call(ButtonType &element, T &func) noexcept {
            func(element);
        }

    private:
        StoreType               button_store;
        Clay_ElementDeclaration host_config;
        ElementConfig           button_config;
    };
} // namespace clay_extension