# IFCOMP Algorithm Theory

This document provides a detailed explanation of the IFCOMP (IF COMPare) algorithm, an 8-pass file comparison algorithm originally designed by Reed Kotler in 1979. The algorithm compares two text files and identifies deletions, insertions, replacements, and moves between them.

## Contents

- Overview: High-level algorithm description
- Data Structures: Explanation of key structures (HashInfo, StringDecl, FileLineDecl, NodeDecl)
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

### HashInfo
```cpp
struct HashInfo {
    uint16_t h1;  // Primary hash: (length << 8) | xor_value
    int64_t h2;   // Secondary hash: bit-set of character pairs
};
```

### StringDecl
Represents a unique line of text:
```cpp
struct StringDecl {
    std::string text;
    string_index next_text_with_same_hash;
    uint8_t file_nlines[two_files];  // Count of occurrences in each file
    line_count file_list[two_files];  // Linked list of line numbers in each file
};
```

### FileLineDecl
Represents a line in one of the input files:
```cpp
struct FileLineDecl {
    line_count ptr0;              // Reference to corresponding line in other file
    string_index file_line_text;  // Index into string_table
    line_count linen;             // Line number in this file
    LineType ptr_type;            // syt_type, unique_type, or match_type
};
```

### NodeDecl
Represents a segment in the tree structure:
```cpp
struct NodeDecl {
    line_count cost;        // Number of lines (negative = unmatched)
    line_count linen;       // Starting line number (negative for file2)
    tree_index prev;        // Previous node in linked list
    tree_index next;        // Next node in linked list
    tree_index branch_start;// Start of branch (for non-leaf nodes)
    tree_index branch_end;  // End of branch (for non-leaf nodes)
};
```

### LineType Enumeration
- `syt_type`: Line not yet matched (SYT = Symbol Table)
- `unique_type`: Line that appears exactly once in each file
- `match_type`: Line that has been matched in both files

---

## Pass 1: Hash Table Construction

**Purpose**: Read both input files and build hash tables to enable efficient line matching.

### Algorithm

1. **Read each file line by line**
2. **Compute hash for each line** using `hash_line()`
3. **Enter line into hash table** using `enter_line()`

### Hash Function Details

The hash function (`hash_line`) computes two values:

1. **h1 (uint16_t)**: Primary hash
   - Format: `(length << 8) | xor_value`
   - `xor_value`: Computed by XORing all characters in pairs
   - Each pair of characters is processed: `xor_val = (xor_val | char1) & ~(xor_val & char1)`

2. **h2 (int64_t)**: Secondary hash
   - A bit-set where bit `j % 31` is set if character pair `j` appears
   - Each pair of characters `(char1, char2)` forms a 16-bit value: `(char1 << 8) | char2`
   - For odd-length strings, the last character is paired with 0

### Hash Table Structure

- **256 buckets**: Hash table uses `h1 % 256` to index into buckets
- **Sorted chains**: Within each bucket, nodes are sorted by hash code (using `hashcode_compare`)
- **Collision handling**: Multiple strings with the same hash are chained via `next_text_with_same_hash`

### Enter Line Process

For each line, `enter_line()` performs:

1. **Find bucket**: `bucket = sec_hash_start_node[h.h1 % nbuckets]`
2. **Search sorted chain**: Compare hash codes to find insertion point
3. **Handle collisions**: If hash matches, search text chain for exact match
4. **Create entries**:
   - New hash node if hash code not found
   - New string entry if text not found
   - Update `file_nlines` and `file_list` if text already exists

### String Table Organization

The string table maintains:
- Unique text strings
- Occurrence counts per file (`file_nlines`)
- Linked lists of line numbers per file (`file_list`)

### File Line Tracking

For each line read:
```cpp
file_line[which_file][linen].ptr_type = LineType::syt_type;  // Not yet matched
file_line[which_file][linen].file_line_text = string_index;  // Reference to string
file_line[which_file][linen].linen = linen;                 // Line number
file_line[which_file][linen].ptr0 = -1;                      // No match yet
```

### Important: 1-Based Indexing

The implementation uses **1-based indexing** for all table access, matching the original legacy code:

