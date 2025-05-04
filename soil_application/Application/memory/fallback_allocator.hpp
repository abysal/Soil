#pragma once
#include "../../types.hpp"
#include "pointer.hpp"
#include <array>
#include <cstdint>
#include <memory>
#include <memory_resource>
#include <print>
#include <span>
#include <stdexcept>

namespace soil {

    template <usize BLOCKSIZE = 8192, usize MAX_PRE_ALLOC_BLOCKS = 2>
    class BlockAllocator : public std::pmr::memory_resource {
    private:
        using BlockMemory = std::array<std::byte, BLOCKSIZE>;
        using BlockType   = std::unique_ptr<BlockMemory>;
        usize                  current_block_index{0};
        usize                  current_index_in_block{0};
        std::vector<BlockType> allocations{};

    private:
        BlockType& current_block() noexcept {
            return this->allocations[this->current_block_index];
        }

        void upgrade_new_block() noexcept {
            this->current_block_index++;
            this->current_index_in_block = 0;

            this->allocations.reserve(this->allocations.size() + 1);
            this->allocations.emplace_back(std::move(std::make_unique<BlockMemory>(BlockMemory{}
            )));
        }

        void* internal_allocate(usize alignement, usize count) noexcept {
            auto& block = this->current_block();

            const usize start_space = BLOCKSIZE - this->current_index_in_block;
            usize       space       = start_space;

            void* out = block->data() + this->current_index_in_block;

            if (!std::align(alignement, count, out, space)) {
                return nullptr;
            }

            if ((uintptr_t)out + count > (uintptr_t)block.get() + BLOCKSIZE) {
                return nullptr;
            }

            uintptr_t end   = (uintptr_t)out;
            uintptr_t start = (uintptr_t)block->data();

            const usize used_space_in_align  = end - start;
            this->current_index_in_block    += used_space_in_align + count;

            return out;
        }

    public:
        BlockAllocator() noexcept {
            this->allocations.emplace_back(std::make_unique<BlockMemory>(BlockMemory{}));
        }

        void clear() {
            this->current_block_index    = 0;
            this->current_index_in_block = 0;

            if (this->allocations.size() > MAX_PRE_ALLOC_BLOCKS) {
                this->allocations.resize(MAX_PRE_ALLOC_BLOCKS);
            }
        }

    protected:
        __declspec(allocator) void* do_allocate(usize count, usize alignement) override {

            if (const auto alloc = this->internal_allocate(alignement, count); alloc) {
                return alloc;
            } else {
                this->upgrade_new_block();
                return this->internal_allocate(alignement, count);
            }
        }

        void do_deallocate(void* pointer, size_t bytes, size_t alignment) override { return; }

        bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
            return this == &other;
        }
    };

    class FallBackAllocator : public std::pmr::memory_resource {
    private:
        bool                                         using_fallback{false};
        usize                                        index{0};
        std::span<u8>                                memory{};
        std::observer_ptr<std::pmr::memory_resource> fallback_allocator{nullptr};

    protected:
        void* do_allocate(usize count, usize alignment) override {
            void* out_pointer{};

            if (!this->using_fallback) {
                usize space   = memory.size() - this->index;
                void* address = &this->memory[this->index];
                if (std::align(alignment, count, address, space)) {

                    uintptr_t begin  = (uintptr_t)this->memory.data();
                    uintptr_t start  = std::bit_cast<uintptr_t>(address);
                    u32       offset = start - begin;
                    out_pointer      = &this->memory[offset];
                    this->index      = offset + count;

                    return out_pointer;
                }
                std::println("Using fallback allocator");
                this->using_fallback = true;
            }
            out_pointer = fallback_allocator->allocate(count, alignment);
            return out_pointer;
        }

        void do_deallocate(void* pointer, size_t bytes, size_t alignment) override {

            uintptr_t value = (uintptr_t)pointer;
            uintptr_t start = (uintptr_t)this->memory.data();
            uintptr_t end   = (uintptr_t)(this->memory.data() + this->memory.size() - 1);

            if (value >= start && value <= end) {
                return;
            }

            if (this->using_fallback) {
                this->fallback_allocator->deallocate(pointer, bytes, alignment);
                return;
            }

            throw std::logic_error("Invalid allocation state");
        }

        bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
            return this == &other;
        }

    public:
        FallBackAllocator(
            std::observer_ptr<std::pmr::memory_resource> fallback, std::span<u8> memory
        ) noexcept
            : fallback_allocator(fallback), memory(memory) {};

        void reset() noexcept {
            this->index          = 0;
            this->using_fallback = false;
        }
    };
} // namespace soil