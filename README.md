# Text File Comparator (IFCOMP)

File Comparator program IFCOMP is a sophisticated text file comparator originally designed for IBM OS/VS compatible systems. IFCOMP accepts as input two text files and produces a listing of their differences in pseudo-update form, making it highly useful for monitoring changes made to software at the source code level.

## History

IFCOMP was originally written by Reed Kotler in the XPL programming language for an IBM 370-series computer in 1979. The implementation is based on a file comparison algorithm described in the NASA Tech Brief publication below. Later, Tom Penello converted it from XPL to MetaWare High C.

```
Document ID             19820000365
Document Type           Other - NASA Tech Brief
Authors                 Kotler, R. S. (Intermetrics, Inc.)
Date Acquired           August 10, 2013
Publication Date        May 1, 1983
Publication Information Publication: NASA Tech Briefs
                        Volume: 7
                        Issue: 2
                        ISSN: 0145-319X
Subject Category        MATHEMATICS AND INFORMATION SCIENCES
Report/Patent Number    MSC-20276
Distribution Limits     Public
Copyright               Work of the US Gov. Public Use Permitted.
```

Original publication: [https://ntrs.nasa.gov/citations/19820000365](https://ntrs.nasa.gov/citations/19820000365)

## Features

- **Detects Multiple Change Types**: Identifies deletions, insertions, replacements, and movements of code blocks
- **Pseudo-Update Output**: Presents differences in a format similar to source code update commands
- **Efficient Algorithm**: Uses an 8-pass algorithm with hash-based line matching and tree structures
- **Change Statistics**: Provides summary statistics including:
  - Lines deleted from old file
  - Lines inserted in new file
  - Lines replaced
  - Lines moved
  - Number of change blocks

## Algorithm Overview

The IFCOMP algorithm performs an 8-pass comparison:

**Passes 1-4 (Initial Matching)**:
- **Pass 1**: Builds hash tables for both input files
- **Pass 2**: Identifies unique line pairs (lines that appear once in each file)
- **Pass 3**: Extends matches forward from unique pairs
- **Pass 4**: Extends matches backward from unique pairs

**Passes 5-8 (Tree Building)**:
- **Pass 5**: Identifies lines that don't match and builds initial trees
- **Pass 6**: Handles deletions in old file
- **Pass 7**: Handles insertions and replacements
- **Pass 8**: Finalizes output formatting

The algorithm uses a sophisticated hash function to quickly identify matching and unique lines across both files, enabling efficient detection of moved code blocks.

## Usage

### Basic Usage

Compare two files:
```bash
$ ifcomp oldfile.txt newfile.txt
```

The program will produce output showing differences in pseudo-update form, including:
- Deleted lines from the old file
- Inserted lines in the new file  
- Replaced blocks (old lines → new lines)
- Moved blocks
- Summary statistics

### Command Line Options

- `-stat`: Print detailed memory usage statistics
- `-debug`: Enable all debug output modes
- `-st`: Enable symbol table debugging
- `-stfull`: Enable full symbol table debugging
- `-trees`: Enable tree structure debugging
- `-treesfull`: Enable full tree structure debugging
- `-alloc`: Enable memory allocation debugging
- `-nofree`: Disable memory freeing (for debugging)

### Example Output

```
Comparing: file1.txt file2.txt

*** AFTER LINE(s) ======================================= ***
      3|C
*** DELETE LINE(s) -------------------------------------- ***
      4|Y
*** ===================================================== ***

*** AFTER LINE(s) ======================================= ***
      5|D
*** REPLACE LINE(s) ------------------------------------- ***
      6|W
      7|E
*** WITH LINE(s) ---------------------------------------- ***
+     5|E
*** ===================================================== ***

       4 lines deleted from old.
       0 lines inserted in new.
       2 lines deleted from old and replaced with 1 lines of new.
       2 lines moved in old.
       5 change blocks.
```

## Build

### Using Make

On Mac:
```bash
$ brew install cmocka
$ make
```

On Ubuntu:
```bash
$ sudo apt install libcmocka-dev
$ make
```

Run tests:
```bash
$ make test
```

Other Make targets:
- `make install`: Install the binary to system
- `make clean`: Remove build directory
- `make reindent`: Format code with clang-format

### Using CMake

CMake is supported with C11 and C++17 standards:

```bash
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
```

Run tests:
```bash
$ ctest --verbose
```

Or build in Debug mode:
```bash
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ cmake --build .
```

On Mac, cmocka is detected automatically via Homebrew. On Ubuntu, install `libcmocka-dev`.

### Project Structure

```
ifcomp/
├── main.c              # Main entry point and command-line parsing
├── ifcomp.c            # Core algorithm implementation (1376 lines)
├── ifcomp.h            # Header file with public API
├── unit_tests.c        # Unit tests
├── tests/              # GoogleTest-based C++ tests
│   ├── test_identical_files.cpp
│   ├── test_complex_changes.cpp
│   ├── test_permutation_changes.cpp
│   └── test_much_writing.cpp
├── rust/               # Rust implementation (experimental)
│   ├── src/
│   │   ├── main.rs
│   │   └── lib.rs
│   └── tests/
└── CMakeLists.txt      # CMake build configuration
```

## Testing

The project includes comprehensive tests using GoogleTest:

```bash
# Run all tests
$ make test

# Or with CMake
$ ctest --verbose
```

Tests cover:
- Identical files
- Simple changes (insert, delete)
- Complex changes (replace, move)
- Permutations of similar content
- Edge cases (empty files, etc.)

## Implementation

### C Implementation (Main)

The primary implementation is in C with the following key components:

- **Hash-based Line Matching**: Custom hash function for efficient line comparison
- **8-Pass Algorithm**: Multi-pass comparison for accurate change detection
- **Tree Structures**: Internal tree representation for tracking changes
- **Memory Management**: Dynamic allocation with statistics tracking

**Standards**: C11 with standard library only

**Build System**: CMake 3.15+ with optional Make wrapper

### Rust Implementation (Experimental)

A Rust version exists in the `rust/` directory using:
- `clap` for command-line argument parsing
- `hdiff` for difference computation
- Integration tests with expected output files

## License

MIT License

Copyright (c) 1979-2022 Reed Kotler

See [MIT-LICENSE](MIT-LICENSE) for full text.

## References

- NASA Tech Brief: [IFCOMP File Comparator](https://ntrs.nasa.gov/citations/19820000365)
- Algorithm: Hash-based text file comparison with 8-pass processing
- Original Language: XPL (1979)
- Port: MetaWare High C (Tom Penello)
