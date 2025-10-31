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

- **Index 0 is reserved**: Used as NULL sentinel values (`NULL_LINE_LIST`, `NULL_STRING_LIST`, `NULL_HASH_LIST`, `NULL_NODE`)
- **Dummy entries**: Tables (`line_table`, `string_table`, `hash_node`, `node`) are initialized with dummy entries at index 0
- **Valid indices start at 1**: The first real entry is at index 1, not index 0
- **Why**: This allows 0 to serve as a sentinel value, simplifying null checks throughout the algorithm

Initialization:
```cpp
// Dummy entries at index 0
line_table.emplace_back();   // Dummy entry at index 0
string_table.emplace_back(); // Dummy entry at index 0
hash_node.emplace_back();    // Dummy entry at index 0
node.emplace_back();         // Dummy entry at index 0 (added in pass5)
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

1. **Initialize node table**: Add dummy entry at index 0 for 1-based indexing
2. **Create header nodes** for both files
3. **Group consecutive lines** into segments based on `ptr_type` and `ptr0` continuity
4. **Create nodes** for each segment with appropriate cost (positive for matched, negative for unmatched)
5. **Link nodes** in doubly-linked lists
6. **Create trailer nodes** for both files

### Segment Grouping

Lines are grouped into contiguous segments based on their `ptr_type`:

1. **syt_type segments**: Unmatched lines (marked as deletions with negative cost)
   - Consecutive `syt_type` lines are grouped together
   - Cost is negative (e.g., -3 for 3 consecutive unmatched lines)

2. **Matched/unique segments**: Matched or unique lines (positive cost)
   - Consecutive matched lines with consecutive `ptr0` values are grouped
   - Cost is positive (e.g., +3 for 3 consecutive matched lines)
   - Requires both `ptr_type != syt_type` AND consecutive `ptr0` values

### Node Creation Process

For each file, `pass5_doit()` groups lines into segments:

```cpp
while i <= total_lines:
    if file_line[i].ptr_type == syt_type:
        // Unmatched block (will be deletion)
        while (i+1 <= total_lines AND
               file_line[i+1].ptr_type == syt_type):
            extend block (i++)
        i++
        cost = -(block_size)  // Negative for deletion
    else:
        // Matched block - requires consecutive ptr0
        ptr0 = file_line[i].ptr0
        exp_ptr0 = ptr0 + 1
        while (i+1 <= total_lines AND
               file_line[i+1].ptr_type != syt_type AND
               file_line[i+1].ptr0 == exp_ptr0):
            extend block (i++, exp_ptr0++)
        i++
        cost = block_size  // Positive for match
```

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
  - `prev = NULL_NODE`, `next` points to first segment

- **Trailer nodes**: Line (total_lines + 1), cost 0, serve as list tails
  - File1 trailer: line = `total_lines[FIRST_FILE] + 1` (positive)
  - File2 trailer: line = `-(total_lines[SECOND_FILE] + 1)` (negative)
  - `next = NULL_NODE`, `prev` points to last segment

- **Header/Trailer Links**:
  - `file_line[FIRST_FILE][0].ptr0 = 0` (header references itself)
  - `file_line[SECOND_FILE][0].ptr0 = 0` (header references itself)
  - Trailers reference each other for pass8 lookups

### Tree Structure

After Pass 5, each file has a linear tree (doubly-linked list):
```
[header] → [segment1] → [segment2] → ... → [trailer]
```

**Important properties:**
- All nodes are initially **leaf nodes** (`branch_start == NULL_NODE`)
- Nodes with negative cost represent unmatched regions (potential deletions/insertions)
- Nodes with positive cost represent matched/unique regions
- Each segment contains consecutive lines with the same matching status

### Example

```
File1: A(match) B(match) C(syt) D(syt) E(match) F(match)
File2: A(match) B(match) X(syt) Y(syt) E(match) F(match)

