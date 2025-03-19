#pragma once
#include <clay.h>
#include <random>
#include <types.hpp>
namespace soil {

    class ClayIdGenerator {
    public:
        static ClayIdGenerator& instance() noexcept;

        Clay_ElementId new_id() noexcept;
        void           wipe() noexcept;

        ClayIdGenerator() noexcept;
    private:
    private:

        std::mt19937_64 random_engine;
        u64 seed_base;
        

    };
}