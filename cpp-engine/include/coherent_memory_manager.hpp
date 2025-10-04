#pragma once

#include <iostream>
#include <vector>
#include <cstring>
#include <memory>

// Windows NUMA support
#ifdef _WIN32
#include <windows.h>
#include <winnt.h>
#endif

class CoherentMemoryManager {
public:
    // Initialize NUMA system - call this first
    static bool initialize() {
#ifdef _WIN32
        // Check if NUMA is available on Windows
        DWORD highestNodeNumber;
        if (!GetNumaHighestNodeNumber(&highestNodeNumber)) {
            std::cout << "[NUMA] Running in single-node mode on Windows" << std::endl;
            return false;
        }
        std::cout << "[NUMA] Windows system with " << highestNodeNumber + 1 << " nodes" << std::endl;
        return true;
#else
        std::cout << "[NUMA] Running on non-Windows system, using fallback" << std::endl;
        return false;
#endif
    }

    // Get number of NUMA nodes
    static int get_num_numa_nodes() {
#ifdef _WIN32
        DWORD highestNodeNumber;
        if (GetNumaHighestNodeNumber(&highestNodeNumber)) {
            return highestNodeNumber + 1;
        }
#endif
        return 1; // Single node fallback
    }

    // Pin thread to specific CPU cores of a NUMA node (Windows version)
    static bool pin_thread_to_numa(int numa_node) {
#ifdef _WIN32
        DWORD_PTR processAffinity, systemAffinity;
        if (!GetProcessAffinityMask(GetCurrentProcess(), &processAffinity, &systemAffinity)) {
            return false;
        }
        
        // Set thread affinity to first CPU in the node (simplified)
        DWORD_PTR affinityMask = 1ULL << numa_node;
        return SetThreadAffinityMask(GetCurrentThread(), affinityMask) != 0;
#else
        (void)numa_node;
        return false;
#endif
    }

    // Allocate memory - simplified for Windows
    static void* allocate_numa_local(size_t size, int numa_node) {
        (void)numa_node; // NUMA allocation not implemented for Windows yet
        return malloc(size);
    }

    // Free memory
    static void free_numa_local(void* ptr, size_t size) {
        (void)size;
        free(ptr);
    }

    // Get current NUMA node
    static int get_current_numa_node() {
        return 0; // Simplified
    }
};