After Pass 5:
Tree1: [header] → [AB: +2] → [CD: -2] → [EF: +2] → [trailer]
Tree2: [header] → [AB: +2] → [XY: -2] → [EF: +2] → [trailer]
```

**Segment details:**
- AB segment: cost=+2, contains lines 1-2 (matched)
- CD segment: cost=-2, contains lines 3-4 (unmatched, negative cost)
- EF segment: cost=+2, contains lines 5-6 (matched)

### Helper Functions

**make_node(NodeDecl)**: Creates a new node in the node table and returns its index
- First node created after dummy entry will be at index 1
- Returns `node.size() - 1`

**leaf(tree_index)**: Checks if a node is a leaf (has no branch structure)
- Returns `true` if `branch_start == NULL_NODE`
- All nodes created by pass5 are initially leaves

**true_line_of(tree_index)**: Gets absolute line number from a node
- Handles negative line numbers (file2 uses negative values)
- Returns `abs(node[N].linen)`

**free_node(tree_index)**: Adds a node to the free list for reuse
- Only works if `debug_dont_free == false`
- Links node into `free_nodes_start` chain

**each_line_in_node(tree_index, bool always, int starting_line, function)**: Iterates through all lines in a node
- If `always=false`: Only iterates if cost > 0 (skips negative cost segments)
- If `always=true`: Uses absolute value of cost (iterates all segments)
- `starting_line`: Skip lines before this line number
- For leaf nodes: iterates from `noden` to `noden.next`
- For branch nodes: iterates from `branch_start` to `noden`

**count_node(tree_index, LineKinds&)**: Counts cosmetic and non-cosmetic lines in a node
- Uses `each_line_in_node` with `always=false`
- Therefore, **negative cost segments are not counted** (they're counted in pass6)
- `cosmetic_line()` currently always returns `false`, so all lines are non-cosmetic

### Important: Negative Cost Segments

**Critical behavior**: Segments with negative cost (unmatched lines) are handled differently:

- `each_line_in_node()` with `always=false` **skips** negative cost segments
  - Loop condition: `for (sline = max_start; sline < last; sline++)`
  - If cost < 0, then `last = sline + cost < sline`, so loop never executes

- `count_node()` uses `always=false`, so unmatched segments are **not counted**
  - Unmatched segments are counted separately in pass6 (delete/insert operations)

- To iterate unmatched segments, use `always=true`:
  - `each_line_in_node(node, true, 0, callback)` will iterate all lines regardless of cost sign

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

### Phase 1: Replace/Delete (`pass6_do_replace_delete`)

Scans file1 tree sequentially looking for nodes with negative cost (unmatched segments created by pass5).

For each unmatched segment in file1:

1. **Check if replaceable**: Call `pass6_replaceable(node1)`
   - Find corresponding location in file2 based on previous matched node
   - Check if file2 has an unmatched segment at that position
   - If yes → **REPLACE**
   - If no → **DELETE**

2. **Replace operation** (`pass6_replace_lines`):
   - Increment `nchange_blocks` counter
   - Make both node costs positive (now matched)
   - Count lines for statistics (`replace1_stats`, `replace2_stats`)
   - Print context with `after_header()`
   - Print "REPLACE LINE(s)" header
   - Print lines from file1
   - Print "WITH LINE(s)" header
   - Print lines from file2
   - Print trailer
   - Detach file1 node from its tree
   - If previous node is header: attach file2 node to header as branch
   - Otherwise: combine previous node with file2 node (creates branch structure)

3. **Delete operation** (`delete_lines`):
   - Increment `nchange_blocks` counter
   - Make cost positive (for output purposes, but node is detached)
   - Print context with `after_header()` (shows where deletion occurs)
   - Print "DELETE LINE(s)" header
   - Print deleted lines
   - Print trailer
   - Count statistics (`delete_stats`)
   - Detach node from tree (removed from linked list)

**Important**: The iterator `j` is saved before processing because nodes may be detached during processing.

### Replaceability Check (`pass6_replaceable`)

A node in file1 is replaceable if there's a corresponding unmatched node in file2 at the same relative position.

**Algorithm:**
```cpp
prev = node1.prev  // Previous node in file1 tree
prev_other = find_node(file2_tree, file1_line[true_line_of(prev)].ptr0)
noden_other = prev_other.next  // Next node after prev_other in file2
if noden_other.cost < 0:  // Unmatched segment
    return noden_other  // Replaceable!
