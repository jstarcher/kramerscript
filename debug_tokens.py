#!/usr/bin/env python3
"""Debug script to examine tokens."""

import sys
sys.path.insert(0, '/Users/jstarcher/devel/kramerscript')

from kramerscript import tokenize

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
for i, token in enumerate(tokens):
    print(f"{i}: {token}")
