import sys
import os

# Add current directory to path
sys.path.append(os.path.dirname(__file__))

try:
    import llamashield_py as llamashield_engine
    print("✅ C++ engine imported successfully")
except ImportError as e:
    print(f"❌ Failed to import C++ engine: {e}")
    print("Available files in python directory:")
    for f in os.listdir('.'):
        if 'llamashield' in f.lower() or f.endswith('.pyd'):
            print(f"  - {f}")
    sys.exit(1)

# The rest of your web_service.py code remains the same...
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from typing import List, Dict
import asyncio
import logging
from llm_orchestrator import LLMOrchestrator, ThreatAnalysis

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = FastAPI(title="LlamaShield API", version="1.0.0")

class URLRequest(BaseModel):
    urls: List[str]
    use_ai_analysis: bool = True

class URLResponse(BaseModel):
    url: str
    is_blocked: bool
    reason: str
    confidence: float = 0.0

class AnalysisRequest(BaseModel):
    urls: List[str]

class AnalysisResponse(BaseModel):
    analyses: List[Dict]

class LlamaShieldService:
    def __init__(self, cerebras_api_key: str):
        self.numa_filter = llamashield_engine.NUMAOptimizedFilter()
        self.numa_filter.initialize(1000000)  # 1M capacity
        
        self.llm_orchestrator = LLMOrchestrator(cerebras_api_key)
        logger.info("LlamaShield service initialized")
    
    def check_urls(self, urls: List[str]) -> List[URLResponse]:
        """Check URLs against all filter layers"""
        results = []
        for url in urls:
            is_blocked = self.numa_filter.contains(url)
            reason = "L2_Morton" if is_blocked else "ALLOWED"
            results.append(URLResponse(
                url=url,
                is_blocked=is_blocked,
                reason=reason
            ))
        return results
    
    async def analyze_urls_with_ai(self, urls: List[str]) -> List[ThreatAnalysis]:
        """Analyze URLs with AI and update filters"""
        return await self.llm_orchestrator.analyze_and_update_filters(urls, self.numa_filter)

# Initialize service (in production, get API key from environment)
service = LlamaShieldService("csk-pp39v6wh9dtwtm8ycdn268vje3fn6hmenem6xfh8hexk33mf")

@app.get("/")
async def root():
    return {"message": "LlamaShield URL Filtering API", "status": "operational"}

@app.post("/check-urls", response_model=List[URLResponse])
async def check_urls(request: URLRequest):
    """Check URLs against filtering system"""
    try:
        results = service.check_urls(request.urls)
        
        # If AI analysis requested and some URLs weren't blocked
        if request.use_ai_analysis:
            uncertain_urls = [result.url for result in results if not result.is_blocked]
            if uncertain_urls:
                logger.info(f"Sending {len(uncertain_urls)} uncertain URLs to AI analysis")
                ai_analyses = await service.analyze_urls_with_ai(uncertain_urls)
                
                # Update results based on AI analysis
                for analysis in ai_analyses:
                    if analysis.is_malicious:
                        # Find and update the corresponding result
                        for result in results:
                            if result.url == analysis.url:
                                result.is_blocked = True
                                result.reason = f"AI_{analysis.threat_type}"
                                result.confidence = analysis.confidence
                                break
        
        return results
    
    except Exception as e:
        logger.error(f"Error checking URLs: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/analyze-urls", response_model=AnalysisResponse)
async def analyze_urls(request: AnalysisRequest):
    """Analyze URLs with AI only (doesn't update filters)"""
    try:
        analyses = await service.analyze_urls_with_ai(request.urls)
        return AnalysisResponse(
            analyses=[analysis.__dict__ for analysis in analyses]
        )
    except Exception as e:
        logger.error(f"Error analyzing URLs: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/stats")
async def get_stats():
    """Get system statistics"""
    service.numa_filter.print_stats()
    return {"status": "Statistics printed to console"}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)