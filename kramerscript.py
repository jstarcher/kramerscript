#!/usr/bin/env python3
"""
╔════════════════════════════════════════════════════════════════════════════╗
║                                                                            ║
║              KramerScript – A Language About Nothing™                      ║
║                                                                            ║
║  An esoteric HTTP server generator language using only Seinfeld quotes.   ║
║  "These pretzels are making me thirsty!"                                  ║
║                                                                            ║
╚════════════════════════════════════════════════════════════════════════════╝

EXAMPLE PROGRAM: server.kramer
═════════════════════════════════════════════════════════════════════════════

YO JERRY!

SERVER "0.0.0.0:413" {
    ROUTE "/" {
        SERVE 200 "I'm out there Jerry and I'm loving every minute of it!"
    }
    
    ROUTE "/health" {
        SERVE 200 "Giddy up!"
    }
    
    ROUTE "/notexist" {
        SERVE 404 "Master of your domain? Not found!"
    }
}

"These pretzels are making me thirsty!" LOG

NOINE!

═════════════════════════════════════════════════════════════════════════════
"""

import re
import sys
import random
import threading
from http.server import HTTPServer, BaseHTTPRequestHandler
from pathlib import Path
from urllib.parse import urlparse

# ╔════════════════════════════════════════════════════════════════════════╗
# ║                      KRAMER QUOTE DATABASE                            ║
# ╚════════════════════════════════════════════════════════════════════════╝

KRAMER_QUOTES = [
    "Giddy up!",
    "These pretzels are making me thirsty!",
    "I'm out there Jerry and I'm loving every minute of it!",
    "Master of your domain!",
    "I'm a freak!",
    "The bro! The manssiere!",
    "I'm making a salad.",
    "Coffee's for closers!",
    "Let's go to the diner.",
    "Hellooo!",
    "I'm sliding in!",
    "You know, we're living in a society!",
    "That's gold Jerry, gold!",
    "Hello Newman!",
    "Serenity now!",
    "Not that there's anything wrong with that!",
    "Double dip!",
    "It's the little things.",
    "Yada yada yada.",
    "Moops!",
    "I'm back, baby!",
]

# ╔════════════════════════════════════════════════════════════════════════╗
# ║                         TOKEN DEFINITIONS                             ║
# ╚════════════════════════════════════════════════════════════════════════╝

TOKEN_PATTERNS = [
    (r'YO\s+JERRY!', 'START'),
    (r'NOINE!', 'END'),
    (r'GIDDYUP!', 'GIDDYUP'),
    (r'SERVE\s+(\d+)\s+("(?:\\.|[^"\\])*")', 'SERVE'),
    (r'SERVE\s+(\d+)', 'SERVE_EMPTY'),
    (r'ROUTE\s+("(?:\\.|[^"\\])*")\s*\{', 'ROUTE_START'),
    (r'\{', 'LBRACE'),
    (r'\}', 'RBRACE'),
    (r'"(?:\\.|[^"\\])*"\s+LOG', 'LOG'),
    (r'SLIDE_IN', 'SLIDE_IN'),
    (r'SERVER\s+("(?:\\.|[^"\\])*")\s*\{', 'SERVER_START'),
]

class KramerToken:
    def __init__(self, type_, value):
        self.type = type_
        self.value = value
    
    def __repr__(self):
        return f"Token({self.type}, {self.value!r})"

# ╔════════════════════════════════════════════════════════════════════════╗
# ║                            LEXER                                       ║
# ╚════════════════════════════════════════════════════════════════════════╝

def tokenize(source):
    """Lex KramerScript source code into tokens."""
    tokens = []
    pos = 0
    
    while pos < len(source):
        # Skip whitespace
        match = re.match(r'\s+', source[pos:])
        if match:
            pos += len(match.group(0))
            continue
        
        matched = False
        
        # Try to match patterns (uppercase for matching but preserve strings)
        source_upper = source[pos:].upper()
        
        # Match patterns
        for pattern, token_type in TOKEN_PATTERNS:
            match = re.match(pattern, source_upper)
            if match:
                matched = True
                matched_len = len(match.group(0))
                
                # Extract the actual value from original source (preserving case)
                value = source[pos:pos + matched_len]
                
                tokens.append(KramerToken(token_type, value))
                pos += matched_len
                break
        
        if not matched:
            pos += 1
    
    return tokens