- **Index 0 is reserved**: Used as NULL sentinel values (`NULL_LINE_LIST`, `NULL_STRING_LIST`, `NULL_HASH_LIST`)
- **Dummy entries**: Tables (`line_table`, `string_table`, `hash_node`) are initialized with dummy entries at index 0
- **Valid indices start at 1**: The first real entry is at index 1, not index 0
- **Why**: This allows 0 to serve as a sentinel value, simplifying null checks throughout the algorithm

Initialization:
```cpp
// Dummy entries at index 0
line_table.emplace_back();   // Dummy entry at index 0
string_table.emplace_back(); // Dummy entry at index 0
hash_node.emplace_back();    // Dummy entry at index 0
// First real entry will be at index 1
```

### Cleanup

After Pass 1, the hash node table is cleared (no longer needed after initial indexing).

---

## Pass 2: Unique Pair Identification

**Purpose**: Identify lines that appear exactly once in each file and match them as unique pairs.

### Algorithm

```cpp
for each string in string_table:
    if string.file_nlines[first_file] == 1 AND
       string.file_nlines[second_file] == 1:
        // Found a unique pair
        Mark both lines as unique_type
        Set ptr0 to reference each other
```

### Process

1. **Iterate through string_table**: For each unique text string
2. **Check uniqueness**: Line appears exactly once in file1 AND exactly once in file2
3. **Create bidirectional links**:
   ```cpp
   file1_line[linen1].ptr_type = LineType::unique_type;
   file1_line[linen1].ptr0 = linen2;
   file2_line[linen2].ptr_type = LineType::unique_type;
   file2_line[linen2].ptr0 = linen1;
   ```

### Why Unique Pairs Matter

Unique pairs serve as "anchors" for matching because:
- They can only match each other (unique in both files)
- They provide starting points for extending matches
- They help identify regions that are definitely aligned

### Important: Only Exact Unique Pairs

**Critical behavior**: Pass 2 only marks lines as `unique_type` if they appear **exactly once in both files**. 

- Lines that appear **multiple times** in either file remain `syt_type` after Pass 2
- This is essential for Pass 3, which can only extend from `syt_type` lines
- Duplicate lines (appearing 2+ times) remain `syt_type` even if they match between files

This design ensures:
- Unique pairs are reliable anchors (cannot match elsewhere)
- Duplicate lines can be matched by Pass 3 based on context
- Pass 3 has a clear set of lines to work with (those still `syt_type`)

### Example

If line "function foo()" appears:
- Line 10 in file1 (only occurrence)
- Line 5 in file2 (only occurrence)

Then:
- `file1_line[10].ptr_type = unique_type`
- `file1_line[10].ptr0 = 5`
- `file2_line[5].ptr_type = unique_type`
- `file2_line[5].ptr0 = 10`

If a line appears multiple times in either file, it remains `syt_type`:
- "COMMON" appears 3 times in file1, 3 times in file2 → remains `syt_type` after Pass 2
- "UNIQUE_A" appears once in each file → becomes `unique_type` after Pass 2

---

## Pass 3: Forward Match Extension

**Purpose**: Extend matches forward from unique pairs by checking if subsequent lines match.

### Algorithm

```cpp
for each line m in file1:
    if file1_line[m].ptr_type == unique_type:
        n = file1_line[m].ptr0  // Corresponding line in file2

        // Extend forward while lines match
        while (next lines are syt_type AND text matches):
            Mark both lines as match_type
            Create bidirectional links
            Advance m and n
```

### Process

1. **Scan file1 sequentially**: Starting from line 1
2. **Find unique lines**: When encountering a `unique_type` line
3. **Extend forward**: Check if the next lines match
   - **Both lines must be `syt_type`** (not yet matched, not unique)
   - Text must be identical (`file_line[FIRST_FILE][m].file_line_text == file_line[SECOND_FILE][n].file_line_text`)
   - Lines must be consecutive
4. **Mark matches**: Set `ptr_type = match_type` and create bidirectional links

### Critical: Extension Only from SYT_TYPE Lines

**Important implementation detail**: Pass 3 can only extend from lines that are `syt_type` after Pass 2.

- If a line is already `unique_type` (marked by Pass 2), Pass 3 will **not** extend from it
- Only `syt_type` lines (duplicates or non-unique matches) can be extended
- This is why duplicate lines that match are perfect candidates for Pass 3 extension

