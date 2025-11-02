# Legacy C Implementation of IFCOMP

This directory contains the original C implementation of the IFCOMP file comparator, converted from the 1979 XPL version to C by Tom Penello.

## Overview

The legacy implementation is a faithful transliteration of the original 1979 XPL code written by Reed Kotler for IBM 370-series computers. This C version preserves the original algorithm structure and serves as the reference implementation that other ports (Rust, Go, C++) were based on.

## Files

- **`ifcomp.c`** (1377 lines) - Main algorithm implementation with all 8 passes
- **`main.c`** - Command-line interface and option parsing
- **`ifcomp.h`** - Public API headers and external declarations
- **`unit_tests.c`** - Cmocka-based unit tests
- **`Makefile`** - Build configuration

## Architecture

The implementation uses the original 8-pass algorithm:

### Pass 1: Reading and Hashing
- Reads both input files line by line
- Creates hash-based symbol table (SYT) with double-level hashing
- Uses 16-bit hash scheme (`hash_info` with `h1` and `h2`)
- Establishes line-to-string mappings

### Pass 2: Unique Line Matching
- Identifies lines occurring exactly once in both files
- Marks these as unique matches (`unique_type`)

### Pass 3: Forward Extension
- Extends matches forward from unique lines
- Marks sequences as matched (`match_type`)

### Pass 4: Backward Extension
- Extends matches backward from unique lines
- Completes match identification

### Pass 5: Tree Construction
- Builds doubly-linked trees for unmatched segments
- Creates header and trailer nodes
- Organizes changes into blocks

### Pass 6: Replace, Delete, and Insert
- Processes deletions and replacements
- Handles insertions
- Combines nodes when appropriate

### Pass 7: Adjacent Node Combination
- Combines adjacent nodes that match between files
- Optimizes tree structure

### Pass 8: Move Detection
- Identifies and processes moved blocks
- Reorganizes tree structure

## Data Structures

### Hash-Based Symbol Table
```c
typedef struct {
    unsigned short h1;  // Primary hash (16-bit)
    long h2;            // Secondary hash (32-bit)
} hash_info;
```

### String Table (SYT)
Double-level hashing with buckets and collision chains:
- 256 buckets for primary hashing
- Each hash node contains hash info and string list
- String entries track occurrences in both files

### File Line Arrays
```c
typedef struct {
    line_count ptr0;
    string_index file_line_text;
    line_count linen;
    line_type ptr_type;
} file_line_decl;
```

### Tree Nodes
```c
typedef struct {
    line_count cost;
    line_count linen;
    tree_index prev, next, branch_start, branch_end;
} node_decl;
```

## Key Features

### Double-Level Hashing
The implementation uses a sophisticated double-level hashing scheme to minimize collisions:
1. Primary hash (`h1`): Length and XOR value (16-bit)
2. Secondary hash (`h2`): Character pair-based bitmap (32-bit)

### Dynamic Memory Allocation
Arrays grow automatically when full:
- `next_index()` macro allocates and expands arrays as needed
- Uses `realloc()` to double array size
- Zero-copy string storage in pooled buffers

### Debug Flags
Comprehensive debugging support:
- `-st`: Symbol table debugging
- `-stfull`: Full symbol table debugging
- `-trees`: Tree structure debugging
- `-treesfull`: Full tree structure debugging
- `-alloc`: Memory allocation debugging
- `-nofree`: Disable memory freeing (for debugging)
- `-debug`: Enable all debug modes

## Building

### Requirements
- C compiler (GCC or Clang)
- Cmocka for unit tests

### Mac
```bash
brew install cmocka
make
make test
```

### Ubuntu
```bash
sudo apt install libcmocka-dev
make
make test
```

### Make Targets
- `make` - Build the `ifcomp` binary
- `make clean` - Remove build artifacts
- `make test` - Run unit tests

## Usage

### Basic Comparison
```bash
./ifcomp file1.txt file2.txt
```

### With Statistics
```bash
./ifcomp -stat file1.txt file2.txt
```

### Debug Output
```bash
./ifcomp -debug file1.txt file2.txt
```

### Command-Line Options
- `-stat`: Print memory usage statistics
- `-debug`: Enable all debug output
- `-st`: Enable symbol table debugging
- `-stfull`: Enable full symbol table debugging
- `-trees`: Enable tree debugging
- `-treesfull`: Enable full tree debugging
- `-alloc`: Enable allocation debugging
- `-nofree`: Disable memory freeing

## Algorithm Characteristics

### Strengths
- Efficient hash-based line matching
- Detects all change types: insert, delete, replace, move
- Handles large files with dynamic memory allocation
- Comprehensive debugging support

### Limitations
**Duplicate Line Handling**: Files containing only duplicate lines (e.g., 100 occurrences of "LINE") without unique anchor lines will be reported as replacements rather than matches. This occurs because:
1. Pass 2 requires unique lines (occurring once in each file) as anchors
2. Subsequent passes can only extend matches from these anchors
3. Without unique anchors, duplicate-only sections cannot establish matches

This behavior matches the original 1979 implementation.

## Ports

This C implementation was used as the reference for:
- **Rust** (`rust/`) - Modern Rust port with full test coverage
- **Go** (`go/`) - Go port maintaining algorithm fidelity
- **C++** (`ifcomp.cpp`, `pass1.cpp`-`pass8.cpp`) - Modern C++ refactoring

## Testing

The `unit_tests.c` file includes comprehensive tests using cmocka:
- Identical files
- Complex changes (deletes, moves, replacements)
- Permutation changes
- Much writing example from algorithm documentation

## License

MIT License - Copyright (c) 1979-2022 Reed Kotler

## References

- Original XPL implementation (1979) by Reed Kotler
- Port to C by Tom Penello
- NASA Tech Brief: [https://ntrs.nasa.gov/citations/19820000365](https://ntrs.nasa.gov/citations/19820000365)
- See `../Theory.md` for detailed algorithm documentation

