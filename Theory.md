# IFCOMP Algorithm Theory

This document provides a detailed explanation of the IFCOMP (IF COMPare) algorithm, an 8-pass file comparison algorithm originally designed by Reed Kotler in 1979. The algorithm compares two text files and identifies deletions, insertions, replacements, and moves between them.

## Contents

- Overview: High-level algorithm description
- Data Structures: Explanation of key structures (HashNodeDecl, StringDecl, FileLineDecl, NodeDecl)
- Pass 1: Hash table construction with details on the hash function
- Pass 2: Unique pair identification
- Pass 3: Forward match extension
- Pass 4: Backward match extension
- Pass 5: Tree construction
- Pass 6: Replace/Delete/Insert operations (two phases)
- Pass 7: Combine adjacent nodes
- Pass 8: Move detection and processing
- Additional sections: Output format, complexity analysis, design decisions, limitations

## Overview

IFCOMP uses an 8-pass algorithm to perform file comparison:

- **Passes 1-4**: Build data structures and identify matching lines
- **Passes 5-8**: Build trees and identify changes (deletions, insertions, replacements, moves)

The algorithm uses:
- Hash-based line matching for efficiency
- Tree structures to represent file segments
- Multiple passes to progressively refine the comparison

## Data Structures

Before diving into the passes, it's important to understand the key data structures:

### Nested State Structures

The algorithm organizes related data into nested structures:

#### HashTableState
Hash table state (used only in pass1):
- Stores hash nodes for efficient line lookup
- Contains buckets for organizing lines by hash value

#### FileState
Per-file line data:
- Stores line information for both files
- Tracks total line count per file
- Uses 1-based indexing with dummy entries at index 0

#### LineMatchingState
Line matching tables:
- Maintains unique text strings
- Tracks line mappings between files

#### TreeState
Tree structure for passes 5-8:
- Stores nodes representing file segments
- Maintains tree boundaries for each file
- Manages free nodes for reuse

#### Statistics
Change tracking:
- Counts deletions, insertions, moves, replacements
- Tracks number of change blocks

### StringDecl
Represents a unique line of text:
- Stores the text content
- Links to other strings with same hash
- Tracks occurrence count per file
- Maintains linked lists of line numbers

### FileLineDecl
Represents a line in one of the input files:
- References corresponding line in other file
- Links to unique text in string table
- Stores line number
- Records line type (SYT_TYPE, UNIQUE_TYPE, or MATCH_TYPE)

### NodeDecl
Represents a segment in the tree structure:
- Cost: number of lines (negative = unmatched)
- Line number: starting position (negative for file2)
- Prev/next: doubly-linked list pointers
- Branch pointers: structure for non-leaf nodes

### LineType Enumeration
- `SYT_TYPE`: Line not yet matched (SYT = Symbol Table)
- `UNIQUE_TYPE`: Line that appears exactly once in each file
- `MATCH_TYPE`: Line that has been matched in both files

---

## Pass 1: Hash Table Construction

**Purpose**: Read both input files and build hash tables to enable efficient line matching.

### Algorithm

1. **Read each file line by line**
2. **Compute hash for each line**
3. **Enter line into hash table**

### Hash Function Details

The hash function uses standard library hashing to produce consistent hash values. Hash codes are compared to maintain sorted order within buckets.

### Hash Table Structure

- **256 buckets**: Hash table uses modulo operation to index into buckets
- **Sorted chains**: Within each bucket, nodes are sorted by hash code
- **Collision handling**: Multiple strings with the same hash are chained together

### Enter Line Process

For each line:
1. Find the appropriate bucket
2. Search the sorted chain to find insertion point
3. Handle collisions by searching text chain for exact match
4. Create entries if needed or update existing entries

### String Table Organization

The string table maintains:
- Unique text strings
- Occurrence counts per file
- Linked lists of line numbers per file

### File Line Tracking

For each line read:
- Line is marked as `SYT_TYPE` (not yet matched)
- Reference to unique text in string table is stored
- Line number is recorded
- No match is established yet (ptr0 = -1)

### Important: 1-Based Indexing

The implementation uses **1-based indexing** for all table access:

