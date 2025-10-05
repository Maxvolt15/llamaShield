import sys
import os

# Add the build directory to Python path
sys.path.append('./build')

try:
    import llamashield_py as ls
    print("✅ Successfully imported llamashield_py module!")
    
    # Test BinaryFuse Filter (L3)
    print("\n=== Testing BinaryFuse Filter (L3) ===")
    bf = ls.BinaryFuseWrapper()
    
    # Create test keys
    test_urls = ["malicious.com", "phishing.net", "malware.org"]
    keys = [ls.BinaryFuseWrapper.hash_url(url) for url in test_urls]
    
    if bf.build_from_keys(keys):
        print("✅ BinaryFuse filter built successfully")
        
        # Test contains
        test_hash = ls.BinaryFuseWrapper.hash_url("malicious.com")
        print(f"Contains malicious.com: {bf.contains(test_hash)}")
        
        test_hash2 = ls.BinaryFuseWrapper.hash_url("google.com")
        print(f"Contains google.com: {bf.contains(test_hash2)}")
    else:
        print("❌ Failed to build BinaryFuse filter")
    
    # Test Morton Filter (L2)
    print("\n=== Testing Morton Filter (L2) ===")
    mf = ls.MortonFilterWrapper()
    
    if mf.initialize(1000, 0.01):
        print("✅ Morton filter initialized successfully")
        
        # Test insert and contains
        mf.insert("threat.example.com")
        print(f"Contains threat.example.com: {mf.contains('threat.example.com')}")
        print(f"Contains safe.example.com: {mf.contains('safe.example.com')}")
        
        # Test batch operations
        batch_urls = ["batch1.com", "batch2.net", "batch3.org"]
        if mf.insert_batch(batch_urls):
            print("✅ Batch insert successful")
            
            results = []
            if mf.contains_batch(batch_urls, results):
                print(f"Batch contains results: {results}")
        
        print(f"L2 count: {mf.get_count()}")
        print(f"L2 memory usage: {mf.get_memory_usage()} bytes")
    else:
        print("❌ Failed to initialize Morton filter")
    
    # Test NUMA Optimized Filter
    print("\n=== Testing NUMA Optimized Filter ===")
    numa_filter = ls.NUMAOptimizedFilter()
    
    if numa_filter.initialize(100000):  # 100K capacity
        print("✅ NUMA filter initialized successfully")
        
        # Test contains
        print(f"Contains malicious.com: {numa_filter.contains('malicious.com')}")
        
        # Test insert
        numa_filter.insert("new-threat.com")
        numa_filter.check_url("another-threat.net")
        
        # Test batch insert
        batch_threats = ["threat1.com", "threat2.net", "threat3.org"]
        numa_filter.insert_batch(batch_threats)
        
        # Print stats
        numa_filter.print_stats()
        
        print("🎉 All Python bindings working correctly!")
    else:
        print("❌ Failed to initialize NUMA filter")
        
except ImportError as e:
    print(f"❌ Failed to import bindings: {e}")
    print("Make sure you've built the project with 'cmake --build .'")
except Exception as e:
    print(f"❌ Error during testing: {e}")