#pragma once
#include "../../types.hpp"
#include "pointer.hpp"
#include <cstdint>
#include <memory>
#include <memory_resource>
#include <print>
#include <span>
#include <stdexcept>

namespace soil {

    class FallBackAllocator : public std::pmr::memory_resource {
    private:
        bool                                         using_fallback{false};
        usize                                        index{0};
        std::span<u8>                                memory{};
        std::observer_ptr<std::pmr::memory_resource> fallback_allocator{nullptr
        };

    protected:
        void *do_allocate(usize count, usize alignment) override {
            void *out_pointer{};

            if (!this->using_fallback) {
                usize space   = memory.size() - this->index;
                void *address = &this->memory[this->index];
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

        void
        do_deallocate(void *pointer, size_t bytes, size_t alignment) override {

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

        bool do_is_equal(const std::pmr::memory_resource &other
        ) const noexcept override {
            return this == &other;
        }

    public:
        FallBackAllocator(
            std::observer_ptr<std::pmr::memory_resource> fallback,
            std::span<u8>                                memory
        ) noexcept
            : fallback_allocator(fallback), memory(memory) {};

        void reset() noexcept {
            this->index          = 0;
            this->using_fallback = false;
        }
    };
} // namespace soil