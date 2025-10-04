#pragma once

#include "concurrentqueue.h"
#include "coherent_memory_manager.hpp"
#include "performance_optimized_filter.hpp"
#include <vector>
#include <thread>
#include <string>
#include <atomic>
#include <functional>
#include <iostream>
#include <memory>

class NUMAOptimizedFilter {
public:
    NUMAOptimizedFilter();
    ~NUMAOptimizedFilter();
    
    // Initialize the system with total capacity
    bool initialize(size_t total_capacity);
    
    // Check if URL exists in filters
    bool contains(const std::string& url);
    
    // Add URL to filters (will route to appropriate NUMA node)
    void insert(const std::string& url);
    
    // Process URLs in batch (more efficient)
    void insert_batch(const std::vector<std::string>& urls);
    
    // Get statistics
    void print_stats() const;
    
    // Public method to dispatch a URL for checking
    void check_url(const std::string& url);

private:
    void worker_loop(int numa_node);
    size_t route_to_numa(const std::string& url) const;
    
    int num_numa_nodes_;
    std::vector<std::unique_ptr<PerformanceOptimizedFilter>> per_node_filters_;
    std::vector<moodycamel::ConcurrentQueue<std::string>> per_node_queues_;
    std::vector<std::thread> worker_threads_;
    std::atomic<bool> running_{true};
    
    // FIX: Use unique_ptr to array instead of vector for atomics
    std::unique_ptr<std::atomic<uint64_t>[]> processed_counts_;
    size_t processed_counts_size_{0};
};