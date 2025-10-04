#include <iostream>
#include <vector>
#include "binfuse/filter.hpp"

void test_binfuse_api() {
    std::cout << "Testing binfuse API..." << std::endl;
    
    // Test 1: Check available methods
    std::vector<uint64_t> keys = {1, 2, 3, 4, 5};
    
    try {
        // Try to construct filter
        binfuse::filter8 filter(keys);
        std::cout << "✓ filter8 construction successful" << std::endl;
        
        // Test contains
        bool result = filter.contains(3);
        std::cout << "✓ contains method works: " << result << std::endl;
        
        // Try to get size
        // filter.size_in_bytes(); // Comment out to see if it exists
        
    } catch (const std::exception& e) {
        std::cout << "✗ Error: " << e.what() << std::endl;
    }
}

int main() {
    test_binfuse_api();
    return 0;
}   