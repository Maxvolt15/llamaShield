import sys
import os

# Add current directory to path
sys.path.append(os.path.dirname(__file__))

try:
    import llamashield_py as ls
    print("✅ SUCCESS: Imported llamashield_py!")
    
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
    print("Files in current directory:")
    for f in os.listdir('.'):
        if 'llamashield' in f.lower() or f.endswith('.pyd'):
            print(f"  - {f}")