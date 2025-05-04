#pragma once
#include "../../types.hpp"
#include "../cursor.hpp"
#include <algorithm>
#include <bit>
#include <optional>
#include <print>
#include <span>
#include <stdexcept>
#include <vector>

namespace soil {

    class TTF {
    public:
        struct TableEntry {
            u32 tag{};
            u32 offset{};
            u32 length{};
        };

    private:
        template <typename T> struct DeserContext {
            BinaryCursor<T, std::endian::big> cursor;
            const std::vector<TableEntry>&    tables;

            constexpr std::optional<usize> jump_to_table_raw(u32 id) noexcept {
                const auto it = std::find_if(
                    this->tables.begin(), this->tables.end(),
                    [&](const TableEntry& table) { return table.tag == id; }
                );

                if (it == this->tables.end()) {
                    return std::nullopt;
                }

                const TableEntry& entry = *it;

                this->cursor.jump(entry.offset);

                return entry.length;
            }
        };

    public:
        template <typename T>
        constexpr TTF(
            const std::vector<TableEntry>& tables, BinaryCursor<T, std::endian::big>&& cursor
        ) {
            DeserContext<T> ctx = {.cursor = cursor, .tables = tables};

            const auto length = ctx.jump_to_table_raw('maxp');

            if (!length.has_value()) {
                throw std::logic_error("Invalid TTF");
            }

            if (ctx.cursor.template get<u32>() != 0x00010000) {
                throw std::logic_error("Missing header");
            }

            this->glyph_count = ctx.cursor.template get<u16>();
        }
        // Parses a TTF file from some memory, assumes its valid data
        // Please call a validation function on this data
        constexpr static TTF from_data(const std::span<u8> data) noexcept {
            BinaryCursor<const std::span<u8>, std::endian::big> cursor{std::move(data)};

            cursor.get<u32>(); // scaler type
            u16 table_number = cursor.get<u16>();
            cursor.get<u16>();
            cursor.get<u16>();
            cursor.get<u16>();

            std::vector<TableEntry> tables{};
            tables.reserve(table_number);

            for (usize i = 0; i < table_number; i++) {
                u32 tag = cursor.get<u32>();
                cursor.get<u32>();
                u32 offset = cursor.get<u32>();
                u32 length = cursor.get<u32>();

                tables.push_back({.tag = tag, .offset = offset, .length = length});
            }

            return TTF{tables, std::move(cursor)};
        }

        [[nodiscard]] u16 glyphs_count() const noexcept { return this->glyph_count; }

    private:
        u16 glyph_count{};
    };
} // namespace soil