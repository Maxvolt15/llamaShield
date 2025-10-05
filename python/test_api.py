import requests
import json

def test_api():
    print("Testing LlamaShield API...")
    
    # Test data
    test_urls = [
        "https://example.com",
        "https://malicious.com",  # Should be blocked (in L3)
        "https://phishing.net",   # Should be blocked (in L3) 
        "https://google.com",
        "https://github.com"
    ]
    
    response = requests.post(
        "http://localhost:8000/check-urls",
        json={
            "urls": test_urls,
            "use_ai_analysis": False  # Disable AI for now
        }
    )
    
    print(f"Status Code: {response.status_code}")
    
    if response.status_code == 200:
        results = response.json()
        print("\n📊 URL Check Results:")
        for result in results:
            status = "🔴 BLOCKED" if result['is_blocked'] else "🟢 ALLOWED"
            print(f"  {status}: {result['url']} ({result['reason']})")
        
        blocked_count = sum(1 for r in results if r['is_blocked'])
        print(f"\n📈 Summary: {blocked_count}/{len(results)} URLs blocked")
    else:
        print(f"Error: {response.text}")

if __name__ == "__main__":
    test_api()