- **Index 0 is reserved**: Used as NULL sentinel values
- **Dummy entries**: Tables are initialized with dummy entries at index 0
- **Valid indices start at 1**: The first real entry is at index 1
- **Why**: This allows 0 to serve as a sentinel value, simplifying null checks

### Cleanup

After Pass 1, the hash node table is cleared (no longer needed after initial indexing).

---

## Pass 2: Unique Pair Identification

**Purpose**: Identify lines that appear exactly once in each file and match them as unique pairs.

### Algorithm

For each string in the string table:
- If the line appears exactly once in file1 AND exactly once in file2:
  - Mark both lines as `UNIQUE_TYPE`
  - Set ptr0 to reference each other

### Process

1. **Iterate through string_table**: For each unique text string
2. **Check uniqueness**: Line appears exactly once in file1 AND exactly once in file2
3. **Create bidirectional links**: Both lines reference each other

### Why Unique Pairs Matter

Unique pairs serve as "anchors" for matching because:
- They can only match each other (unique in both files)
- They provide starting points for extending matches
- They help identify regions that are definitely aligned

### Important: Only Exact Unique Pairs

**Critical behavior**: Pass 2 only marks lines as `UNIQUE_TYPE` if they appear **exactly once in both files**.

- Lines that appear **multiple times** in either file remain `SYT_TYPE` after Pass 2
- This is essential for Pass 3, which can only extend from `SYT_TYPE` lines
- Duplicate lines (appearing 2+ times) remain `SYT_TYPE` even if they match between files

This design ensures:
- Unique pairs are reliable anchors (cannot match elsewhere)
- Duplicate lines can be matched by Pass 3 based on context
- Pass 3 has a clear set of lines to work with (those still `SYT_TYPE`)

### Example

If line "function foo()" appears:
- Line 10 in file1 (only occurrence)
- Line 5 in file2 (only occurrence)

Then both lines are marked as `UNIQUE_TYPE` and reference each other.

If a line appears multiple times in either file, it remains `SYT_TYPE`:
- "COMMON" appears 3 times in file1, 3 times in file2 → remains `SYT_TYPE` after Pass 2
- "UNIQUE_A" appears once in each file → becomes `UNIQUE_TYPE` after Pass 2

---

## Pass 3: Forward Match Extension

**Purpose**: Extend matches forward from unique pairs by checking if subsequent lines match.

### Algorithm

For each line in file1:
- If the line is `UNIQUE_TYPE`:
  - Find corresponding line in file2
  - Extend forward while lines match
  - Mark matching lines as `MATCH_TYPE`
  - Create bidirectional links

### Process

1. **Scan file1 sequentially**: Starting from line 1
2. **Find unique lines**: When encountering a `UNIQUE_TYPE` line
3. **Extend forward**: Check if the next lines match
   - **Both lines must be `SYT_TYPE`** (not yet matched, not unique)
   - Text must be identical
   - Lines must be consecutive
4. **Mark matches**: Set `ptr_type = MATCH_TYPE` and create bidirectional links

### Critical: Extension Only from SYT_TYPE Lines

**Important implementation detail**: Pass 3 can only extend from lines that are `SYT_TYPE` after Pass 2.

- If a line is already `UNIQUE_TYPE` (marked by Pass 2), Pass 3 will **not** extend from it
- Only `SYT_TYPE` lines (duplicates or non-unique matches) can be extended
- This is why duplicate lines that match are perfect candidates for Pass 3 extension

Example:
- File1: UNIQUE_A (unique) → COMMON (syt, duplicate) → COMMON (syt, duplicate)
- File2: UNIQUE_A (unique) → COMMON (syt, duplicate) → COMMON (syt, duplicate)

After Pass 3:
- File1: UNIQUE_A (unique) → COMMON (match) → COMMON (match)
- File2: UNIQUE_A (unique) → COMMON (match) → COMMON (match)

The `COMMON` lines remain `SYT_TYPE` after Pass 2 because they're duplicates, making them eligible for Pass 3 extension.

### Conditions for Extension

