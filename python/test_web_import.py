import sys
import os

# Add current directory to path
sys.path.append(os.path.dirname(__file__))

try:
    import llamashield_py as llamashield_engine
    print("✅ SUCCESS: Imported llamashield_py for web service!")
    
    # Test the service initialization
    service = type('Service', (), {})  # Mock service class
    service.numa_filter = llamashield_engine.NUMAOptimizedFilter()
    
    if service.numa_filter.initialize(1000000):
        print("✅ Service NUMA filter initialized successfully")
        
        # Test a few URLs
        test_urls = ["https://example.com", "https://malicious.com", "https://google.com"]
        for url in test_urls:
            result = service.numa_filter.contains(url)
            print(f"URL: {url} -> Blocked: {result}")
        
        print("🎉 Web service ready to run!")
    else:
        print("❌ Failed to initialize service filter")
        
except ImportError as e:
    print(f"❌ Import failed: {e}")
    print("Files in python directory:")
    for f in os.listdir('.'):
        print(f"  - {f}")