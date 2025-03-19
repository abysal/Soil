#include "./simple_id_generator.hpp"
#include "clay/templates.hpp"
#include <chrono>
#include <memory>
#include <types.hpp>

namespace soil {

    static std::unique_ptr<ClayIdGenerator> generator{std::make_unique<ClayIdGenerator>(ClayIdGenerator{})};

    ClayIdGenerator &ClayIdGenerator::instance() noexcept { return *generator; }

    Clay_ElementId ClayIdGenerator::new_id() noexcept {
        u32 base_number = this->random_engine();

        while (!base_number) {
            base_number = this->random_engine();
        }

        return Clay_ElementId{
            .id       = base_number,
            .offset   = 0,
            .baseId   = (u32)this->seed_base,
            .stringId = {.length = 0, .chars = NULL}
        };
    }

    void ClayIdGenerator::wipe() noexcept { this->random_engine.seed(this->seed_base); }

    ClayIdGenerator::ClayIdGenerator() noexcept
        : random_engine(std::chrono::system_clock::now().time_since_epoch().count()) {
        this->seed_base = std::chrono::system_clock::now().time_since_epoch().count();
    }
} // namespace soil