Match extension stops when:
- End of file reached
- Next line is not `SYT_TYPE` (already matched or unique)
- Text doesn't match
- Line numbers are not consecutive

### Example

- File1: A (unique) → B (syt) → C (syt) → D (syt)
- File2: X (unique) → B (syt) → C (syt) → E (syt)

After Pass 3:
- File1: A (unique) → B (match) → C (match) → D (syt)
- File2: X (unique) → B (match) → C (match) → E (syt)

Lines B and C are matched because they follow unique anchors, are both `SYT_TYPE`, match text, and are consecutive. Extension stops at D/E because text doesn't match.

---

## Pass 4: Backward Match Extension

**Purpose**: Extend matches backward from unique pairs by checking if previous lines match.

### Algorithm

For each line in file1 (scanning backward):
- If the line is `UNIQUE_TYPE`:
  - Find corresponding line in file2
  - Extend backward while lines match
  - Mark matching lines as `MATCH_TYPE`
  - Create bidirectional links

### Process

1. **Scan file1 backward**: Starting from last line
2. **Find unique lines**: When encountering a `UNIQUE_TYPE` line
3. **Extend backward**: Check if the previous lines match
   - Both must be `SYT_TYPE`
   - Text must be identical
   - Lines must be consecutive (decreasing)
4. **Mark matches**: Set `ptr_type = MATCH_TYPE` and create links

### Why Both Directions

Extending in both directions (forward and backward) ensures:
- Maximum match coverage
- Better handling of insertions/deletions in the middle
- More accurate change detection

### Example

- File1: A (syt) → B (syt) → C (unique)
- File2: A (syt) → B (syt) → C (unique)

After Pass 4:
- File1: A (match) → B (match) → C (unique)
- File2: A (match) → B (match) → C (unique)

Lines A and B are matched because they precede unique anchors and match.

---

## Pass 5: Tree Construction

**Purpose**: Build initial tree structures representing file segments (matched and unmatched).

### Algorithm Overview

1. **Initialize node table**: Add dummy entry at index 0 for 1-based indexing
2. **Create header nodes** for both files
3. **Group consecutive lines** into segments based on `ptr_type` and `ptr0` continuity
4. **Create nodes** for each segment with appropriate cost (positive for matched, negative for unmatched)
5. **Link nodes** in doubly-linked lists
6. **Create trailer nodes** for both files

### Segment Grouping

Lines are grouped into contiguous segments based on their `ptr_type`:

1. **SYT_TYPE segments**: Unmatched lines (marked as deletions with negative cost)
   - Consecutive `SYT_TYPE` lines are grouped together
   - Cost is negative (e.g., -3 for 3 consecutive unmatched lines)

2. **Matched/unique segments**: Matched or unique lines (positive cost)
   - Consecutive matched lines with consecutive `ptr0` values are grouped
   - Cost is positive (e.g., +3 for 3 consecutive matched lines)
   - Requires both `ptr_type != LineType::SYT_TYPE` AND consecutive `ptr0` values

### Node Creation Process

For each file, lines are grouped into segments:
- Unmatched block: Group consecutive `SYT_TYPE` lines, assign negative cost
- Matched block: Group consecutive matched lines with consecutive `ptr0`, assign positive cost

**Key Points:**
- Segment grouping stops when `ptr_type` changes OR `ptr0` is not consecutive
- File2 nodes use **negative line numbers** to distinguish from file1
- Segments are created as leaf nodes initially (no branch structure)

### Node Properties

Each node stores:
- **cost**: Number of lines (negative = unmatched/deletion candidate)
- **linen**: Starting line number (negative for file2 to distinguish files)
- **prev/next**: Doubly-linked list pointers
- **branch_start/branch_end**: Initially null (used later)

### Header and Trailer Nodes

- **Header nodes**: Line 0, cost 0, serve as list heads
  - Created first in each file's tree
  - Stored at indices >= 1 (after dummy entry at index 0)

- **Trailer nodes**: Line (total_lines + 1), cost 0, serve as list tails
  - File1 trailer uses positive line number
  - File2 trailer uses negative line number

- **Header/Trailer Links**: Reference each other for tree navigation

### Tree Structure

