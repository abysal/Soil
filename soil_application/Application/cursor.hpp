
#include "../types.hpp"
#include <bit>
#include <climits>
#include <concepts>
#include <print>
#include <type_traits>
#include <vector>

#ifndef CHAR_WIDTH
#define CHAR_WIDTH CHAR_BIT
#endif

namespace soil {

    template <typename T>
    concept VectorLike = requires(T a, usize index) {
        std::same_as<typename T::value_type, u8>;

        { a.size() } -> std::same_as<usize>;
        requires std::is_same_v<
            std::decay_t<decltype(a[index])>, typename T::value_type>;
    };

    template <
        typename StorageBuffer    = std::vector<u8>,
        std::endian DefaultEndian = std::endian::native>
        requires VectorLike<StorageBuffer>
    class BinaryCursor;

    template <typename T, typename X, std::endian Y> struct Deserializer {
        constexpr static T deserialize(BinaryCursor<X, Y> &cursor
        ) noexcept = delete;
    };

    template <typename T, typename X, std::endian Y>
    concept Deserializable = requires(BinaryCursor<X, Y> &cursor) {
        { Deserializer<T, X, Y>::deserialize(cursor) } -> std::same_as<T>;
    };

    template <typename StorageBuffer, std::endian DefaultEndian>
        requires VectorLike<StorageBuffer>
    class BinaryCursor {
    private:
        const StorageBuffer buffer;
        usize               index = 0;

    public:
        constexpr BinaryCursor(StorageBuffer &&buffer, usize starting_index = 0) noexcept
            : buffer(std::move(buffer)), index(starting_index) {};

        template <typename T>
            requires Deserializable<T, StorageBuffer, DefaultEndian>
        constexpr T get() noexcept {
            return Deserializer<T, StorageBuffer, DefaultEndian>::deserialize(
                *this
            );
        }

        constexpr u8 get_byte() noexcept { return this->buffer[index++]; }

        template <typename T>
            requires Deserializable<T, StorageBuffer, DefaultEndian>
        constexpr T peek() const noexcept {
            BinaryCursor *self = const_cast<BinaryCursor *>(this);

            const auto before = self->index;

            const auto return_val =
                self->get<T, StorageBuffer, DefaultEndian>();

            self->index = before;

            return return_val;
        }

        constexpr void inline jump(usize position) noexcept {
            this->index = position;
        }
    };

#define BASIC_TYPE(TYPE)                                                       \
    template <typename S, std::endian E> struct Deserializer<TYPE, S, E> {     \
        constexpr static TYPE deserialize(BinaryCursor<S, E> &cursor           \
        ) noexcept {                                                           \
            TYPE out{};                                                        \
                                                                               \
            for (usize i = 0; i < sizeof(TYPE); i++) {                         \
                out <<= CHAR_WIDTH;                                            \
                out  |= cursor.get_byte();                                     \
            }                                                                  \
                                                                               \
            if constexpr (E == std::endian::native) {                          \
                out = std::byteswap(out);                                      \
            }                                                                  \
                                                                               \
            return out;                                                        \
        }                                                                      \
    };

    BASIC_TYPE(u8);
    BASIC_TYPE(i8);
    BASIC_TYPE(u16);
    BASIC_TYPE(i16);
    BASIC_TYPE(u32);
    BASIC_TYPE(i32);
    BASIC_TYPE(u64);
    BASIC_TYPE(i64);
} // namespace soil