else:
    return NULL_NODE  // Not replaceable → will be DELETE
```

**Key Points:**
- Replacement requires both files to have unmatched segments at the same position
- The position is determined by finding the corresponding previous matched node
- If file2's next node has `cost >= 0`, it's matched, so file1's unmatched segment becomes a DELETE
- The check does NOT verify that following segments match (Reed removed this test)

### Phase 2: Insert (`pass6_do_insert`)

Scans file2 tree sequentially looking for remaining unmatched segments (negative cost).

For each unmatched segment in file2:

1. **Insert operation** (`pass6_insert_lines`):
   - Increment `nchange_blocks` counter
   - Make cost positive (now inserted)
   - Count statistics (`insert_stats`)
   - Find insertion point in file1 based on previous matched node
   - Print context with `after_lines()` or `top_msg()` if at start
   - Print "INSERT LINE(s)" header
   - Print inserted lines
   - Print trailer
   - Detach node from file2 tree
   - If previous node is header: attach to file1 header as branch
   - Otherwise: combine with previous matched node in file1 (creates branch structure)

**Important**: The iterator `j` is saved before processing because nodes may be detached during processing.

### Helper Functions

**find_node(TreeBounds T, tree_index linen)**: Finds node containing specified line number in a tree
- Searches linearly through tree nodes
- Handles negative line numbers (uses absolute value)
- Returns node index or exits with error if not found
- Used to find corresponding nodes between file1 and file2 trees

**detach_node(tree_index noden)**: Removes node from its doubly-linked list
- Updates `prev.next = next` and `next.prev = prev`
- Node remains in `node` table but is no longer in tree structure
- Used for DELETE and INSERT operations

**combine_nodes(tree_index node1, tree_index node2)**: Creates branch structure by combining two nodes
- Creates new parent node with combined cost
- Sets `branch_start = node1` (or node1's branch_start if node1 is already a branch)
- Sets `branch_end = node2` (or node2's branch_end if node2 is already a branch)
- Detaches both original nodes
- Frees original nodes if they were branches (leaves are kept as branch ends)
- Inserts new parent node at node1's position
- Links branch nodes together in sequence
- Transforms linear tree segments into tree branches

**unique_find(tree_index noden)**: Finds first unique line in a node by scanning backward
- Scans from `end_line + cost - 1` down to `end_line`
- Returns first line with `ptr_type == UNIQUE_TYPE`
- Returns `NULL_NODE` if no unique line found
- Used by `after_lines()` to find context anchor point

**after_lines(tree_index noden)**: Prints context lines before a change operation
- Prints "AFTER LINE(s)" header
- Searches backward for unique line to start context
- Handles both leaf and branch nodes when traversing
- Prints all nodes from unique line to the change point

**after_header(tree_index noden)**: Chooses appropriate context header
- If `noden == tree1_start`: calls `top_msg()` → "AFTER TOP"
- Otherwise: calls `after_lines(noden)` → "AFTER LINE(s)"

**top_msg()**: Prints "AFTER TOP" header for insertions at start of file

**print_header(const char *s)**: Prints formatted header with equals separator
- Format: `*** <text> ======================================= ***`

**print_header1(const char *s)**: Prints formatted header with dash separator
- Format: `*** <text> -------------------------------------- ***`

**print_trailer()**: Prints formatted trailer separator
- Format: `*** ===================================================== ***\n`

### Node Combination Details

`combine_nodes(node1, node2)` creates a branch structure when nodes are combined:

**Process:**
1. Calculate combined cost: `cost = node1.cost + node2.cost`
2. Use node1's starting line number
3. Detach node2 from its tree
4. Detach node1 from its tree
5. If node1 is a branch (not leaf): extract its branch structure
6. If node2 is a branch (not leaf): extract its branch structure
7. Create new parent node
8. Insert new node at node1's original position
9. Link branch nodes together in sequence

**Result Structure:**
```
Before: [prev] → [node1] → [next]
        [prev2] → [node2] → [next2]