Example:
```
File1: UNIQUE_A (unique) → COMMON (syt, duplicate) → COMMON (syt, duplicate)
File2: UNIQUE_A (unique) → COMMON (syt, duplicate) → COMMON (syt, duplicate)

After Pass 3:
File1: UNIQUE_A (unique) → COMMON (match) → COMMON (match)
File2: UNIQUE_A (unique) → COMMON (match) → COMMON (match)
```

The `COMMON` lines remain `syt_type` after Pass 2 because they're duplicates, making them eligible for Pass 3 extension.

### Conditions for Extension

Match extension stops when:
- End of file reached
- Next line is not `syt_type` (already matched or unique)
- Text doesn't match
- Line numbers are not consecutive

### Example

```
File1:  A (unique) → B (syt) → C (syt) → D (syt)
File2:  X (unique) → B (syt) → C (syt) → E (syt)

After Pass 3:
File1:  A (unique) → B (match) → C (match) → D (syt)
File2:  X (unique) → B (match) → C (match) → E (syt)
```

Lines B and C are matched because:
1. They follow unique anchors (A and X)
2. Both are `syt_type` (not yet matched)
3. Their text matches between files
4. Extension stops at D/E because text doesn't match

**Important**: If B or C were unique pairs themselves (appearing once in each file), they would have been marked `unique_type` by Pass 2, and Pass 3 would skip them (cannot extend from `unique_type` lines).

---

## Pass 4: Backward Match Extension

**Purpose**: Extend matches backward from unique pairs by checking if previous lines match.

### Algorithm

```cpp
for each line m in file1 (scanning backward):
    if file1_line[m].ptr_type == unique_type:
        n = file1_line[m].ptr0  // Corresponding line in file2

        // Extend backward while lines match
        while (previous lines are syt_type AND text matches):
            Mark both lines as match_type
            Create bidirectional links
            Decrement m and n
```

### Process

1. **Scan file1 backward**: Starting from last line
2. **Find unique lines**: When encountering a `unique_type` line
3. **Extend backward**: Check if the previous lines match
   - Both must be `syt_type`
   - Text must be identical
   - Lines must be consecutive (decreasing)
4. **Mark matches**: Set `ptr_type = match_type` and create links

### Why Both Directions

Extending in both directions (forward and backward) ensures:
- Maximum match coverage
- Better handling of insertions/deletions in the middle
- More accurate change detection

### Example

```
File1:  A (syt) → B (syt) → C (unique)
File2:  A (syt) → B (syt) → C (unique)

After Pass 4:
File1:  A (match) → B (match) → C (unique)
File2:  A (match) → B (match) → C (unique)
```

Lines A and B are matched because they precede unique anchors and match.

---

## Pass 5: Tree Construction

**Purpose**: Build initial tree structures representing file segments (matched and unmatched).

### Algorithm Overview

1. **Create header nodes** for both files
2. **Group consecutive lines** into segments
3. **Create nodes** for each segment
4. **Link nodes** in doubly-linked lists
5. **Create trailer nodes**

### Segment Grouping

Lines are grouped into contiguous segments based on their `ptr_type`:

1. **syt_type segments**: Unmatched lines (marked as deletions with negative cost)
2. **Other segments**: Matched or unique lines (positive cost)

### Node Creation Process

For each file:

```cpp
while i <= total_lines:
    if file_line[i].ptr_type == syt_type:
        // Unmatched block (will be deletion)
        while next line is also syt_type:
            extend block
        cost = -(block_size)  // Negative for deletion
    else:
        // Matched block
        while next line matches consecutively:
            extend block
        cost = block_size  // Positive for match
```

### Node Properties

Each node stores:
- **cost**: Number of lines (negative = unmatched/deletion candidate)
- **linen**: Starting line number (negative for file2 to distinguish files)
- **prev/next**: Doubly-linked list pointers
- **branch_start/branch_end**: Initially null (used later)

### Header and Trailer Nodes

- **Header nodes**: Line 0, cost 0, serve as list heads
- **Trailer nodes**: Line (total_lines + 1), cost 0, serve as list tails
- Headers and trailers are linked: `file1_line[0].ptr0 = 0`, `file2_line[0].ptr0 = 0`

