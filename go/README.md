# IFCOMP Go Implementation

This is a Go language port of the IFCOMP text file comparison algorithm originally written in 1979.

## Overview

IFCOMP is a sophisticated text file comparator that compares two text files and produces a listing of their differences in pseudo-update form. The algorithm uses an 8-pass approach based on Heckel's 1978 algorithm for isolating differences between files.

## Algorithm

The implementation performs 8 passes:

1. **Pass 1**: Hash table construction - reads both files and builds hash-based data structures
2. **Pass 2**: Unique pair identification - matches lines that appear exactly once in each file
3. **Pass 3**: Forward match extension - extends matches forward from unique anchors
4. **Pass 4**: Backward match extension - extends matches backward from unique anchors
5. **Pass 5**: Tree construction - builds tree structures representing file segments
6. **Pass 6**: Replace/Delete/Insert - identifies and outputs changes
7. **Pass 7**: Combine adjacent nodes - merges adjacent nodes for better representation
8. **Pass 8**: Move detection - detects and processes moved code blocks

## Building

```bash
go build
```

This will create the `ifcomp` binary.

## Running

```bash
./ifcomp file1.txt file2.txt
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
      1|A
*** DELETE LINE(s) -------------------------------------- ***
      2|B
*** ===================================================== ***

*** AFTER LINE(s) ======================================= ***
+     2|C
*** INSERT LINE(s) -------------------------------------- ***
*** ===================================================== ***

       1 lines deleted from old.
       0 lines inserted in new.
       0 lines deleted from old and replaced with 0 lines of new.
       0 lines moved in old.
       2 change blocks.
```

## Testing

Run the unit tests:

```bash
go test -v
```

## Implementation Details

The Go implementation closely follows the C++ version's structure:

- **types.go**: Core data structures and types
- **main.go**: Main entry point, initialization, and orchestration
- **pass1.go**: Hash table construction and file reading
- **pass2.go**: Unique pair identification
- **pass3.go**: Forward match extension
- **pass4.go**: Backward match extension
- **pass5.go**: Tree construction
- **pass6.go**: Replace/delete/insert operations
- **pass7.go**: Adjacent node combination
- **pass8.go**: Move detection and processing

## License

MIT License

Copyright (c) 1979-2022 Reed Kotler

See the main project [MIT-LICENSE](../MIT-LICENSE) for full text.

## References

See the main project [README](../README.md) and [Theory.md](../Theory.md) for detailed algorithm documentation.
