# Text File Comparator (IFCOMP)

**C++ Test Coverage: 91.7% (360 tests)**

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

- Detects deletions, insertions, replacements, and moves
- Pseudo-update output format
- Hash-based matching for efficiency
- Change statistics summary

## Limitations

The IFCOMP algorithm has a fundamental limitation when dealing with duplicate lines:

**Duplicate Line Handling**: Files that contain only duplicate lines (e.g., 100 occurrences of "LINE") without unique anchor lines will be reported as replacements rather than matches. This is because the algorithm requires unique lines as anchors to establish matches in Pass 2, and subsequent passes can only extend matches from these anchors.

This behavior matches the original 1979 implementation and is documented in [Theory.md](Theory.md).

## Algorithm Overview

IFCOMP uses an 8-pass algorithm with hash-based line matching and tree structures to identify deletions, insertions, replacements, and moves between two files. For detailed algorithm theory, data structures, and implementation details for each pass, see [Theory.md](Theory.md).

## Usage

### Basic Usage

Compare two files:
```bash
$ ifcomp oldfile.txt newfile.txt
```

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
├── Theory.md           # Detailed algorithm theory and documentation
├── tests/              # GoogleTest-based C++ tests
│   ├── test_identical_files.cpp
│   ├── test_complex_changes.cpp
│   ├── test_permutation_changes.cpp
│   └── test_much_writing.cpp
├── rust/               # Rust implementation
│   ├── src/
│   │   ├── main.rs
│   │   └── lib.rs
│   ├── tests/
│   └── README.md
├── go/                 # Go implementation
│   ├── main.go
│   ├── types.go
│   ├── pass1.go - pass8.go
│   ├── ifcomp_test.go
│   └── README.md
└── CMakeLists.txt      # CMake build configuration
```

## Testing

Run tests:
```bash
$ make test
# Or with CMake
$ ctest --verbose
```

## Implementation

The primary implementation is in C++ (originally C), with a Rust version in the `rust/` directory and a Go implementation in the `go/` directory. See [Theory.md](Theory.md) for implementation details.

### Available Implementations

- **C++**: Primary implementation (original C code, modern C++ refactoring)
- **Rust**: Rust language port in `rust/` - see [rust/README.md](rust/README.md) for details
- **Go**: Go language port in `go/` - see [go/README.md](go/README.md) for details

## License

MIT License

Copyright (c) 1979-2022 Reed Kotler

See [MIT-LICENSE](MIT-LICENSE) for full text.

## References

- [NASA Tech Brief](https://ntrs.nasa.gov/citations/19820000365): Original algorithm description
- [Theory.md](Theory.md): Detailed algorithm documentation
- Original implementation: XPL (1979) by Reed Kotler
- Port: MetaWare High C by Tom Penello
