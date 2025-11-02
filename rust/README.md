# IFCOMP Rust Implementation

This is a Rust language port of the IFCOMP text file comparison algorithm originally written in 1979.

## Overview

IFCOMP is a sophisticated text file comparator that compares two text files and produces a listing of their differences in pseudo-update form. The algorithm uses an 8-pass approach based on Heckel's 1978 algorithm for isolating differences between files.

This Rust implementation provides a complete port of the original algorithm with comprehensive test coverage.

## Algorithm

The implementation performs 8 passes:

1. **Pass 1**: Hash table construction - reads both files and builds hash-based data structures using FNV hashing
2. **Pass 2**: Unique pair identification - matches lines that appear exactly once in each file
3. **Pass 3**: Forward match extension - extends matches forward from unique anchors
4. **Pass 4**: Backward match extension - extends matches backward from unique anchors
5. **Pass 5**: Tree construction - builds tree structures representing file segments
6. **Pass 6**: Replace/Delete/Insert - identifies and outputs changes
7. **Pass 7**: Combine adjacent nodes - merges adjacent nodes for better representation
8. **Pass 8**: Move detection - detects and processes moved code blocks

## Building

### Prerequisites

Ensure you have Rust installed (1.70 or later recommended):

```bash
rustc --version
cargo --version
```

### Build the Binary

```bash
cd rust
cargo build --release
```

This will create the `ifcomp` binary in `target/release/ifcomp`.

### Build Development Version

```bash
cargo build
```

Binary will be in `target/debug/ifcomp`.

## Running

```bash
cargo run -- file1.txt file2.txt
```

Or use the installed binary:

```bash
./target/release/ifcomp file1.txt file2.txt
```

### Command Line Options

- `--stat` or `-s`: Print detailed memory usage statistics
- `--debug` or `-d`: Enable all debug output modes
- `--st`: Enable symbol table debugging
- `--stfull`: Enable full symbol table debugging
- `--trees`: Enable tree structure debugging
- `--treesfull`: Enable full tree structure debugging
- `--alloc`: Enable memory allocation debugging
- `--nofree`: Disable memory freeing (for debugging)

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

## Testing

The Rust implementation includes comprehensive test coverage:

### Run All Tests

```bash
cargo test
```

### Run Integration Tests

```bash
cargo test --test integration_test
```

The integration tests port all 73 test cases from the Go implementation, covering:

- Basic file comparisons
- Duplicate line handling
- Whitespace variations
- Special characters and UTF-8
- Long lines (up to 4096+ bytes)
- Large files (up to 5000+ lines)
- Complex change patterns (deletions, insertions, moves, replacements)
- Edge cases and boundary conditions

### Run Individual Pass Tests

```bash
cargo test --test pass1_test
cargo test --test pass2_test
# ... etc for pass3 through pass8
```

### Run CLI Tests

```bash
cargo test --test cli
```

## Implementation Details

The Rust implementation closely follows the C++ version's structure with Rust-specific improvements:

### Source Structure

- **src/lib.rs**: Main library interface, `Ifcomp` struct, and orchestration
- **src/types.rs**: Core data structures and types
- **src/main.rs**: Command-line entry point
- **src/pass1.rs**: Hash table construction and file reading using FNV hasher
- **src/pass2.rs**: Unique pair identification
- **src/pass3.rs**: Forward match extension
- **src/pass4.rs**: Backward match extension
- **src/pass5.rs**: Tree construction
- **src/pass6.rs**: Replace/delete/insert operations and summary output
- **src/pass7.rs**: Adjacent node combination
- **src/pass8.rs**: Move detection and processing

### Test Structure

- **tests/integration_test.rs**: Comprehensive integration tests ported from Go (73 tests)
- **tests/cli.rs**: CLI argument parsing tests
- **tests/pass1_test.rs** through **tests/pass8_test.rs**: Individual pass unit tests

### Dependencies

- **clap v2**: Command-line argument parsing
- **fnv v1.0**: Fowler-Noll-Vo hash function for fast hashing

### Development Dependencies

- **assert_cmd**: Testing CLI commands
- **predicates**: Test assertions
- **rand**: Random test data generation
- **tempfile**: Temporary file handling for tests
- **regex**: Statistics parsing in integration tests

## Features

- Complete 8-pass algorithm implementation
- Memory-safe Rust code with no unsafe blocks
- Comprehensive test suite (73+ integration tests)
- Full command-line compatibility with C++ version
- Efficient FNV hash-based line matching
- UTF-8 and special character support
- Large file handling (tested up to 5000+ lines)

## Limitations

The IFCOMP algorithm has a fundamental limitation when dealing with duplicate lines:

**Duplicate Line Handling**: Files that contain only duplicate lines (e.g., 100 occurrences of "LINE") without unique anchor lines will be reported as replacements rather than matches. This is because the algorithm requires unique lines as anchors to establish matches in Pass 2, and subsequent passes can only extend matches from these anchors.

This behavior matches the original 1979 implementation and is documented in [Theory.md](../Theory.md).

## License

MIT License

Copyright (c) 1979-2022 Reed Kotler

See the main project [MIT-LICENSE](../MIT-LICENSE) for full text.

## References

- [NASA Tech Brief](https://ntrs.nasa.gov/citations/19820000365): Original algorithm description
- [Theory.md](../Theory.md): Detailed algorithm documentation
- Main project [README](../README.md): General IFCOMP information
- Original implementation: XPL (1979) by Reed Kotler
- Port: MetaWare High C by Tom Penello
