import sys
import os

# Add build directory to path
sys.path.append('./build/Release')

try:
    import llamashield_py as ls
    print("✅ SUCCESS: Imported llamashield_py from build directory!")
    
    # Quick test
    numa_filter = ls.NUMAOptimizedFilter()
    if numa_filter.initialize(100000):
        print("✅ NUMA filter initialized")
        print(f"Contains test: {numa_filter.contains('test.com')}")
        numa_filter.print_stats()
    else:
        print("❌ Failed to initialize")
        
except ImportError as e:
    print(f"❌ Import failed: {e}")