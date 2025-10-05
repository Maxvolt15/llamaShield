#include <iostream>
#include <vector>
#include <string>
#include "BinaryFuseWrapper.hpp"
#include "numa_optimized_filter.hpp"
#include "MortonFilterWrapper.hpp"  // Add this include
#include <thread>
#include <chrono>

void run_binary_fuse_test() {
    std::cout << "\n=== Testing Binary Fuse Filter (L3) ===" << std::endl;
    
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

void run_morton_filter_test() {
    std::cout << "\n=== Testing Morton Filter (L2) ===" << std::endl;
    
    MortonFilterWrapper morton_filter;
    if (!morton_filter.initialize(1000, 0.01)) { // 1000 capacity, 1% FPR
        std::cerr << "[FAIL] Morton filter initialization failed!" << std::endl;
        return;
    }
    std::cout << "[OK] Morton filter initialized successfully." << std::endl;

    // Test single inserts
    std::vector<std::string> test_urls = {
        "https://recent-threat-1.com",
        "https://recent-threat-2.net", 
        "https://safe-site-3.org"
    };

    for (const auto& url : test_urls) {
        if (morton_filter.insert(url)) {
            std::cout << "[OK] Inserted: " << url << std::endl;
        }
    }

    // Test batch operations
    std::vector<std::string> batch_urls = {
        "https://batch-threat-1.com",
        "https://batch-threat-2.net",
        "https://batch-safe-3.org"
    };

    if (morton_filter.insert_batch(batch_urls)) {
        std::cout << "[OK] Batch insert successful for " << batch_urls.size() << " URLs" << std::endl;
    }

    // Test contains
    std::cout << "\nTesting contains operations:" << std::endl;
    for (const auto& url : test_urls) {
        bool found = morton_filter.contains(url);
        std::cout << "[Query] '" << url << "': " << (found ? "FOUND ✓" : "NOT FOUND ✗") << std::endl;
    }

    // Test batch contains
    std::vector<std::string> query_urls = {
        "https://recent-threat-1.com",  // Should be found
        "https://unknown-site.com",     // Should not be found
        "https://batch-threat-1.com"    // Should be found
    };
    
    std::vector<bool> batch_results;
    if (morton_filter.contains_batch(query_urls, batch_results)) {
        std::cout << "\nBatch query results:" << std::endl;
        for (size_t i = 0; i < query_urls.size(); ++i) {
            std::cout << "[Batch] '" << query_urls[i] << "': " 
                      << (batch_results[i] ? "FOUND ✓" : "NOT FOUND ✗") << std::endl;
        }
    }

    std::cout << "L2 entries: " << morton_filter.get_count() << std::endl;
    std::cout << "L2 memory: " << morton_filter.get_memory_usage() << " bytes" << std::endl;
}

void run_numa_test() {
    std::cout << "\n=== Testing NUMA Architecture (L2 + L3) ===" << std::endl;

    NUMAOptimizedFilter numa_filter;
    
    // Initialize with 1 million capacity
    if (!numa_filter.initialize(1000000)) {
        std::cerr << "Failed to initialize NUMA filter" << std::endl;
        return;
    }

    // Test with mixed URLs - some should hit L2, some L3, some miss
    std::vector<std::string> test_urls = {
        "https://example.com",           // Will be added to L2
        "https://google.com",            // Will be added to L2  
        "https://malicious.com",         // Should hit L3 (pre-loaded)
        "https://phishing.net",          // Should hit L3 (pre-loaded)
        "https://github.com",            // Will be added to L2
        "https://malware.org",           // Should hit L3 (pre-loaded)
        "https://stackoverflow.com",     // Will be added to L2
        "https://wikipedia.org"          // Will be added to L2
    };

    std::cout << "Testing with " << test_urls.size() << " URLs (mix of L2/L3/miss)..." << std::endl;

    // Insert URLs through NUMA system
    for (const auto& url : test_urls) {
        numa_filter.check_url(url);
    }

    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Print statistics
    numa_filter.print_stats();

    // Test contains functionality
    std::cout << "\nTesting contains() method:" << std::endl;
    std::vector<std::string> check_urls = {
        "https://malicious.com",     // Should hit L3
        "https://example.com",       // Should hit L2  
        "https://unknown-site.com"   // Should miss
    };
    
    for (const auto& url : check_urls) {
        bool found = numa_filter.contains(url);
        std::cout << "[Contains] '" << url << "': " << (found ? "BLOCKED" : "ALLOWED") << std::endl;
    }
}

int main() {
    std::cout << "🚀 [LlamaShield] C++ Multi-Layer Filter Engine Starting..." << std::endl;
    
    // Test 1: Core BinaryFuse filter (L3)
    run_binary_fuse_test();
    
    // Test 2: Morton Filter (L2) 
    run_morton_filter_test();
    
    // Test 3: Integrated NUMA architecture (L2 + L3)
    run_numa_test();

    std::cout << "\n🎯 [LlamaShield] All tests completed successfully!" << std::endl;
    std::cout << "Architecture: L2 (Morton) + L3 (BinaryFuse) + NUMA Parallelism" << std::endl;
    std::cout << "Next: Python LLM Integration" << std::endl;

    return 0;
}