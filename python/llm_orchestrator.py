import asyncio
import aiohttp
import json
from typing import List, Dict, Optional
from dataclasses import dataclass
import logging

@dataclass
class ThreatAnalysis:
    url: str
    is_malicious: bool
    confidence: float
    threat_type: str
    indicators: List[str]

class CerebrasClient:
    def __init__(self, api_key: str, base_url: str = "https://api.cerebras.ai/v1"):
        self.api_key = api_key
        self.base_url = base_url
        self.session: Optional[aiohttp.ClientSession] = None
        self.logger = logging.getLogger(__name__)
    
    async def __aenter__(self):
        self.session = aiohttp.ClientSession(
            headers={"Authorization": f"Bearer {self.api_key}"},
            timeout=aiohttp.ClientTimeout(total=30)
        )
        return self
    
    async def __aexit__(self, exc_type, exc_val, exc_tb):
        if self.session:
            await self.session.close()
    
    async def analyze_urls_batch(self, urls: List[str]) -> List[ThreatAnalysis]:
        """Analyze batch of URLs using Cerebras LLM"""
        if not self.session:
            raise RuntimeError("Client not initialized. Use async context manager.")
        
        prompts = self._create_security_prompts(urls)
        results = []
        
        try:
            async with self.session.post(
                f"{self.base_url}/completions",
                json={
                    "model": "cerebras-llama-70b",
                    "prompt": prompts,
                    "max_tokens": 500,
                    "temperature": 0.1,
                    "batch_size": len(urls)
                }
            ) as response:
                if response.status == 200:
                    data = await response.json()
                    results = self._parse_llm_responses(urls, data)
                else:
                    self.logger.error(f"Cerebras API error: {response.status}")
                    # Fallback: mark all as suspicious for safety
                    results = [ThreatAnalysis(url, True, 0.5, "api_error", []) for url in urls]
        
        except Exception as e:
            self.logger.error(f"Cerebras API exception: {e}")
            results = [ThreatAnalysis(url, True, 0.5, "api_exception", []) for url in urls]
        
        return results
    
    def _create_security_prompts(self, urls: List[str]) -> str:
        """Create security analysis prompts for LLM"""
        prompts = []
        for url in urls:
            prompt = f"""
            SECURITY ANALYSIS - URL THREAT CLASSIFICATION
            
            URL: {url}
            
            Analyze this URL for security threats. Consider:
            - Phishing indicators (fake login, credential harvesting)
            - Malware distribution (executable downloads, exploit kits)  
            - Scam/fraud patterns (fake services, financial fraud)
            - Social engineering tactics
            
            Respond with JSON only:
            {{
                "is_malicious": true/false,
                "confidence": 0.0-1.0,
                "threat_type": "phishing/malware/scam/safe",
                "indicators": ["list", "of", "specific", "indicators"]
            }}
            """
            prompts.append(prompt)
        
        return "\n---\n".join(prompts)
    
    def _parse_llm_responses(self, urls: List[str], api_response: Dict) -> List[ThreatAnalysis]:
        """Parse LLM responses into ThreatAnalysis objects"""
        results = []
        
        try:
            choices = api_response.get("choices", [])
            for url, choice in zip(urls, choices):
                text = choice.get("text", "").strip()
                
                # Try to extract JSON from response
                json_start = text.find('{')
                json_end = text.rfind('}') + 1
                
                if json_start != -1 and json_end != 0:
                    json_str = text[json_start:json_end]
                    try:
                        data = json.loads(json_str)
                        results.append(ThreatAnalysis(
                            url=url,
                            is_malicious=data.get("is_malicious", False),
                            confidence=data.get("confidence", 0.0),
                            threat_type=data.get("threat_type", "unknown"),
                            indicators=data.get("indicators", [])
                        ))
                        continue
                    except json.JSONDecodeError:
                        pass
                
                # Fallback if JSON parsing fails
                results.append(ThreatAnalysis(
                    url=url, 
                    is_malicious="malicious" in text.lower(),
                    confidence=0.7 if "malicious" in text.lower() else 0.3,
                    threat_type="unknown",
                    indicators=["llm_parse_error"]
                ))
        
        except Exception as e:
            self.logger.error(f"Failed to parse LLM responses: {e}")
            # Create fallback results
            results = [ThreatAnalysis(url, False, 0.3, "parse_error", []) for url in urls]
        
        return results

class LLMOrchestrator:
    def __init__(self, cerebras_api_key: str):
        self.cerebras_client = CerebrasClient(cerebras_api_key)
        self.logger = logging.getLogger(__name__)
    
    async def analyze_and_update_filters(self, urls: List[str], morton_filter) -> List[ThreatAnalysis]:
        """Analyze URLs with LLM and update Morton filter with threats"""
        if not urls:
            return []
        
        self.logger.info(f"Analyzing {len(urls)} URLs with Cerebras LLM")
        
        async with self.cerebras_client as client:
            analyses = await client.analyze_urls_batch(urls)
        
        # Update Morton filter with identified threats
        threats_to_block = [analysis.url for analysis in analyses if analysis.is_malicious]
        if threats_to_block:
            morton_filter.insert_batch(threats_to_block)
            self.logger.info(f"Added {len(threats_to_block)} threats to Morton filter")
        
        return analyses