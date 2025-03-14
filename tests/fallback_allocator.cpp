#include <cassert>
#include <cstring>
#include <gtest/gtest.h>
#include <Application/memory/fallback_allocator.hpp>
#include <print>
#include <string>

using namespace soil;

TEST(AllocatorTests, BasicAdd) {
    using Alloc = FallBackAllocator;

    constexpr usize allocation_size = 80960 * 4;

    std::span<u8> memory = std::span<u8>{
        (u8 *)std::aligned_alloc(
            std::alignment_of_v<std::max_align_t>, allocation_size
        ),
        allocation_size
    };

    auto backup_alloc = std::pmr::new_delete_resource();

    Alloc base_allocator{
        std::observer_ptr<std::pmr::memory_resource>(backup_alloc),
        memory
    };
    namespace pmr = std::pmr;


    pmr::vector<pmr::string> string_list{&base_allocator};
    

    for (int i = 0; i < 100; i++) {
        string_list.push_back(pmr::string{"Hello world!", &base_allocator});

        for (const auto& ele : string_list) {
            std::string str{ele.c_str()};
            std::string expected{"Hello World!"};
        }
    }


    
}