After:  [prev] → [parent (branch)]
                      ↓
              [branch_start] → ... → [branch_end]
                     ↑                    ↑
                  node1 sequence      node2 sequence
```

This transforms linear tree segments into tree branches, enabling move detection in pass8.

### Statistics Tracking

Pass6 updates separate statistics for each operation type:

- **delete_stats**: Counts lines deleted from file1
  - Updated by `count_node(noden, delete_stats)` in `delete_lines()`
  - Uses `always=false`, so only counts non-cosmetic lines
  
- **insert_stats**: Counts lines inserted from file2
  - Updated by `count_node(noden, insert_stats)` in `pass6_insert_lines()`
  
- **replace1_stats**: Counts lines replaced in file1
  - Updated by `count_node(node1, replace1_stats)` in `pass6_replace_lines()`
  
- **replace2_stats**: Counts lines replaced in file2
  - Updated by `count_node(node2, replace2_stats)` in `pass6_replace_lines()`

- **nchange_blocks**: Incremented for each DELETE, INSERT, or REPLACE operation

### Output Format

Pass 6 produces formatted output for each operation:

**DELETE operation:**
```
*** AFTER LINE(s) ======================================= ***
     10|matched line before deletion
*** DELETE LINE(s) -------------------------------------- ***
     11|deleted line
*** ===================================================== ***
```

**INSERT operation:**
```
*** AFTER LINE(s) ======================================= ***
     10|matched line before insertion
*** INSERT LINE(s) -------------------------------------- ***
+    11|inserted line
*** ===================================================== ***
```

**REPLACE operation:**
```
*** AFTER LINE(s) ======================================= ***
     10|matched line before replacement
*** REPLACE LINE(s) ------------------------------------- ***
     11|old line
*** WITH LINE(s) ---------------------------------------- ***
+    11|new line
*** ===================================================== ***
```

**Insert at top:**
```
*** AFTER TOP =========================================== ***
*** INSERT LINE(s) -------------------------------------- ***
+     1|inserted at start
*** ===================================================== ***
```

Line numbers are prefixed with `+` for file2 lines (insertions/replacements) to distinguish from file1 line numbers.

### Important Implementation Details

1. **Iterator Safety**: In `pass6_do_replace_delete()` and `pass6_do_insert()`, the iterator `j` is saved before processing because nodes may be detached during processing, which would invalidate `node[i].next`.

2. **Cost Sign Change**:
   - In `delete_lines()`: Cost is made positive before counting (for output purposes)
   - In `pass6_replace_lines()`: Both costs are made positive (now matched)
   - In `pass6_insert_lines()`: Cost is made positive (now inserted)

3. **Branch Creation**: Replace and insert operations create branch structures, transforming the initially linear trees from pass5 into trees with branches. This is necessary for pass8 to detect moved segments.

4. **Context Printing**: `after_lines()` traverses backward through the tree (handling branches) to find a unique line for context, ensuring users can identify where changes occur.

5. **Order of Operations**: Pass6 processes replace/delete before insert. This was switched by Reed from the original order (likely to handle replacements before insertions for better output organization).

---

## Pass 7: Combine Adjacent Nodes

**Purpose**: Merge adjacent nodes that are also adjacent in the other file.

### Algorithm

```cpp
for each node in file1:
    if node is a leaf AND next_node is a leaf:
        if node and next_node are also adjacent in file2:
            combine_nodes(node, next_node)  // In both files
