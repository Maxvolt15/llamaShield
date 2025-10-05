#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "BinaryFuseWrapper.hpp"
#include "MortonFilterWrapper.hpp"

class PerformanceOptimizedFilter {
private:
    BinaryFuseWrapper binary_fuse_filter_;  // L3: Static historical threats
    MortonFilterWrapper morton_filter_;     // L2: Dynamic recent threats
    size_t capacity_;
    
public:
    PerformanceOptimizedFilter() : capacity_(0) {}
    
    bool initialize(size_t capacity) {
        capacity_ = capacity;
        
        std::cout << "[PerformanceFilter] Initializing L2+L3 filters with capacity: " 
                  << capacity << std::endl;
        
        // Initialize L3 with some test data (in real usage, this would be loaded from disk)
        std::vector<uint64_t> l3_test_keys = {
            BinaryFuseWrapper::hash_url("https://malicious.com"),
            BinaryFuseWrapper::hash_url("https://phishing.net"),
            BinaryFuseWrapper::hash_url("https://malware.org")
        };
        
        bool l3_ok = binary_fuse_filter_.build_from_keys(l3_test_keys);
        
        // Initialize L2 Morton Filter
        bool l2_ok = morton_filter_.initialize(capacity / 10, 0.01); // 10% of capacity
        
        std::cout << "[PerformanceFilter] L3 (BinaryFuse): " << (l3_ok ? "OK" : "FAIL") 
                  << ", L2 (Morton): " << (l2_ok ? "OK" : "FAIL") << std::endl;
        
        return l3_ok && l2_ok;
    }
    
    bool contains(const std::string& url) const {
        // Fast path: Check L2 Morton filter first (dynamic threats)
        if (morton_filter_.contains(url)) {
            std::cout << "[PerformanceFilter] L2 HIT: " << url << std::endl;
            return true;
        }
        
        // Slow path: Check L3 Binary Fuse filter (static threats)
        uint64_t hash = BinaryFuseWrapper::hash_url(url);
        if (binary_fuse_filter_.contains(hash)) {
            std::cout << "[PerformanceFilter] L3 HIT: " << url << std::endl;
            return true;
        }
        
        std::cout << "[PerformanceFilter] MISS: " << url << std::endl;
        return false;
    }
    
    void insert(const std::string& url) {
        // Add to L2 Morton filter (dynamic cache)
        if (morton_filter_.insert(url)) {
            std::cout << "[PerformanceFilter] Added to L2: " << url << std::endl;
        } else {
            std::cerr << "[PerformanceFilter] Failed to add to L2: " << url << std::endl;
        }
        
        // Note: L3 Binary Fuse is static and not updated at runtime
        // In production, L3 would be rebuilt periodically from updated threat intelligence
    }
    
    void insert_batch(const std::vector<std::string>& urls) {
        if (urls.empty()) return;
        
        std::cout << "[PerformanceFilter] Batch inserting " << urls.size() << " URLs to L2" << std::endl;
        
        if (morton_filter_.insert_batch(urls)) {
            std::cout << "[PerformanceFilter] Batch insert successful" << std::endl;
        } else {
            std::cerr << "[PerformanceFilter] Batch insert failed" << std::endl;
        }
    }
    
    size_t get_memory_usage() const {
        return morton_filter_.get_memory_usage() + sizeof(BinaryFuseWrapper);
    }
    
    size_t get_l2_count() const {
        return morton_filter_.get_count();
    }
    
    void print_stats() const {
        std::cout << "\n=== Performance Filter Statistics ===" << std::endl;
        std::cout << "L2 (Morton) entries: " << morton_filter_.get_count() << std::endl;
        std::cout << "L2 memory usage: " << morton_filter_.get_memory_usage() << " bytes" << std::endl;
        std::cout << "L3 (BinaryFuse): Static threat database" << std::endl;
    }
};