#include "numa_optimized_filter.hpp"
#include <iostream>
#include <chrono>
#include <unordered_map>

NUMAOptimizedFilter::NUMAOptimizedFilter() 
    : num_numa_nodes_(1), processed_counts_size_(0) {
}

NUMAOptimizedFilter::~NUMAOptimizedFilter() {
    running_ = false;
    
    // Stop worker threads
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

bool NUMAOptimizedFilter::initialize(size_t total_capacity) {
    // Initialize NUMA system
    if (!CoherentMemoryManager::initialize()) {
        std::cout << "[NUMAFilter] Using single-node fallback mode" << std::endl;
    }
    
    num_numa_nodes_ = CoherentMemoryManager::get_num_numa_nodes();
    size_t per_node_capacity = total_capacity / num_numa_nodes_;
    
    std::cout << "[NUMAFilter] Initializing with " << num_numa_nodes_ 
              << " NUMA nodes, " << per_node_capacity << " capacity each" << std::endl;
    
    // FIX: Use array instead of vector for atomics
    processed_counts_size_ = num_numa_nodes_;
    processed_counts_ = std::make_unique<std::atomic<uint64_t>[]>(num_numa_nodes_);
    
    // Initialize all atomics to 0
    for (int i = 0; i < num_numa_nodes_; ++i) {
        processed_counts_[i].store(0, std::memory_order_relaxed);
        
        // Create filter for this node using unique_ptr
        auto filter = std::make_unique<PerformanceOptimizedFilter>();
        filter->initialize(per_node_capacity);
        per_node_filters_.push_back(std::move(filter));
        
        // Create queue for this node
        per_node_queues_.emplace_back();
    }
    
    // Start worker threads
    for (int i = 0; i < num_numa_nodes_; ++i) {
        worker_threads_.emplace_back(&NUMAOptimizedFilter::worker_loop, this, i);
    }
    
    std::cout << "[NUMAFilter] Initialization complete with " << worker_threads_.size() << " worker threads" << std::endl;
    return true;
}

size_t NUMAOptimizedFilter::route_to_numa(const std::string& url) const {
    // Simple hash-based routing for good distribution
    std::hash<std::string> hasher;
    return hasher(url) % num_numa_nodes_;
}

bool NUMAOptimizedFilter::contains(const std::string& url) {
    if (per_node_filters_.empty()) return false;
    
    size_t numa_node = route_to_numa(url);
    return per_node_filters_[numa_node]->contains(url);
}

void NUMAOptimizedFilter::insert(const std::string& url) {
    if (per_node_queues_.empty()) return;
    
    size_t numa_node = route_to_numa(url);
    per_node_queues_[numa_node].enqueue(url);
}

void NUMAOptimizedFilter::check_url(const std::string& url) {
    // For now, just insert to demonstrate the flow
    insert(url);
}

void NUMAOptimizedFilter::insert_batch(const std::vector<std::string>& urls) {
    if (per_node_queues_.empty()) return;
    
    // Group URLs by NUMA node for efficient batch processing
    std::vector<std::vector<std::string>> batches(num_numa_nodes_);
    
    for (const auto& url : urls) {
        size_t numa_node = route_to_numa(url);
        batches[numa_node].push_back(url);
    }
    
    // Enqueue batches to respective NUMA nodes
    for (size_t i = 0; i < batches.size(); ++i) {
        if (!batches[i].empty()) {
            for (const auto& url : batches[i]) {
                per_node_queues_[i].enqueue(url);
            }
        }
    }
}

void NUMAOptimizedFilter::worker_loop(int numa_node) {
    // Pin this thread to the target NUMA node
    if (!CoherentMemoryManager::pin_thread_to_numa(numa_node)) {
        std::cerr << "[Worker " << numa_node << "] Failed to pin to NUMA node" << std::endl;
    } else {
        std::cout << "[Worker " << numa_node << "] Successfully pinned to NUMA node" << std::endl;
    }
    
    if (numa_node >= per_node_queues_.size() || numa_node >= per_node_filters_.size()) {
        std::cerr << "[Worker " << numa_node << "] Invalid NUMA node index" << std::endl;
        return;
    }
    
    auto& queue = per_node_queues_[numa_node];
    auto* filter = per_node_filters_[numa_node].get();
    
    while (running_) {
        std::string url;
        
        // Try to dequeue a URL
        if (queue.try_dequeue(url)) {
            // Process the URL
            filter->insert(url);
            processed_counts_[numa_node].fetch_add(1, std::memory_order_relaxed);
            
            std::cout << "[Worker " << numa_node << "] Processed: " << url << std::endl;
        } else {
            // Brief sleep to avoid busy-waiting when queue is empty
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
}

void NUMAOptimizedFilter::print_stats() const {
    std::cout << "\n=== NUMA Filter Statistics ===" << std::endl;
    std::cout << "NUMA Nodes: " << num_numa_nodes_ << std::endl;
    
    uint64_t total_processed = 0;
    for (size_t i = 0; i < processed_counts_size_; ++i) {
        uint64_t count = processed_counts_[i].load(std::memory_order_relaxed);
        total_processed += count;
        std::cout << "Node " << i << " processed: " << count << " URLs" << std::endl;
    }
    
    std::cout << "Total processed: " << total_processed << " URLs" << std::endl;
}