# ╔════════════════════════════════════════════════════════════════════════╗
# ║                          PARSER & INTERPRETER                         ║
# ╚════════════════════════════════════════════════════════════════════════╝

class KramerInterpreter:
    def __init__(self):
        self.routes = {}
        self.server_config = None
        self.logs = []
    
    def parse(self, tokens):
        """Parse tokens into a program structure."""
        if not tokens or tokens[0].type != 'START':
            raise SyntaxError("Program must start with 'YO JERRY!'")
        
        if not any(t.type == 'END' for t in tokens):
            raise SyntaxError("Program must end with 'NOINE!'")
        
        i = 1
        while i < len(tokens) and tokens[i].type != 'END':
            token = tokens[i]
            
            if token.type == 'SERVER_START':
                host_port = self._extract_string(token.value)
                i = self._parse_server_block(tokens, i + 1, host_port)
            elif token.type == 'LOG':
                message = self._extract_log_string(token.value)
                self.logs.append(message)
                i += 1
            elif token.type == 'SLIDE_IN':
                # Async context - flavor only
                i += 1
            else:
                i += 1
        
        return True
    
    def _parse_server_block(self, tokens, start_idx, host_port):
        """Parse SERVER { ROUTE ... } block."""
        self.server_config = host_port
        i = start_idx
        depth = 1  # We're inside one server block
        
        while i < len(tokens) and depth > 0:
            if tokens[i].type == 'LBRACE':
                depth += 1
                i += 1
            elif tokens[i].type == 'RBRACE':
                depth -= 1
                if depth == 0:
                    break
                i += 1
            elif tokens[i].type == 'ROUTE_START':
                path = self._extract_string(tokens[i].value)
                i = self._parse_route(tokens, i + 1, path)
            else:
                i += 1
        
        return i + 1 if i < len(tokens) else i
    
    def _parse_route(self, tokens, start_idx, path):
        """Parse ROUTE "/path" { SERVE ... } block."""
        i = start_idx
        route_handlers = []
        depth = 1  # We're inside one route block
        
        while i < len(tokens) and depth > 0:
            if tokens[i].type == 'LBRACE':
                depth += 1
                i += 1
            elif tokens[i].type == 'RBRACE':
                depth -= 1
                if depth == 0:
                    break
                i += 1
            elif tokens[i].type == 'SERVE':
                # SERVE token contains: SERVE <status> "<body>"
                match = re.search(r'SERVE\s+(\d+)\s+"([^"]*)"', tokens[i].value)
                if match:
                    status = int(match.group(1))
                    body = match.group(2)
                    route_handlers.append((status, body))
                i += 1
            elif tokens[i].type == 'SERVE_EMPTY':
                # SERVE token contains: SERVE <status>
                match = re.search(r'SERVE\s+(\d+)', tokens[i].value)
                if match:
                    status = int(match.group(1))
                    route_handlers.append((status, ""))
                i += 1
            else:
                i += 1
        
        if route_handlers:
            self.routes[path] = route_handlers[-1]  # Last SERVE wins
        
        return i + 1 if i < len(tokens) else i
    
    def _extract_string(self, text):
        """Extract quoted string from token value."""
        match = re.search(r'"([^"]*)"', text)
        return match.group(1) if match else ""
    
    def _extract_log_string(self, text):
        """Extract string from LOG token."""
        match = re.search(r'"([^"]*)"', text)
        return match.group(1) if match else ""

# ╔════════════════════════════════════════════════════════════════════════╗
# ║                       HTTP REQUEST HANDLER                            ║
# ╚════════════════════════════════════════════════════════════════════════╝

class KramerHTTPHandler(BaseHTTPRequestHandler):
    """HTTP request handler for KramerScript routes."""
    
    interpreter = None
    docs_path = None
    
    def do_GET(self):
        """Handle GET requests."""
        path = urlparse(self.path).path
        
        # Check registered routes
        if path in self.interpreter.routes:
            status, body = self.interpreter.routes[path]
            self.send_response(status)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            self.wfile.write(body.encode())
            return
        
        # Try to serve from docs folder
        if self.docs_path and self.docs_path.exists():
            file_path = (self.docs_path / path.lstrip('/')).resolve()
            
            # Security: ensure path is within docs folder
            try:
                file_path.relative_to(self.docs_path)
            except ValueError:
                self.send_response(403)
                self.end_headers()
                return
            
            if file_path.is_file():
                try:
                    with open(file_path, 'rb') as f:
                        self.send_response(200)
                        self.send_header('Content-type', 'text/html')
                        self.end_headers()
                        self.wfile.write(f.read())
                    return
                except Exception:
                    pass
        
        # 404
        self.send_response(404)
        self.send_header('Content-type', 'text/plain')
        self.end_headers()
        self.wfile.write(b"Not found!")
    
    def log_message(self, format, *args):
        """Suppress default logging."""
        pass

