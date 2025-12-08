#!/usr/bin/env python3
"""Debug extraction."""

import sys
import re
sys.path.insert(0, '/Users/jstarcher/devel/kramerscript')

from kramerscript import tokenize, KramerInterpreter

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

tokens = tokenize(source)

interp = KramerInterpreter()

# Test extraction
test_route_token = 'ROUTE "/HEALTH" {'
match = re.search(r'"([^"]*)"', test_route_token)
print(f"Extracted path: {match.group(1)}")

# Now parse
interp.parse(tokens)
print(f"\nRoutes registered: {interp.routes}")
