import sys
import os

try:
    import llamashield_engine as ls
    print("✅ SUCCESS: Imported llamashield_engine!")
    
    # Quick test
    print("Testing basic functionality...")
    
    # Test NUMA filter
    numa_filter = ls.NUMAOptimizedFilter()
    if numa_filter.initialize(100000):
        print("✅ NUMA filter initialized")
        
        # Test contains
        result = numa_filter.contains("malicious.com")
        print(f"Contains 'malicious.com': {result}")
        
        # Test insert
        numa_filter.insert("test-threat.com")
        
        # Print stats
        numa_filter.print_stats()
        print("🎉 C++ engine integrated successfully!")
    else:
        print("❌ Failed to initialize NUMA filter")
        
except ImportError as e:
    print(f"❌ Import failed: {e}")
    print("Current directory files:")
    for file in os.listdir('.'):
        if 'llamashield' in file.lower() or file.endswith('.pyd'):
            print(f"  - {file}")