# ╔════════════════════════════════════════════════════════════════════════╗
# ║                         MAIN EXECUTION                                ║
# ╚════════════════════════════════════════════════════════════════════════╝

def run_kramer_script(filename):
    """Execute a .kramer script file."""
    try:
        with open(filename, 'r') as f:
            source = f.read()
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found, you dummy!")
        sys.exit(1)
    
    # Tokenize and parse
    tokens = tokenize(source)
    interpreter = KramerInterpreter()
    interpreter.parse(tokens)
    
    # Print logs
    for log_msg in interpreter.logs:
        print(log_msg)
    
    # Start server if configured
    if interpreter.server_config:
        print("These pretzels are making me thirsty!")
        host, port = interpreter.server_config.split(':')
        port = int(port)
        
        KramerHTTPHandler.interpreter = interpreter
        KramerHTTPHandler.docs_path = Path('./docs')
        
        server = HTTPServer((host, port), KramerHTTPHandler)
        print(f"Giddy up! Server running on {host}:{port}")
        
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            print("\nNOINE!")
            server.shutdown()

def main():
    """Main entry point."""
    if len(sys.argv) < 2:
        print(random.choice(KRAMER_QUOTES))
        sys.exit(0)
    
    kramer_file = sys.argv[1]
    if not kramer_file.endswith('.kramer'):
        print("These aren't Kramer files, Jerry!")
        sys.exit(1)
    
    run_kramer_script(kramer_file)

if __name__ == '__main__':
    main()


# ╔════════════════════════════════════════════════════════════════════════╗
# ║                   README: KramerScript                                ║
# ╚════════════════════════════════════════════════════════════════════════╝