```

### Adjacency Check

Two nodes are adjacent if:
- They are consecutive in file1's tree (`node1.next == node2`)
- They are both **leaf nodes** (not branch structures)
- Their corresponding nodes in file2 are also consecutive

### Process

1. **Scan file1 tree**: For each node, check if it can combine with next
2. **Skip branch nodes**: Branch nodes (created by pass6 replacements/insertions) are skipped
   - Branch nodes represent replaced/inserted segments
   - They're already "combined" structures and shouldn't be combined again
   - Attempting to combine branch nodes causes infinite loops (bug fixed)
3. **Verify adjacency in file2**:
   - Find file2 nodes corresponding to file1 nodes using `ptr0` values
   - Check if they're consecutive
4. **Combine**: If adjacent in both files, merge them
   - Creates larger matched segments
   - Simplifies tree structure
   - Updates costs

### Why Combine

Combining reduces tree complexity:
- Fewer nodes to process
- Better representation of unchanged regions
- Cleaner output

### Branch Node Handling

**Important**: Pass 7 only processes **leaf nodes** (nodes without branch structures).

Branch nodes are skipped because:
- They're created by pass6's REPLACE and INSERT operations
- They represent segments that have already been processed
- Branch nodes' `ptr0` values may not correctly map to file2 branch structures
- Attempting to combine branch nodes causes infinite loops in the original implementation

**Bug Fix**: The implementation now explicitly checks `leaf(node1)` and `leaf(node2)` before attempting to combine nodes. This prevents infinite loops when pass7 runs after pass6.

### Safety Check

A safety check prevents infinite loops:
- Tracks iteration count (max 10,000 iterations)
- Exits with error if limit exceeded
- Prevents hangs on edge cases

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

**Note**: If pass6 has created branch structures (REPLACE/INSERT operations), pass7 will skip those branches and only combine remaining leaf nodes.

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
9. **Node table dummy entry**: Pass5 initializes node table with dummy entry at index 0, maintaining 1-based indexing consistency
10. **Negative cost semantics**: Negative cost segments (unmatched) are handled differently - `each_line_in_node(always=false)` skips them, requiring `always=true` to iterate
11. **Branch structure creation**: Pass6 creates branch structures via `combine_nodes()` for replace/insert operations, transforming linear trees into trees with branches to enable move detection in pass8
12. **Iterator safety**: Pass6 saves iterators before node detachment to prevent invalidation during tree traversal when nodes are removed from linked lists
13. **Pass7 branch node skipping**: Pass7 only processes leaf nodes, skipping branch nodes created by pass6. Branch nodes' `ptr0` mappings don't correctly map to file2 branch structures, causing infinite loops if attempted. The fix explicitly checks `leaf()` before combining, preventing infinite loops when pass7 runs after pass6
14. **Pass7 safety check**: Added iteration count limit (10,000) to prevent infinite loops on edge cases, providing defensive programming protection
15. **Pass8 safety check**: Added iteration count limit (10,000) to prevent infinite loops during move detection, matching Pass7's defensive programming approach

---

## Limitations and Edge Cases

1. **Hash collisions**: Rare but possible; handled by text comparison
2. **Large files**: Memory usage scales with unique line count
3. **Many moves**: Pass 8 may be slow with many small moves
4. **Identical lines**: Multiple occurrences require unique anchors
5. **Complete rewrites**: No anchors means few matches detected
6. **Duplicate-only files**: If all lines are duplicates (appear multiple times), Pass 2 won't mark any as unique, so Pass 3 has no anchors to extend from. This means identical files containing only duplicate lines will incorrectly show as replacements rather than no changes. This is by design: Pass 2 marks lines as unique only if they appear once, and Pass 3 requires unique anchors to extend matches. No workaround exists for truly duplicate-only files
7. **File length mismatches**: Pass 3 stops when one file runs out of lines, even if the other continues
8. **Index bounds**: Must ensure table accesses stay within bounds when files have different lengths
9. **Pass7 branch node limitation**: Pass7 cannot combine branch nodes created by pass6 (REPLACE/INSERT operations). This is by design - branch nodes represent already-processed segments and should not be combined again. Pass7 only processes leaf nodes

---

## Conclusion

The IFCOMP algorithm provides a sophisticated approach to file comparison that goes beyond simple diff algorithms by:
- Detecting moved code blocks
- Providing detailed change statistics
- Using efficient hash-based matching
- Producing readable pseudo-update output

The 8-pass design allows for progressive refinement, ensuring accurate change detection even in complex scenarios with multiple changes, moves, and replacements.