After Pass 5, each file has a linear tree (doubly-linked list):
- Header → segment1 → segment2 → ... → trailer

**Important properties:**
- All nodes are initially **leaf nodes**
- Nodes with negative cost represent unmatched regions (potential deletions/insertions)
- Nodes with positive cost represent matched/unique regions
- Each segment contains consecutive lines with the same matching status

### Example

- File1: A(match) B(match) C(syt) D(syt) E(match) F(match)
- File2: A(match) B(match) X(syt) Y(syt) E(match) F(match)

After Pass 5:
- Tree1: header → AB: +2 → CD: -2 → EF: +2 → trailer
- Tree2: header → AB: +2 → XY: -2 → EF: +2 → trailer

**Segment details:**
- AB segment: cost=+2, contains lines 1-2 (matched)
- CD segment: cost=-2, contains lines 3-4 (unmatched, negative cost)
- EF segment: cost=+2, contains lines 5-6 (matched)

### Helper Functions

- **make_node**: Creates a new node in the node table
- **leaf**: Checks if a node is a leaf (has no branch structure)
- **true_line_of**: Gets absolute line number from a node
- **free_node**: Adds a node to the free list for reuse
- **each_line_in_node**: Iterates through all lines in a node
  - If `always=false`: Only iterates if cost > 0 (skips negative cost segments)
  - If `always=true`: Uses absolute value of cost (iterates all segments)
