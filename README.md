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

```bash
$ make
```

Run tests:
```bash
$ make test
# Or with CMake
$ ctest --test-dir build/tests
```

Other Make targets:
- `make install`: Install the binary to system
- `make clean`: Remove build directory
- `make reindent`: Format code with clang-format

### Project Structure

```
ifcomp/
├── main.cpp            # Main entry point and command-line parsing
├── ifcomp.cpp          # Core algorithm implementation
├── ifcomp.h            # Header file with public API
├── pass1.cpp - pass8.cpp  # Algorithm pass implementations
├── Makefile            # Make build configuration
├── CMakeLists.txt      # CMake build configuration
├── Theory.md           # Detailed algorithm theory and documentation
├── calculate_coverage.py  # Test coverage calculation script
├── tests/              # GoogleTest-based C++ tests
│   ├── CMakeLists.txt
│   ├── test_helpers.h
│   ├── ifcomp_driver.h
│   ├── test_identical_files.cpp
│   ├── test_complex_changes.cpp
│   ├── test_permutation_changes.cpp
│   ├── test_much_writing.cpp
│   ├── test_cli.cpp
│   ├── test_file_io_errors.cpp
│   ├── test_pass1.cpp - test_pass8.cpp
│   └── test_*.cpp      # Additional test files
├── rust/               # Rust implementation
│   ├── Cargo.toml
│   ├── src/
│   │   ├── main.rs
│   │   ├── lib.rs
│   │   ├── types.rs
│   │   └── pass1.rs - pass8.rs
│   ├── tests/
│   │   ├── cli.rs
│   │   ├── integration_test.rs
│   │   └── pass1_test.rs - pass8_test.rs
│   └── README.md
├── go/                 # Go implementation
│   ├── go.mod
│   ├── main.go
│   ├── types.go
│   ├── pass1.go - pass8.go
│   ├── pass1_test.go - pass8_test.go
│   ├── ifcomp_test.go
│   └── README.md
├── legacy/             # Original C implementation
│   ├── main.c
│   ├── ifcomp.c
│   ├── ifcomp.h
│   ├── unit_tests.c
│   ├── Makefile
│   └── README.md
└── MIT-LICENSE         # License file
```

## Implementation

The primary implementation is in C++ (originally C), with a Rust version in the `rust/` directory and a Go implementation in the `go/` directory. See [Theory.md](Theory.md) for implementation details.

### Available Implementations

- **C++**: Primary implementation (modern C++ refactoring)
- **Rust**: Rust language port in `rust/` - see [rust/README.md](rust/README.md) for details
- **Go**: Go language port in `go/` - see [go/README.md](go/README.md) for details
- **C**: Original C code, ported from XPL

## License

MIT License

Copyright (c) 1979-2022 Reed Kotler

See [MIT-LICENSE](MIT-LICENSE) for full text.

## References

- [NASA Tech Brief](https://ntrs.nasa.gov/citations/19820000365): Original algorithm description
- [Theory.md](Theory.md): Detailed algorithm documentation
- Original implementation: XPL (1979) by Reed Kotler
- Port: MetaWare High C by Tom Penello
