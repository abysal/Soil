#pragma once

#include <cstdlib>

namespace soil {
#ifdef _MSC_VER
    void *aligned_alloc_wrapper(std::size_t size, std::size_t alignment) {
        return _aligned_malloc(size, alignment); // For MSVC
    }
#else
    void *aligned_alloc_wrapper(std::size_t size, std::size_t alignment) {
        return std::aligned_alloc(alignment, size); // For other compilers (C++17+)
    }
#endif

    
    // Custom aligned memory deallocation function
    void aligned_free_wrapper(void *ptr) {
#ifdef _MSC_VER
        _aligned_free(ptr); // MSVC aligned free
#else
        std::free(ptr); // C++ aligned memory uses free
#endif
    }
}