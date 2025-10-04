#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "BinaryFuseWrapper.hpp"

class PerformanceOptimizedFilter {
private:
    BinaryFuseWrapper binary_fuse_filter_;
    std::vector<std::string> recent_urls_; // Simple cache for demo
    
public:
    PerformanceOptimizedFilter() = default;
    
    bool initialize(size_t capacity) {
        std::cout << "[PerformanceFilter] Initialized with capacity: " << capacity << std::endl;
        
        // For demo, create some test data
        std::vector<uint64_t> test_keys = {
            BinaryFuseWrapper::hash_url("https://malicious.com"),
            BinaryFuseWrapper::hash_url("https://phishing.net")
        };
        
        return binary_fuse_filter_.build_from_keys(test_keys);
    }
    
    bool contains(const std::string& url) const {
        // Check BinaryFuse filter first
        uint64_t hash = BinaryFuseWrapper::hash_url(url);
        if (binary_fuse_filter_.contains(hash)) {
            return true;
        }
        
        // Simple cache check (will be replaced with Morton filter)
        for (const auto& cached_url : recent_urls_) {
            if (cached_url == url) {
                return true;
            }
        }
        
        return false;
    }
    
    void insert(const std::string& url) {
        // Add to recent URLs cache
        recent_urls_.push_back(url);
        
        // Keep cache size reasonable
        if (recent_urls_.size() > 1000) {
            recent_urls_.erase(recent_urls_.begin());
        }
        
        std::cout << "[PerformanceFilter] Inserted URL: " << url << std::endl;
    }
    
    size_t get_memory_usage() const {
        return recent_urls_.size() * sizeof(std::string);
    }
};