### Tree Structure

After Pass 5, each file has a linear tree (doubly-linked list):
```
[header] → [segment1] → [segment2] → ... → [trailer]
```

Nodes with negative cost represent unmatched regions (potential deletions/insertions).

### Example

```
File1: A(match) B(match) C(syt) D(syt) E(match) F(match)
File2: A(match) B(match) X(syt) Y(syt) E(match) F(match)

Tree1: [header] → [AB: +2] → [CD: -2] → [EF: +2] → [trailer]
Tree2: [header] → [AB: +2] → [XY: -2] → [EF: +2] → [trailer]
```

---

## Pass 6: Replace/Delete/Insert Operations

**Purpose**: Identify and process deletions, insertions, and replacements between the files.

Pass 6 consists of two phases:
1. **Replace/Delete**: Process unmatched segments in file1
2. **Insert**: Process unmatched segments in file2

### Phase 1: Replace/Delete

Scans file1 tree looking for nodes with negative cost (unmatched segments).

For each unmatched segment in file1:

1. **Check if replaceable**: Call `pass6_replaceable(node1)`
   - Find corresponding location in file2
   - Check if file2 has an unmatched segment at that position
   - If yes → **REPLACE**
   - If no → **DELETE**

2. **Replace operation** (`pass6_replace_lines`):
   - Make costs positive (now matched)
   - Count lines for statistics
   - Print output blocks
   - Combine nodes using `combine_nodes()` which creates a branch structure
   - Detach old node from file1

3. **Delete operation** (`delete_lines`):
   - Increment change block counter
   - Print "AFTER LINE(s)" context
   - Print "DELETE LINE(s)"
   - Count statistics
   - Detach node from tree

### Replaceability Check

A node is replaceable if:
- There's a corresponding unmatched node in file2
- The nodes are adjacent to matching segments

The algorithm:
```cpp
prev = node1.prev  // Previous node in file1
prev_other = find_node_in_file2(prev's corresponding line)
noden_other = prev_other.next  // Next node in file2
if noden_other.cost < 0:  // Unmatched
    return noden_other  // Replaceable!
```

### Phase 2: Insert

Scans file2 tree looking for unmatched segments (negative cost).

For each unmatched segment in file2:

1. **Insert operation** (`pass6_insert_lines`):
   - Make cost positive (now inserted)
   - Count statistics
   - Find insertion point in file1
   - Print output blocks
   - If inserting at top: attach to header
   - Otherwise: combine with previous node in file1

2. **Output format**:
   - "AFTER LINE(s)" or "AFTER TOP" context
   - "INSERT LINE(s)" header
   - Lines from file2
   - Trailer

### Node Combination

`combine_nodes(node1, node2)` creates a branch structure:
- Creates new parent node
- `branch_start` points to node1's sequence
- `branch_end` points to node2's sequence
- Links branches together
- Frees original leaf nodes

This transforms the linear tree into a tree with branches.

### Output Format

Pass 6 produces output like:
```
*** AFTER LINE(s) ======================================= ***
     10|matched line
*** DELETE LINE(s) -------------------------------------- ***
     11|deleted line
*** ===================================================== ***
```

---

## Pass 7: Combine Adjacent Nodes

**Purpose**: Merge adjacent nodes that are also adjacent in the other file.

### Algorithm

```cpp
for each node in file1:
    if node and next_node are also adjacent in file2:
        combine_nodes(node, next_node)  // In both files
```

### Adjacency Check

Two nodes are adjacent if:
- They are consecutive in file1's tree (`node1.next == node2`)
- Their corresponding nodes in file2 are also consecutive

### Process

1. **Scan file1 tree**: For each node, check if it can combine with next
2. **Verify adjacency in file2**:
   - Find file2 nodes corresponding to file1 nodes
   - Check if they're consecutive
3. **Combine**: If adjacent in both files, merge them
   - Creates larger matched segments
   - Simplifies tree structure
   - Updates costs

### Why Combine

Combining reduces tree complexity:
- Fewer nodes to process
- Better representation of unchanged regions
- Cleaner output

### Example

