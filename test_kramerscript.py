#!/usr/bin/env python3
"""Test script to verify KramerScript interpreter functionality."""

import sys
sys.path.insert(0, '/Users/jstarcher/devel/kramerscript')

from kramerscript import tokenize, KramerInterpreter

def test_basic_parsing():
    """Test basic KramerScript parsing."""
    source = """
    YO JERRY!
    
    SERVER "0.0.0.0:413" {
        ROUTE "/" {
            SERVE 200 "I'm out there Jerry and I'm loving every minute of it!"
        }
        
        ROUTE "/health" {
            SERVE 200 "Giddy up!"
        }
    }
    
    "These pretzels are making me thirsty!" LOG
    
    NOINE!
    """
    
    # Test tokenization
    tokens = tokenize(source)
    print(f"✓ Tokenization successful: {len(tokens)} tokens")
    
    # Test parsing
    interp = KramerInterpreter()
    interp.parse(tokens)
    print(f"✓ Parsing successful")
    
    # Verify routes were registered
    assert "/" in interp.routes, "Root route not found"
    print(f"✓ Root route registered")
    
    assert "/health" in interp.routes, f"Health route not found. Available: {interp.routes.keys()}"
    print(f"✓ Health route registered")
    
    # Verify server config
    assert interp.server_config == "0.0.0.0:413", f"Server config incorrect: {interp.server_config}"
    print(f"✓ Server config correct: {interp.server_config}")
    
    # Verify logs
    assert len(interp.logs) == 1, f"Expected 1 log, got {len(interp.logs)}"
    assert "These pretzels" in interp.logs[0], "Log message incorrect"
    print(f"✓ Logging works: '{interp.logs[0]}'")
    
    # Verify route handlers
    status, body = interp.routes["/"]
    assert status == 200, f"Status incorrect: {status}"
    assert "loving" in body.lower(), f"Body incorrect: {body}"
    print(f"✓ Route handler parsing correct")
    
    print("\n✅ All tests passed! KramerScript is ready to serve!")

if __name__ == '__main__':
    test_basic_parsing()
