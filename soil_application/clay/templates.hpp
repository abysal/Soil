//
// Created by Akashi on 02/03/25.
//

#ifndef TEMPLATES_HPP
#define TEMPLATES_HPP
#include <array>
#include <clay.h>
#include <format>
#include <print>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

template <> struct std::formatter<Clay_String> {
    constexpr auto parse(std::format_parse_context &c) { return c.begin(); }

    auto format(const Clay_String &text, std::format_context &ctx) const {
        return std::format_to(
            ctx.out(), "{}", std::string_view(text.chars, text.length)
        );
    }
};

template <std::integral T> void log_integral_as_chars(T val) noexcept {
    std::array<char, sizeof(T)> v =
        std::bit_cast<std::array<char, sizeof(T)>>((val));

    std::string_view s{v.data(), v.size()};

    std::println("{}", s);
}

template <typename... T>
std::index_sequence_for<T...> tuple_list(const std::tuple<T...> &t) noexcept {
    return std::index_sequence_for<T...>{};
}

template <typename T> struct is_vector : std::false_type {};

template <typename T> struct is_vector<std::vector<T>> : std::true_type {};

template <bool b>
using BoolToConstant =
    typename std::conditional<b, std::true_type, std::false_type>;

#define SIMPLE_OPTIONAL_FUNCTION(FUNC_NAME, STRUCT_NAME)                       \
                                                                               \
    template <typename T>                                                      \
    concept STRUCT_NAME = requires(T t) {                                      \
        { t.FUNC_NAME() } -> std::same_as<void>;                               \
    };                                                                         \
                                                                               \
    template <typename T> struct Has##STRUCT_NAME : std::false_type {};        \
                                                                               \
    template <STRUCT_NAME T> struct Has##STRUCT_NAME<T> : std::true_type {};   \
                                                                               \
    template <typename T> struct STRUCT_NAME##Caller {                         \
        static void FUNC_NAME(T &) noexcept {};                                \
    };                                                                         \
                                                                               \
    template <STRUCT_NAME T> struct STRUCT_NAME##Caller<T> {                   \
        static void FUNC_NAME(T &element) noexcept { element.FUNC_NAME(); };   \
    };

#define COMPLEX_OPTIONAL_FUNCTION(FUNC_NAME, STRUCT_NAME, RETURN_TYPE, ...)    \
                                                                               \
    template <typename T>                                                      \
    concept STRUCT_NAME = requires(T t) {                                      \
        { t.FUNC_NAME() } -> std::same_as<RETURN_TYPE>;                        \
    };                                                                         \
                                                                               \
    template <typename T> struct Has##STRUCT_NAME : std::false_type {};        \
                                                                               \
    template <STRUCT_NAME T> struct Has##STRUCT_NAME<T> : std::true_type {};   \
                                                                               \
    template <typename T> struct STRUCT_NAME##Caller {                         \
        static RETURN_TYPE FUNC_NAME(T &) noexcept { return __VA_ARGS__; };    \
    };                                                                         \
                                                                               \
    template <STRUCT_NAME T> struct STRUCT_NAME##Caller<T> {                   \
        static RETURN_TYPE FUNC_NAME(T &element) noexcept {                    \
            return element.FUNC_NAME();                                        \
        };                                                                     \
    };

#define COMPLEX_OPTIONAL_ARGS_FUNCTION(                                        \
    FUNC_NAME, STRUCT_NAME, RETURN_TYPE, DEFAULT_RETURN, ARG_TYPE              \
)                                                                              \
                                                                               \
    template <typename T>                                                      \
    concept STRUCT_NAME = requires(T t, ARG_TYPE a) {                          \
        { t.FUNC_NAME(a) } -> std::same_as<RETURN_TYPE>;                       \
    };                                                                         \
                                                                               \
    template <typename T> struct Has##STRUCT_NAME : std::false_type {};        \
                                                                               \
    template <STRUCT_NAME T> struct Has##STRUCT_NAME<T> : std::true_type {};   \
                                                                               \
    template <typename T> struct STRUCT_NAME##Caller {                         \
        static RETURN_TYPE FUNC_NAME(T &, ARG_TYPE arg) noexcept {             \
            return DEFAULT_RETURN;                                             \
        };                                                                     \
    };                                                                         \
                                                                               \
    template <STRUCT_NAME T> struct STRUCT_NAME##Caller<T> {                   \
        static RETURN_TYPE FUNC_NAME(T &element, ARG_TYPE arg) noexcept {      \
            return element.FUNC_NAME(arg);                                     \
        };                                                                     \
    };

inline void hash_combine(std::size_t &seed) {}

template <typename T, typename... Rest>
inline void hash_combine(std::size_t &seed, const T &v, Rest... rest) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    hash_combine(seed, rest...);
}
#endif // TEMPLATES_HPP
