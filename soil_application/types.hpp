//
// Created by Akashi on 02/03/25.
//

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef TYPES_HPP
#define TYPES_HPP
#include <algorithm>
#include <clay.h>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>

using u8  = unsigned char;
using i8  = char;
using u16 = unsigned short;
using i16 = short;
using u32 = unsigned int;
using i32 = int;
using u64 = unsigned long long;
using i64 = long long;

using f32 = float;
using f64 = double;
#ifdef _DEBUG
#define DEBUG_ONLY(...) __VA_ARGS__
constexpr bool IS_DEBUG = true;
#else
constexpr bool IS_DEBUG = false;
#define DEBUG_ONLY(...)
#endif

#if _WIN32 || _WIN64
#if _WIN64
#define PTR64
#else
#define PTR32
#endif
#elif __GNUC__
#if __x86_64__ || __ppc64__
#define PTR64
#else
#define PTR32
#endif
#elif UINTPTR_MAX > UINT_MAX
#define PTR64
#else
#define PTR32
#endif

#ifdef PTR64
using usize = size_t;

#else

using usize = size_t;

#endif

namespace soil {
    template <typename T> struct Vector2Base {
        T x;
        T y;
    };

    template <typename T> struct Vector3Base {
        T x;
        T y;
    };

    template <typename T> struct Vector4Base {
        T x;
        T y;
        T z;
        T w;

        template <typename Self, typename U>
        constexpr Self operator*(this const Self& self, U v) noexcept {
            if constexpr (std::is_floating_point<U>::value) {
                return {
                    static_cast<T>(std::clamp(
                        static_cast<U>(self.x) * v,
                        static_cast<U>(std::numeric_limits<T>::min()),
                        static_cast<U>(std::numeric_limits<T>::max())
                    )),
                    static_cast<T>(std::clamp(
                        static_cast<U>(self.y) * v,
                        static_cast<U>(std::numeric_limits<T>::min()),
                        static_cast<U>(std::numeric_limits<T>::max())
                    )),
                    static_cast<T>(std::clamp(
                        static_cast<U>(self.z) * v,
                        static_cast<U>(std::numeric_limits<T>::min()),
                        static_cast<U>(std::numeric_limits<T>::max())
                    )),
                    static_cast<T>(std::clamp(
                        static_cast<U>(self.w) * v,
                        static_cast<U>(std::numeric_limits<T>::min()),
                        static_cast<U>(std::numeric_limits<T>::max())
                    ))
                };
            } else {
                return {
                    static_cast<T>(std::clamp(
                        self.x * static_cast<T>(v), std::numeric_limits<T>::min(),
                        std::numeric_limits<T>::max()
                    )),
                    static_cast<T>(std::clamp(
                        self.y * static_cast<T>(v), std::numeric_limits<T>::min(),
                        std::numeric_limits<T>::max()
                    )),
                    static_cast<T>(std::clamp(
                        self.z * static_cast<T>(v), std::numeric_limits<T>::min(),
                        std::numeric_limits<T>::max()
                    )),
                    static_cast<T>(std::clamp(
                        self.w * static_cast<T>(v), std::numeric_limits<T>::min(),
                        std::numeric_limits<T>::max()
                    ))
                };
            }
        }

        template <typename Self>
        constexpr bool operator==(this const Self& self, const Self& o) noexcept {
            return self.x == o.x && self.y == o.y && self.z == o.z && self.w == o.w;
        }
    };

    using Vec4i = Vector4Base<i32>;
    using Vec2i = Vector2Base<i32>;
    using Vec2d = Vector2Base<double>;

    // Assumes that this is valid hex
    template <std::integral T> constexpr T from_hex(std::string_view str) noexcept {

        const auto char_conv = [](const char c) {
            return (c >= '0' && c <= '9')   ? (c - '0')
                   : (c >= 'A' && c <= 'F') ? (c - 'A' + 10)
                   : (c >= 'a' && c <= 'f') ? (c - 'a' + 10)
                                            : -1;
        };

        T out{0};

        for (const char c : str) {
            out <<= 4;
            out  |= char_conv(c);
        }

        return out;
    }

    static_assert(from_hex<i32>("FF") == 255, "Hex Conversion Failed");
    static_assert(from_hex<i32>("A1") == 161, "From Hex Failed");

    struct Color : public Vector4Base<u8> {
        // NOLINTNEXTLINE
        operator Clay_Color() const noexcept {
            return {
                static_cast<f32>(this->x), static_cast<f32>(this->y), static_cast<f32>(this->z),
                static_cast<f32>(this->w)
            };
        }

        constexpr Color(std::string_view color) {
            if (color.length() > 9) {
                throw std::invalid_argument("The color passed into the Color hex "
                                            "decoder was not a valid hex code");
            }

            if (color.starts_with('#')) {
                color = color.substr(1);
            }

            const auto section = [&] {
                const u8 out = from_hex<u8>(color.substr(0, 2));

                color = color.substr(2);

                return out;
            };

            this->x = section();
            this->y = section();
            this->z = section();
            this->w = (color.length() == 0) ? 255 : section();
        }

        constexpr Color(u8 red, u8 green, u8 blue, u8 alpha = 255) {
            this->x = red;
            this->y = green;
            this->z = blue;
            this->w = alpha;
        }
    };

    static_assert(Color("FFFFFF") == Color{255, 255, 255, 255}, "Color Conversion Broken!");

    static_assert(Color("#FFFFFFFF") == Color{255, 255, 255, 255}, "Color Conversion Broken!");
    static_assert(Color("FFFFFF00") == Color{255, 255, 255, 0}, "Color Conversion Broken!");
    static_assert(Color("FEFDFBFF") == Color{254, 253, 251, 255}, "Color Conversion Broken!");

    template <typename... StorageTypes> class DynamicVariant {
    public:
        template <typename T> consteval auto add_type() const {
            return DynamicVariant<StorageTypes..., T>();
        }

        constexpr auto create_variant() const { return variant_type(); }

    private:
        using variant_type = std::variant<StorageTypes...>;
    };

} // namespace soil

#endif // TYPES_HPP
