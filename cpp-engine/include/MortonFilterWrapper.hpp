#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cstdint>

// Forward declaration - no external includes
struct morton_handle_t;

class MortonFilterWrapper {
public:
    MortonFilterWrapper();
    ~MortonFilterWrapper();
    
    // Initialize with expected capacity and false positive rate
    bool initialize(size_t capacity, double false_positive_rate = 0.01);
    
    // Single element operations
    bool insert(const std::string& element);
    bool contains(const std::string& element) const;
    
    // Batch operations
    bool insert_batch(const std::vector<std::string>& elements);
    bool contains_batch(const std::vector<std::string>& elements, 
                       std::vector<bool>& results) const;
    
    // Memory management
    size_t get_memory_usage() const;
    size_t get_count() const;
    
    // Save/load for persistence
    bool save_to_file(const std::string& path) const;
    bool load_from_file(const std::string& path);

private:
    morton_handle_t* handle_;
    
    // Remove the adapter function declarations since we're using stub implementation
    // bool adapter_initialize(size_t capacity, double false_positive_rate);
    // bool adapter_insert(const char* data, size_t length);
    // bool adapter_contains(const char* data, size_t length) const;
    // bool adapter_insert_batch(const std::vector<const char*>& data_ptrs, 
    //                          const std::vector<size_t>& lengths);
    // bool adapter_contains_batch(const std::vector<const char*>& data_ptrs,
    //                            const std::vector<size_t>& lengths,
    //                            std::vector<bool>& results) const;
    // void adapter_free();
};