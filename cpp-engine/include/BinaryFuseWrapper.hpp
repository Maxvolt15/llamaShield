#pragma once

#include <cstdint>
#include <vector>
#include <string>

// Forward declaration
struct binfuse_handle_t;

class BinaryFuseWrapper {
private:
    binfuse_handle_t* handle_;

public:
    BinaryFuseWrapper();
    ~BinaryFuseWrapper();

    bool build_from_keys(const std::vector<uint64_t>& keys);
    bool contains(uint64_t key) const;
    bool save_to_file(const std::string& path) const;
    bool load_from_file(const std::string& path);
    
    static uint64_t hash_url(const std::string& url);

private:
    bool adapter_build(binfuse_handle_t** out_handle, const uint64_t* keys, size_t n);
    bool adapter_free(binfuse_handle_t* h);
    bool adapter_contains(binfuse_handle_t* h, uint64_t key) const;
    bool adapter_serialize(binfuse_handle_t* h, std::ostream& out) const;
    binfuse_handle_t* adapter_deserialize(std::istream& in) const;
};