README = """
╔═════════════════════════════════════════════════════════════════════════════╗
║                                                                             ║
║                 KramerScript – A Language About Nothing™                    ║
║                                                                             ║
║  "I'm out there Jerry, and I'm loving every minute of it!"                 ║
║                                                                             ║
╚═════════════════════════════════════════════════════════════════════════════╝

WHAT IS KramerScript?
═════════════════════════════════════════════════════════════════════════════

KramerScript is an esoteric programming language where the ONLY valid syntax
is Seinfeld/Kramer quotes. It compiles to HTTP servers that serve static
content. Because what's a programming language without a little pizzazz?

Why? Because society!


INSTALLATION & USAGE
═════════════════════════════════════════════════════════════════════════════

    $ python kramerscript.py server.kramer

If you run it without arguments, you get a random Kramer quote. Better than
a lot of programming languages!

    $ python kramerscript.py
    > Master of your domain!


SYNTAX REFERENCE
═════════════════════════════════════════════════════════════════════════════

YO JERRY!              ← Start your program (mandatory)
NOINE!                 ← End your program (mandatory)
GIDDYUP!               ← Return/exit function
SERVER "host:port" {   ← Start server on host:port
  ...
}
ROUTE "/path" {        ← Define HTTP route
  SERVE status "body"  ← Send response
}
"quote" LOG            ← Print to console
SLIDE_IN               ← Async context (flavor only)


HILARIOUS EXAMPLE PROGRAMS
═════════════════════════════════════════════════════════════════════════════

#1: The Classic Nothing
────────────────────────
YO JERRY!
"These pretzels are making me thirsty!" LOG
NOINE!

Result: Prints a complaint about pretzels.


#2: Master of the Domain (Port 9000)
────────────────────────────────────
YO JERRY!
SERVER "0.0.0.0:9000" {
    ROUTE "/" {
        SERVE 200 "Master of your domain!"
    }
}
NOINE!

Result: HTTP server that enforces domain mastery.


#3: The Bro
────────────
YO JERRY!
"I'm making a salad." LOG
SERVER "localhost:8080" {
    ROUTE "/bro" {
        SERVE 200 "The bro! The manssiere!"
    }
    ROUTE "/salad" {
        SERVE 418 "The salad is not a meal!"
    }
}
NOINE!

Result: A salad-themed web service. You know, we're living in a society!


#4: Hello Newman!
─────────────────
YO JERRY!
SLIDE_IN
SERVER "127.0.0.1:413" {
    ROUTE "/newman" {
        SERVE 200 "Hello Newman!"
    }
}
"I'm back baby!" LOG
NOINE!

Result: The asynchronous Newman HTTP endpoint.


#5: The Double Dip
───────────────────
YO JERRY!
SERVER "0.0.0.0:5000" {
    ROUTE "/" {
        SERVE 200 "Double dip!"
    }
    ROUTE "/dip" {
        SERVE 200 "You can't dip twice!"
    }
}
NOINE!

Result: A dipping-themed REST API nobody asked for.


#6: Serenity Now!
──────────────────
YO JERRY!
"Serenity now!" LOG
"These pretzels are making me thirsty!" LOG
NOINE!

Result: Contradictory emotions in standard output.


#7: Moops!
──────────
YO JERRY!
SERVER "0.0.0.0:80" {
    ROUTE "/" {
        SERVE 200 "Moops!"
    }
    ROUTE "/history" {
        SERVE 200 "It's actually Moops!"
    }
}
NOINE!

Result: A historically accurate web server.


#8: The Marble Rye
────────────────────
YO JERRY!
SLIDE_IN
"Not that there's anything wrong with that!" LOG
SERVER "localhost:3000" {
    ROUTE "/rye" {
        SERVE 200 "Where's the marble rye?"
    }
}
NOINE!

Result: A bread-focused web service with async flavor.


#9: Gold, Jerry!
─────────────────
YO JERRY!
"That's gold Jerry, gold!" LOG
SERVER "0.0.0.0:4040" {
    ROUTE "/gold" {
        SERVE 200 "Pure gold!"
    }
    ROUTE "/gimme" {
        SERVE 403 "You can't have the gold!"
    }
}
NOINE!

Result: A precious metals web server.


#10: The Yada Yada Protocol
────────────────────────────
YO JERRY!
"Yada yada yada." LOG
SLIDE_IN
SERVER "127.0.0.1:8888" {
    ROUTE "/important" {
        SERVE 200 "Yada yada yada"
    }
    ROUTE "/" {
        SERVE 200 "Yada yada yada"
    }
}
"Yada yada yada!" LOG
NOINE!

Result: A web service that abbreviates everything. Details!


#11: The Festivus Surprise
────────────────────────────
YO JERRY!
"I'm out there Jerry, and I'm loving every minute of it!" LOG
"The feats of strength!" LOG
SLIDE_IN
SERVER "0.0.0.0:666" {
    ROUTE "/festivus" {
        SERVE 200 "Festivus for the rest of us!"
    }
}
NOINE!

Result: A holiday-themed web server that's not a holiday.


TECHNICAL DETAILS
═════════════════════════════════════════════════════════════════════════════

• Language: Python 3 (stdlib only – no dependencies!)
• File Extension: .kramer (case-insensitive)
• Server Backend: Python's http.server
• Static Files: Served from ./docs/ folder
• Port Range: Any valid port (but 413 is more fun)
• Performance: Terrible
• Fun Factor: 11/10


WHAT YOU CAN'T DO
═════════════════════════════════════════════════════════════════════════════

• Variables (use your memory!)
• Functions (define once, quote forever!)
• Loops (that's what recursive HTTP requests are for!)
• Conditionals (all routes execute always!)
• Comments (everything IS a comment in KramerScript!)
• Comments (seriously, we can't find them)
• Error handling (just slide in and hope!)
• Type safety (types are for closers!)


FORTUNE COOKIE WISDOM
═════════════════════════════════════════════════════════════════════════════

"The man in the coffee shop said, 'Giddy up!' and the web server was born."
                                                        - Confuscius, probably

"Not that there's anything wrong with esoteric languages!" 
                                                        - Perfectly reasonable

"These pretzels are making me thirsty!"
                                                        - The whole point


Happy scripting! Now get out!
"""

# Uncomment to print README:
# print(README)
