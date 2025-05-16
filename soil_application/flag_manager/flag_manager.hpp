#pragma once

#include <atomic>

#define FLAG(name)                                                                             \
    std::atomic_flag name{};                                                                   \
    bool             process_##name() {                                                        \
        const auto value = this->name.test();                                      \
        this->name.clear();                                                        \
        return value;                                                              \
    }                                                                                          \
    void set_##name() { (void)this->name.test_and_set(); }

namespace soil {
    struct FlagManager {
        FLAG(rebuild_tree)

        static FlagManager& flag_manager() {
            static FlagManager manager{};
            return manager;
        }
    };
} // namespace soil

#undef FLAG