- **count_node**: Counts cosmetic and non-cosmetic lines in a node
  - Uses `each_line_in_node` with `always=false`
  - Therefore, **negative cost segments are not counted** (they're counted in pass6)

### Important: Negative Cost Segments

**Critical behavior**: Segments with negative cost (unmatched lines) are handled differently:

- `each_line_in_node()` with `always=false` **skips** negative cost segments
- `count_node()` uses `always=false`, so unmatched segments are **not counted**
- Unmatched segments are counted separately in pass6 (delete/insert operations)
- To iterate unmatched segments, use `always=true`

---

## Pass 6: Replace/Delete/Insert Operations

**Purpose**: Identify and process deletions, insertions, and replacements between the files. Converts unmatched segments (negative cost) from pass5 into explicit change operations with formatted output.

**Input**: Trees from pass5 with unmatched segments (negative cost nodes) representing lines that don't match between files.

**Output**:
- Formatted change operations (DELETE, INSERT, REPLACE)
- Updated tree structures with branch nodes for replacements
- Statistics for delete, insert, and replace operations

Pass 6 consists of two phases executed sequentially:
1. **Replace/Delete**: Process unmatched segments in file1
2. **Insert**: Process unmatched segments in file2

### Phase 1: Replace/Delete

Scans file1 tree sequentially looking for nodes with negative cost (unmatched segments created by pass5).

For each unmatched segment in file1:

1. **Check if replaceable**: Find corresponding location in file2 based on previous matched node
   - If file2 has an unmatched segment at that position → **REPLACE**
   - If no unmatched segment → **DELETE**

2. **Replace operation**:
   - Increment change blocks counter
   - Make both node costs positive (now matched)
   - Count lines for statistics
   - Print context and headers
   - Print lines from file1 and file2
   - Detach file1 node from its tree
   - Attach file2 node to header as branch or combine with previous node (creates branch structure)

3. **Delete operation**:
   - Increment change blocks counter
   - Make cost positive (for output purposes, but node is detached)
   - Print context (shows where deletion occurs)
   - Print deleted lines
   - Count statistics
   - Detach node from tree (removed from linked list)

**Important**: The iterator is saved before processing because nodes may be detached during processing.

### Replaceability Check

A node in file1 is replaceable if there's a corresponding unmatched node in file2 at the same relative position.

**Key Points:**
- Replacement requires both files to have unmatched segments at the same position
- The position is determined by finding the corresponding previous matched node
- If file2's next node has `cost >= 0`, it's matched, so file1's unmatched segment becomes a DELETE
- The check does NOT verify that following segments match

### Phase 2: Insert

Scans file2 tree sequentially looking for remaining unmatched segments (negative cost).

For each unmatched segment in file2:

1. **Insert operation**:
   - Increment change blocks counter
   - Make cost positive (now inserted)
   - Count statistics
   - Find insertion point in file1 based on previous matched node
   - Print context or "AFTER TOP" if at start
   - Print inserted lines
   - Detach node from file2 tree
   - Attach to file1 header as branch or combine with previous matched node (creates branch structure)

**Important**: The iterator is saved before processing because nodes may be detached during processing.

### Helper Functions

- **find_node**: Finds node containing specified line number in a tree
- **detach_node**: Removes node from its doubly-linked list
- **combine_nodes**: Creates branch structure by combining two nodes
  - Transforms linear tree segments into tree branches
- **unique_find**: Finds first unique line in a node by scanning backward
- **after_lines**: Prints context lines before a change operation
- **after_header**: Chooses appropriate context header
- **top_msg**: Prints "AFTER TOP" header for insertions at start
- **print_header/print_header1/print_trailer**: Format output headers and trailers

### Node Combination Details

`combine_nodes` creates a branch structure when nodes are combined, transforming linear tree segments into tree branches with parent-child relationships. This enables move detection in pass8.

### Statistics Tracking

Pass6 updates separate statistics for each operation type:
- Deletion stats
- Insertion stats
- Replacement stats (both files)
- Change block counter

### Output Format

Pass 6 produces formatted output for each operation type:
- **DELETE**: Shows context, deleted lines, and separator
- **INSERT**: Shows context (or "AFTER TOP"), inserted lines with `+` prefix, and separator
- **REPLACE**: Shows context, old lines, new lines with `+` prefix, and separator

Line numbers are prefixed with `+` for file2 lines to distinguish from file1 line numbers.

### Important Implementation Details

1. **Iterator Safety**: The iterator is saved before processing because nodes may be detached during processing

2. **Cost Sign Change**: Costs are made positive for output purposes and statistics

3. **Branch Creation**: Replace and insert operations create branch structures, transforming the initially linear trees from pass5 into trees with branches. This is necessary for pass8 to detect moved segments.

4. **Context Printing**: Traverses backward through the tree to find a unique line for context

5. **Order of Operations**: Pass6 processes replace/delete before insert for better output organization

---

## Pass 7: Combine Adjacent Nodes

**Purpose**: Merge adjacent nodes that are also adjacent in the other file.

### Algorithm

For each node in file1 tree:
- If node and next_node are also adjacent in file2:
  - Combine both nodes

### Adjacency Check

Two nodes are adjacent if:
- They are consecutive in file1's tree
- Their corresponding nodes in file2 are also consecutive

### Process

1. **Scan file1 tree**: For each node, check if it can combine with next
2. **Verify adjacency in file2**: Find corresponding nodes and check if consecutive
3. **Combine**: If adjacent in both files, merge them
   - Creates larger matched segments
   - Simplifies tree structure
   - Updates costs

### Why Combine

Combining reduces tree complexity:
- Fewer nodes to process
- Better representation of unchanged regions
- Cleaner output

### Safety Check

A safety check prevents infinite loops:
- Tracks iteration count (max 10,000 iterations)
- Exits with error if limit exceeded
- Prevents hangs on edge cases

### Example

- Before Pass 7: A(+1) → B(+1) → C(+1)
- After Pass 7: ABC(+3)

If A, B, C are adjacent in both files, they're combined into one node.

---

## Pass 8: Move Detection and Processing

**Purpose**: Detect and process moved code blocks.

### Algorithm Overview

Pass 8 repeatedly scans both trees in parallel, looking for misalignments that indicate moves.

### Scanning Process

While scanning:
- Skip headers and scan in parallel while aligned
- When misalignment found: find minimum cost node in misaligned region
- Determine target position in file2
- Move the segment
- Restart from beginning

### Minimum Cost Selection

Finds the node with minimum cost in a range:
- Scans all nodes from start to end
- Returns node with smallest cost value
- Prefers smaller segments (fewer lines to move)

### Move Operation

1. **Detach** node from its current position
2. **Insert** node at target position
3. **Print output**: context, "MOVE LINE(s)" header, moved lines, trailer
4. **Update statistics**: Count moved lines
5. **Re-run Pass 7**: Try to combine adjacent nodes after move

### Why Minimum Cost

Selecting minimum cost minimizes:
- Number of lines moved at once
- Complexity of move operations
- Likelihood of incorrect moves

### Iteration

Pass 8 restarts from the beginning after each move because:
- Moves change the tree structure
- Earlier segments may now be moveable
- Ensures all possible moves are detected

### Example

- Before move: A → B → C → D → E vs A → C → D → B → E
- After scanning: A matches, then misalignment at B
- Move: B should come after D (where it appears in file2)
- After move: A → C → D → B → E (aligned)

### Termination

Pass 8 terminates when scanning reaches the end without misalignment.

---

## Output Format

The algorithm produces pseudo-update format output with four block types:

1. **DELETE**: Shows context lines, deleted lines, and separators
2. **INSERT**: Shows "AFTER TOP" or context, inserted lines with `+` prefix, and separators
3. **REPLACE**: Shows context, old lines, new lines with `+` prefix, and separators
4. **MOVE**: Shows context, moved lines, and separators

### Summary Statistics

At the end, displays counts for:
- Lines deleted from old
- Lines inserted in new
- Lines replaced in old and new
- Lines moved in old
- Total change blocks

---

## Algorithm Properties

### Time Complexity

- **Pass 1**: O(N + M) where N, M are line counts
- **Pass 2**: O(S) where S is number of unique strings
- **Pass 3-4**: O(N + M) sequential scans
- **Pass 5**: O(N + M) single pass per file
- **Pass 6**: O(N + M) processing each node once
- **Pass 7**: O(N) combining adjacent nodes
- **Pass 8**: O(N²) worst case (restarts after each move)

**Overall**: O(N²) in worst case, O(N log N) average case

### Space Complexity

- **Hash tables**: O(S) where S is number of unique lines
- **String table**: O(S × L) where L is average line length
- **File line arrays**: O(N + M)
- **Tree nodes**: O(N + M)

**Overall**: O(N + M + S × L)

### Correctness

The algorithm guarantees:
- All unique pairs are identified
- Matches are extended maximally
- Changes are classified correctly
- Moves are detected when possible

---

## Key Design Decisions

1. **Hash-based matching**: Enables O(1) average case lookup
2. **Multi-pass approach**: Allows progressive refinement
3. **Tree structure**: Enables efficient move detection
4. **Unique anchors**: Provides reliable matching points (Pass 2)
5. **Bidirectional extension**: Maximizes match coverage (Passes 3-4)
6. **Minimum cost moves**: Reduces move complexity (Pass 8)
7. **Negative cost semantics**: Unmatched segments have negative cost, matched segments positive
8. **Branch creation**: Pass 6 creates branches for replacements/insertions to enable move detection
9. **Safety checks**: Iteration limits prevent infinite loops in passes 7-8

---

## Limitations and Edge Cases

1. **Hash collisions**: Rare but possible; handled by text comparison
2. **Large files**: Memory usage scales with unique line count
3. **Many moves**: Pass 8 may be slow with many small moves
4. **Identical lines**: Multiple occurrences require unique anchors
5. **Complete rewrites**: No anchors means few matches detected
6. **Duplicate-only files**: If all lines are duplicates (appear multiple times), Pass 2 won't mark any as unique, so Pass 3 has no anchors to extend from. This means identical files containing only duplicate lines will incorrectly show as replacements rather than no changes. This is by design: Pass 2 marks lines as unique only if they appear once, and Pass 3 requires unique anchors to extend matches.
7. **File length mismatches**: Pass 3 stops when one file runs out of lines, even if the other continues
8. **Index bounds**: Must ensure table accesses stay within bounds when files have different lengths

---

## Conclusion

The IFCOMP algorithm provides a sophisticated approach to file comparison that goes beyond simple diff algorithms by:
- Detecting moved code blocks
- Providing detailed change statistics
- Using efficient hash-based matching
- Producing readable pseudo-update output

The 8-pass design allows for progressive refinement, ensuring accurate change detection even in complex scenarios with multiple changes, moves, and replacements.