```
Before Pass 7:
File1: [A: +1] → [B: +1] → [C: +1]
File2: [A: +1] → [B: +1] → [C: +1]

After Pass 7:
File1: [ABC: +3]
File2: [ABC: +3]
```

If A, B, C are adjacent in both files, they're combined into one node.

---

## Pass 8: Move Detection and Processing

**Purpose**: Detect and process moved code blocks.

### Algorithm Overview

Pass 8 repeatedly scans both trees in parallel, looking for misalignments that indicate moves.

### Scanning Process

```cpp
while true:
    i = tree1_start
    j = tree2_start

    // Skip headers
    i = i.next
    j = j.next

    // Scan in parallel while aligned
    while i != tree1_end AND file1[i].ptr0 == file2[j].line:
        i = i.next
        j = j.next

    // Misalignment found
    if i == tree1_end: break  // Done

    // Find minimum cost node in misaligned region
    min_node = pass8_min_cost_node(i, tree1_end)

    // Find where it should go in file2
    target_pos = find_node_in_file1(corresponding_to_prev_in_file2)

    // Move the segment
    pass8_move_lines(target_pos, min_node)

    // Restart from beginning
```

### Minimum Cost Selection

`pass8_min_cost_node(start, end)` finds the node with minimum cost in a range:
- Scans all nodes from `start` to `end`
- Returns node with smallest `cost` value
- Prefers smaller segments (fewer lines to move)

### Move Operation

`pass8_move_lines(node1, node2)`:

1. **Detach node2** from its current position
2. **Insert node2** after `node1`
3. **Print output**:
   - "AFTER LINE(s)" context
   - "MOVE LINE(s)" header
   - Lines from moved segment
   - Trailer
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

```
Before move:
File1: A → B → C → D → E
File2: A → C → D → B → E

After scanning: A matches, then misalignment at B
- B should come after D (where it appears in file2)
- Move B to after D

After move:
File1: A → C → D → B → E
File2: A → C → D → B → E
```

### Termination

Pass 8 terminates when:
- Scanning reaches the end without misalignment
- All segments are properly aligned

---

## Output Format

The algorithm produces pseudo-update format output:

### Block Types

1. **DELETE**:
```
*** AFTER LINE(s) ======================================= ***
     <context lines>
*** DELETE LINE(s) -------------------------------------- ***
     <deleted lines>
*** ===================================================== ***
```

2. **INSERT**:
```
*** AFTER TOP =========================================== ***
*** INSERT LINE(s) -------------------------------------- ***
+    <inserted lines>
*** ===================================================== ***
```

3. **REPLACE**:
```
*** AFTER LINE(s) ======================================= ***
     <context lines>
*** REPLACE LINE(s) ------------------------------------- ***
     <old lines>
*** WITH LINE(s) ---------------------------------------- ***
+    <new lines>
*** ===================================================== ***
```

4. **MOVE**:
```
*** AFTER LINE(s) ======================================= ***
     <context lines>
*** MOVE LINE(s) ---------------------------------------- ***
     <moved lines>
*** ===================================================== ***
```

### Summary Statistics

At the end:
```
     <n> lines deleted from old.
     <n> lines inserted in new.
     <n> lines deleted from old and replaced with <n> lines of new.
     <n> lines moved in old.
     <n> change blocks.
```

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
4. **Unique anchors**: Provides reliable matching points
5. **Bidirectional extension**: Maximizes match coverage
6. **Minimum cost moves**: Reduces move complexity
7. **1-based indexing**: Uses index 0 as NULL sentinel, simplifying null checks
8. **SYT_TYPE requirement for extension**: Pass 3 only extends from `syt_type` lines, ensuring unique pairs remain unique while allowing duplicate matches to be extended contextually

---

## Limitations and Edge Cases

1. **Hash collisions**: Rare but possible; handled by text comparison
2. **Large files**: Memory usage scales with unique line count
3. **Many moves**: Pass 8 may be slow with many small moves
4. **Identical lines**: Multiple occurrences require unique anchors
5. **Complete rewrites**: No anchors means few matches detected
6. **Duplicate-only files**: If all lines are duplicates (appear multiple times), Pass 2 won't mark any as unique, so Pass 3 has no anchors to extend from
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
