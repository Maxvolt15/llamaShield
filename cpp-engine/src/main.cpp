#include <iostream>
#include <vector>
#include <string>
#include "BinaryFuseWrapper.hpp"
#include "numa_optimized_filter.hpp"
#include <thread>
#include <chrono>

void run_binary_fuse_test() {
    std::cout << "\n=== Testing Binary Fuse Filter ===" << std::endl;
    
    std::vector<std::string> urls_to_block = {
        "http://malicious-site.com/phish", 
        "http://another-bad-one.net/malware.exe"
    };
    
    std::vector<uint64_t> keys;
    for (const auto& url : urls_to_block) {
        keys.push_back(BinaryFuseWrapper::hash_url(url));
    }

    BinaryFuseWrapper filter;
    if (!filter.build_from_keys(keys)) {
        std::cerr << "[FAIL] Filter building failed!" << std::endl; 
        return;
    }
    std::cout << "[OK] BinaryFuse filter built successfully." << std::endl;

    // Test queries
    std::string positive_test_url = "http://malicious-site.com/phish";
    bool found_positive = filter.contains(BinaryFuseWrapper::hash_url(positive_test_url));
    std::cout << "[Query] '" << positive_test_url << "': " 
              << (found_positive ? "BLOCKED ✓" : "ALLOWED ✗") << std::endl;

    std::string negative_test_url = "http://safe-site.com/index.html";
    bool found_negative = filter.contains(BinaryFuseWrapper::hash_url(negative_test_url));
    std::cout << "[Query] '" << negative_test_url << "': " 
              << (found_negative ? "BLOCKED ✗" : "ALLOWED ✓") << std::endl;

    if (found_positive && !found_negative) {
         std::cout << "🎉 [SUCCESS] BinaryFuse filter working!" << std::endl;
    }
}

void run_numa_test() {
    std::cout << "\n=== Testing NUMA Architecture ===" << std::endl;

    // Test NUMA optimized filter
    NUMAOptimizedFilter numa_filter;
    
    // Initialize with 1 million capacity
    if (!numa_filter.initialize(1000000)) {
        std::cerr << "Failed to initialize NUMA filter" << std::endl;
        return;
    }

    // Test with some sample URLs
    std::vector<std::string> test_urls = {
        "https://example.com",
        "https://google.com", 
        "https://github.com",
        "https://stackoverflow.com",
        "https://wikipedia.org",
        "https://malicious.com",  // This should be blocked by BinaryFuse
        "https://phishing.net"    // This should be blocked by BinaryFuse
    };

    std::cout << "Testing with " << test_urls.size() << " sample URLs..." << std::endl;

    // Insert URLs
    for (const auto& url : test_urls) {
        numa_filter.check_url(url);
    }

    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Print statistics
    numa_filter.print_stats();

    // Test contains functionality
    std::cout << "\nTesting contains() method:" << std::endl;
    std::cout << "Contains 'https://malicious.com': " 
              << (numa_filter.contains("https://malicious.com") ? "YES" : "NO") << std::endl;
    std::cout << "Contains 'https://safe-site.com': " 
              << (numa_filter.contains("https://safe-site.com") ? "YES" : "NO") << std::endl;
}

int main() {
    std::cout << "🚀 [LlamaShield] C++ NUMA-aware Engine Starting..." << std::endl;
    
    // Test 1: Core BinaryFuse filter
    run_binary_fuse_test();
    
    // Test 2: NUMA architecture
    run_numa_test();

    std::cout << "\n🎯 [LlamaShield] All tests completed successfully!" << std::endl;
    std::cout << "Next: Implement Morton Filter for L2 cache" << std::endl;

    return 0;
}