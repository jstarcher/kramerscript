# KramerScript – A Language About Nothing™

> *"I'm out there Jerry, and I'm loving every minute of it!"*

KramerScript is an esoteric programming language where **every keyword is a Seinfeld/Kramer quote**. Written in C, it compiles to a single native executable that runs functional HTTP servers. Because what's a programming language without pizzazz?

## Overview

- **Language Name:** KramerScript
- **File Extension:** `.kramer`
- **Implementation:** Pure C (POSIX sockets, pthreads)
- **Binary Size:** ~24KB native executable
- **Purpose:** Define HTTP servers using only Seinfeld quotes
- **Status:** 100% functional and hilariously themed
- **Platform:** macOS, Linux, Unix

## Quick Start

```bash
# Compile the C implementation
make

# Run the interpreter with no arguments for a Kramer quote
./kramerscript

# Run a .kramer program
./kramerscript server.kramer
```

## Compilation

```bash
# Build the native binary
make

# Clean build
make clean

# Optional: Install to /usr/local/bin
make install

# Test the server
make test
```

## Example Program

```kramer
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
```

When you run this:
1. The server starts on `0.0.0.0:413`
2. `GET /` returns 200 with Kramer's life philosophy
3. `GET /health` returns 200 with Kramer's enthusiasm
4. Both messages print to console
5. The message "These pretzels are making me thirsty!" prints on startup
5. Any undefined routes serve from `./docs/` or return 404

## Syntax Reference

| Syntax | Meaning |
|--------|---------|
| `YO JERRY!` | Program start (required) |
| `NOINE!` | Program end (required) |
| `SERVER "host:port" { ... }` | Define HTTP server |
| `ROUTE "/path" { ... }` | Define route handler |
| `SERVE status "body"` | Send HTTP response |
| `"quote" LOG` | Print to console |
| `SLIDE_IN` | Async context (flavor only) |
| `GIDDYUP!` | Return/exit (reserved for future) |
| `BIT "name" { ... }` | Define a reusable "bit" (function-like block) |

## Key Features

✅ **Real HTTP Server** – Actually handles requests using `http.server`  
✅ **Static File Serving** – Serves files from `./docs/` folder  
✅ **Multiple Routes** – Define as many routes as you want  
✅ **Logging** – Print messages to console during execution  
✅ **No Dependencies** – Uses only Python stdlib  
✅ **Case-Insensitive Keywords** – Keywords work in any case  
✅ **Seinfeld Accurate** – Every keyword is a real quote or catchphrase  

## How It Works

1. **Lexer** – Tokenizes `.kramer` files, recognizing Seinfeld quotes as keywords
2. **Parser** – Builds a program structure from tokens
3. **Interpreter** – Executes by setting up routes and HTTP handlers
4. **Server** – Runs an HTTP server using `http.server.HTTPServer`

## Architecture

```
kramerscript.py (620 lines)
├── Lexer (tokenize)
├── Parser (KramerInterpreter)
├── HTTP Handler (KramerHTTPHandler)
├── Quote Database (KRAMER_QUOTES)
├── Full README
└── Example program (in docstring)
```

## Example Programs

### 1. Classic "Nothing"
```kramer
YO JERRY!
"These pretzels are making me thirsty!" LOG
NOINE!
```

### 2. Health Check Service
```kramer
YO JERRY!
SERVER "localhost:8080" {
    ROUTE "/health" {
        SERVE 200 "Giddy up!"
    }
}
NOINE!
```

### 3. The Domain Master
```kramer
YO JERRY!
SERVER "0.0.0.0:9000" {
    ROUTE "/" {
        SERVE 200 "Master of your domain!"
    }
}
NOINE!
```

### 4. Multiple Routes
```kramer
YO JERRY!

SERVER "127.0.0.1:3000" {
    ROUTE "/" {
        SERVE 200 "Hello!"
    }
    
    ROUTE "/newman" {
        SERVE 200 "Hello Newman!"
    }
    
    ROUTE "/fold" {
        SERVE 418 "I'm not a folder!"
    }
}

"These pretzels are making me thirsty!" LOG

NOINE!
```

### 5. Functions ("bits")
```kramer
YO JERRY!

BIT "greeting" {
Hello from the bit!
}

BIT "footer" {
---
The End.
}

SERVER "127.0.0.1:9413" {
    ROUTE "/" {
        SERVE 200 "Welcome! {{greeting}} {{footer}}"
    }
}

NOINE!
```

## Testing

The repository includes `test_kramerscript.py` which verifies:
- ✅ Tokenization
- ✅ Parsing
- ✅ Route registration
- ✅ Server configuration
- ✅ Console logging
- ✅ Response body handling

Run tests with:
```bash
python3 test_kramerscript.py
```

## File Structure

```
kramerscript/
├── kramerscript.py          # Main interpreter (620 lines)
├── server.kramer            # Example program
├── test_kramerscript.py     # Test suite
├── debug_tokens.py          # Token debugging utility
├── debug_extract.py         # Extraction debugging utility
├── docs/
│   └── index.html           # Static file example
└── README.md                # This file
```

## Random Startup Quotes

When run with no arguments, the interpreter prints a random Kramer quote:

- "Giddy up!"
- "These pretzels are making me thirsty!"
- "I'm out there Jerry and I'm loving every minute of it!"
- "Master of your domain!"
- "The bro! The manssiere!"
- "Hello Newman!"
- "Serenity now!"
- ...and 14 more!

## Technical Details

- **Language:** Python 3.6+
- **Dependencies:** None (stdlib only)
- **File Size:** ~620 lines
- **Performance:** Single-threaded HTTP server
- **Concurrency:** One request at a time
- **Port Range:** Any valid port (413 recommended for Kramer vibes)

## Limitations

❌ No variables  
✅ Functions ("bits") — reusable blocks you can insert in responses  
❌ No loops  
❌ No conditionals  
❌ No type system  
❌ No comments (everything is a quote!)  

> "You know, we're living in a society! We're supposed to act in a *particular way*!" – George Costanza (applies to language design too)

## Famous Quotes Used

All keywords are either direct Kramer quotes or famous Seinfeld catchphrases:

- **YO JERRY!** → How Kramer greets Jerry at his door
- **NOINE!** → Kramer's famous sneeze sound
- **GIDDYUP!** → Kramer's enthusiastic exclamation
- **SERVE** → "Let me serve you!" (Kramer's hospitality)
- **ROUTE** → Kramer's unconventional paths
- **SLIDE_IN** → Kramer's signature entrance
- **MASTER OF YOUR DOMAIN** → The contest episode
- **SERENITY NOW** → Frank's mantra

## Why Does This Exist?

> "Why? Because society!" – Kramer

KramerScript proves that programming languages don't need to make sense to be Turing-complete... well, actually it's not Turing-complete, but it *does* run HTTP servers!

## Fun Fact

The interpreter outputs "These pretzels are making me thirsty!" when the server starts. This is 100% required and cannot be disabled because it's perfect.

---

**"Not that there's anything wrong with that!"**

Made with ❤️ and far too much Seinfeld appreciation.
