use ifcomp::{FileIndex, Ifcomp, LineCount, LineType, LineKinds, NULL_NODE};

// Test helper: create fresh Ifcomp instance for each test
fn setup_pass5_test() -> Ifcomp {
    Ifcomp::new()
}

// ============================================================================
// Tests for helper functions
// ============================================================================

#[test]
fn test_pass5_make_node_basic() {
    // After pass1-4, pass5 creates nodes
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Should have created dummy entry + header, segment, and trailer nodes
    assert!(
        ifc.tree_state.node.len() > 3,
        "Should have dummy + header, segment, and trailer nodes"
    );
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Tree should have header"
    );
    assert!(
        (ifc.tree_state.trees[0].start as usize) >= 1,
        "Header should be at index >= 1 (after dummy)"
    );
    assert_ne!(
        ifc.tree_state.trees[0].end, NULL_NODE,
        "Tree should have trailer"
    );
}

#[test]
fn test_pass5_leaf_basic() {
    // Test leaf() function - nodes created by pass5 should be leaves initially
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Segment node should be a leaf (no branch structure initially)
    let segment_node = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;
    assert!(
        ifc.leaf(segment_node),
        "Segment node should be a leaf"
    );
}

#[test]
fn test_pass5_true_line_of_file1() {
    // Test true_line_of() for file1 (positive line numbers)
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Segment node should have positive line number
    let segment_node = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;
    let line = ifc.true_line_of(segment_node);
    assert_eq!(line, 1, "File1 segment should have line 1");
    assert!(line >= 0, "Line should be non-negative");
}

#[test]
fn test_pass5_true_line_of_file2() {
    // Test true_line_of() for file2 (negative line numbers stored as negative)
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Segment node for file2 should have negative line number
    let segment_node = ifc.tree_state.node[ifc.tree_state.trees[1].start].next;
    let stored_line = ifc.tree_state.node[segment_node].linen;
    assert!(
        stored_line < 0,
        "File2 segment should have negative line number"
    );

    let line = ifc.true_line_of(segment_node);
    assert_eq!(line, 1, "true_line_of should return absolute value 1");
    assert!(line >= 0, "Line should be non-negative");
}

#[test]
fn test_pass5_free_node_basic() {
    // Test free_node() function
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Get a node to free
    let segment_node = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;
    let original_free_start = ifc.tree_state.free_nodes_start;

    // Free the node
    ifc.free_node(segment_node);

    assert_eq!(
        ifc.tree_state.free_nodes_start, segment_node,
        "Freed node should be at head of free list"
    );
    assert_eq!(
        ifc.tree_state.node[segment_node].next, original_free_start,
        "Freed node should link to previous free start"
    );
}

// ============================================================================
// Tests for pass5() - Basic functionality
// ============================================================================

#[test]
fn test_pass5_single_matched_line() {
    // Single line that matches
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Should have: header -> segment -> trailer
    let header = ifc.tree_state.trees[0].start;
    let segment = ifc.tree_state.node[header].next;
    let trailer = ifc.tree_state.trees[0].end;

    assert_ne!(header, NULL_NODE, "Should have header");
    assert_ne!(segment, NULL_NODE, "Should have segment");
    assert_ne!(trailer, NULL_NODE, "Should have trailer");

    // Header should point to segment
    assert_eq!(
        ifc.tree_state.node[header].next, segment,
        "Header should point to segment"
    );
    assert_eq!(
        ifc.tree_state.node[segment].prev, header,
        "Segment should point back to header"
    );

    // Segment should point to trailer
    assert_eq!(
        ifc.tree_state.node[segment].next, trailer,
        "Segment should point to trailer"
    );
    assert_eq!(
        ifc.tree_state.node[trailer].prev, segment,
        "Trailer should point back to segment"
    );

    // Segment should have positive cost (matched)
    assert!(
        ifc.tree_state.node[segment].cost > 0,
        "Matched segment should have positive cost"
    );
    assert_eq!(
        ifc.tree_state.node[segment].cost, 1,
        "Single line segment should have cost 1"
    );
}

#[test]
fn test_pass5_multiple_matched_lines() {
    // Multiple lines that match
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Should have single segment with cost 3
    let header = ifc.tree_state.trees[0].start;
    let segment = ifc.tree_state.node[header].next;

    assert_eq!(
        ifc.tree_state.node[segment].cost, 3,
        "Three matched lines should have cost 3"
    );
    assert!(
        ifc.tree_state.node[segment].cost > 0,
        "Matched segment should have positive cost"
    );
}

#[test]
fn test_pass5_single_unmatched_line() {
    // Single unmatched line (SYT_TYPE)
    let mut ifc = setup_pass5_test();
    let file1 = "DIFFERENT1\n".as_bytes();
    let file2 = "DIFFERENT2\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Should have segment with negative cost
    let header = ifc.tree_state.trees[0].start;
    let segment = ifc.tree_state.node[header].next;

    assert!(
        ifc.tree_state.node[segment].cost < 0,
        "Unmatched segment should have negative cost"
    );
    assert_eq!(
        ifc.tree_state.node[segment].cost, -1,
        "Single unmatched line should have cost -1"
    );
}

#[test]
fn test_pass5_multiple_unmatched_lines() {
    // Multiple unmatched lines (SYT_TYPE)
    let mut ifc = setup_pass5_test();
    let file1 = "DIFF1\nDIFF2\nDIFF3\n".as_bytes();
    let file2 = "OTHER1\nOTHER2\nOTHER3\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Should have single segment with negative cost
    let header = ifc.tree_state.trees[0].start;
    let segment = ifc.tree_state.node[header].next;

    assert!(
        ifc.tree_state.node[segment].cost < 0,
        "Unmatched segment should have negative cost"
    );
    assert_eq!(
        ifc.tree_state.node[segment].cost, -3,
        "Three unmatched lines should have cost -3"
    );
}

#[test]
fn test_pass5_mixed_matched_and_unmatched() {
    // Mix of matched and unmatched lines
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\nDIFF1\nUNIQUE_B\nDIFF2\n".as_bytes();
    let file2 = "UNIQUE_A\nOTHER1\nUNIQUE_B\nOTHER2\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Should have 4 segments: matched, unmatched, matched, unmatched
    let header = ifc.tree_state.trees[0].start;
    let seg1 = ifc.tree_state.node[header].next;
    let seg2 = ifc.tree_state.node[seg1].next;
    let seg3 = ifc.tree_state.node[seg2].next;
    let seg4 = ifc.tree_state.node[seg3].next;

    // First segment: matched (UNIQUE_A)
    assert!(
        ifc.tree_state.node[seg1].cost > 0,
        "First segment should be matched"
    );
    assert_eq!(
        ifc.tree_state.node[seg1].cost, 1,
        "First segment should have cost 1"
    );

    // Second segment: unmatched (DIFF1)
    assert!(
        ifc.tree_state.node[seg2].cost < 0,
        "Second segment should be unmatched"
    );
    assert_eq!(
        ifc.tree_state.node[seg2].cost, -1,
        "Second segment should have cost -1"
    );

    // Third segment: matched (UNIQUE_B)
    assert!(
        ifc.tree_state.node[seg3].cost > 0,
        "Third segment should be matched"
    );
    assert_eq!(
        ifc.tree_state.node[seg3].cost, 1,
        "Third segment should have cost 1"
    );

    // Fourth segment: unmatched (DIFF2)
    assert!(
        ifc.tree_state.node[seg4].cost < 0,
        "Fourth segment should be unmatched"
    );
    assert_eq!(
        ifc.tree_state.node[seg4].cost, -1,
        "Fourth segment should have cost -1"
    );
}

#[test]
fn test_pass5_header_nodes() {
    // Test header node creation
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Header nodes should have cost 0, line 0
    let header1 = ifc.tree_state.trees[0].start;
    let header2 = ifc.tree_state.trees[1].start;

    assert_eq!(
        ifc.tree_state.node[header1].cost, 0,
        "Header should have cost 0"
    );
    assert_eq!(
        ifc.tree_state.node[header1].linen, 0,
        "Header should have line 0"
    );
    assert_eq!(
        ifc.tree_state.node[header2].cost, 0,
        "Header should have cost 0"
    );
    assert_eq!(
        ifc.tree_state.node[header2].linen, 0,
        "Header should have line 0"
    );
}

#[test]
fn test_pass5_trailer_nodes() {
    // Test trailer node creation
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Trailer nodes should have cost 0, line (total_lines + 1)
    let trailer1 = ifc.tree_state.trees[0].end;
    let trailer2 = ifc.tree_state.trees[1].end;

    assert_eq!(
        ifc.tree_state.node[trailer1].cost, 0,
        "Trailer should have cost 0"
    );
    assert_eq!(
        ifc.tree_state.node[trailer1].linen, 2,
        "Trailer should have line 2 (total_lines + 1)"
    );
    assert_eq!(
        ifc.tree_state.node[trailer2].cost, 0,
        "Trailer should have cost 0"
    );
    assert_eq!(
        ifc.tree_state.node[trailer2].linen, -2,
        "File2 trailer should have negative line -2"
    );
}

#[test]
fn test_pass5_doubly_linked_list() {
    // Test doubly-linked list structure
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\nDIFF1\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nOTHER1\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let header = ifc.tree_state.trees[0].start;
    let seg1 = ifc.tree_state.node[header].next;
    let seg2 = ifc.tree_state.node[seg1].next;
    let seg3 = ifc.tree_state.node[seg2].next;
    let trailer = ifc.tree_state.trees[0].end;

    // Forward links
    assert_eq!(
        ifc.tree_state.node[header].next, seg1,
        "Header should point to seg1"
    );
    assert_eq!(
        ifc.tree_state.node[seg1].next, seg2,
        "Seg1 should point to seg2"
    );
    assert_eq!(
        ifc.tree_state.node[seg2].next, seg3,
        "Seg2 should point to seg3"
    );
    assert_eq!(
        ifc.tree_state.node[seg3].next, trailer,
        "Seg3 should point to trailer"
    );

    // Backward links
    assert_eq!(
        ifc.tree_state.node[seg1].prev, header,
        "Seg1 should point back to header"
    );
    assert_eq!(
        ifc.tree_state.node[seg2].prev, seg1,
        "Seg2 should point back to seg1"
    );
    assert_eq!(
        ifc.tree_state.node[seg3].prev, seg2,
        "Seg3 should point back to seg2"
    );
    assert_eq!(
        ifc.tree_state.node[trailer].prev, seg3,
        "Trailer should point back to seg3"
    );
}

#[test]
fn test_pass5_file2_negative_line_numbers() {
    // Test that file2 nodes have negative line numbers
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let seg1 = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;
    let seg2 = ifc.tree_state.node[ifc.tree_state.trees[1].start].next;

    // File1 should have positive line number
    assert!(
        ifc.tree_state.node[seg1].linen > 0,
        "File1 segment should have positive line number"
    );
    assert_eq!(
        ifc.tree_state.node[seg1].linen, 1,
        "File1 segment should have line 1"
    );

    // File2 should have negative line number
    assert!(
        ifc.tree_state.node[seg2].linen < 0,
        "File2 segment should have negative line number"
    );
    assert_eq!(
        ifc.tree_state.node[seg2].linen, -1,
        "File2 segment should have line -1"
    );
}

#[test]
fn test_pass5_consecutive_unmatched_lines() {
    // Multiple consecutive unmatched lines should form single segment
    let mut ifc = setup_pass5_test();
    let file1 = "DIFF1\nDIFF2\nDIFF3\nDIFF4\n".as_bytes();
    let file2 = "OTHER1\nOTHER2\nOTHER3\nOTHER4\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let header = ifc.tree_state.trees[0].start;
    let segment = ifc.tree_state.node[header].next;

    // Should be single segment with all 4 lines
    assert_eq!(
        ifc.tree_state.node[segment].cost, -4,
        "Four consecutive unmatched lines should form single segment with cost -4"
    );
    assert!(
        ifc.tree_state.node[segment].cost < 0,
        "Should have negative cost"
    );
}

#[test]
fn test_pass5_consecutive_matched_lines() {
    // Multiple consecutive matched lines should form single segment
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let header = ifc.tree_state.trees[0].start;
    let segment = ifc.tree_state.node[header].next;

    // Should be single segment with all 4 lines
    assert_eq!(
        ifc.tree_state.node[segment].cost, 4,
        "Four consecutive matched lines should form single segment with cost 4"
    );
    assert!(
        ifc.tree_state.node[segment].cost > 0,
        "Should have positive cost"
    );
}

#[test]
fn test_pass5_mixed_with_pass3_pass4_extension() {
    // Test with pass3 and pass4 extensions
    let mut ifc = setup_pass5_test();
    let file1 = "COMMON\nCOMMON\nUNIQUE_A\nCOMMON\nCOMMON\n".as_bytes();
    let file2 = "COMMON\nCOMMON\nUNIQUE_A\nCOMMON\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3(); // Forward extension
    ifc.pass4(); // Backward extension
    ifc.pass5();

    // After pass3/4, all COMMON lines should be MATCH_TYPE
    // Should have single segment with cost 5
    let header = ifc.tree_state.trees[0].start;
    let segment = ifc.tree_state.node[header].next;

    assert_eq!(
        ifc.tree_state.node[segment].cost, 5,
        "All 5 lines should be in single matched segment with cost 5"
    );
    assert!(
        ifc.tree_state.node[segment].cost > 0,
        "Should have positive cost"
    );
}

#[test]
fn test_pass5_empty_lines_files() {
    // Test with single empty line
    let mut ifc = setup_pass5_test();
    let file1 = "\n".as_bytes();
    let file2 = "\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Should still have header and trailer
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have header"
    );
    assert_ne!(
        ifc.tree_state.trees[0].end, NULL_NODE,
        "Should have trailer"
    );

    // Should have at least one segment for the empty line
    let header = ifc.tree_state.trees[0].start;
    assert_ne!(
        ifc.tree_state.node[header].next, NULL_NODE,
        "Should have segment after header"
    );
}

#[test]
fn test_pass5_complex_pattern() {
    // Complex pattern with multiple segments
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\nDIFF1\nDIFF2\nUNIQUE_B\nCOMMON\nCOMMON\nDIFF3\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nOTHER1\nOTHER2\nUNIQUE_B\nCOMMON\nCOMMON\nOTHER3\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();

    // Count segments
    let mut segment_count = 0;
    let mut current = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;
    while current != ifc.tree_state.trees[0].end {
        segment_count += 1;
        current = ifc.tree_state.node[current].next;
    }

    // Should have multiple segments (matched, unmatched, matched, etc.)
    assert!(
        segment_count > 1,
        "Complex pattern should create multiple segments"
    );
}

#[test]
fn test_pass5_single_segment_all_matched() {
    // All lines match - should have single segment
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let header = ifc.tree_state.trees[0].start;
    let segment = ifc.tree_state.node[header].next;
    let trailer = ifc.tree_state.trees[0].end;

    // Should have only one segment between header and trailer
    assert_eq!(
        ifc.tree_state.node[segment].next, trailer,
        "Should have single segment"
    );
    assert_eq!(
        ifc.tree_state.node[segment].cost, 3,
        "Single segment should contain all 3 lines with cost 3"
    );
}

#[test]
fn test_pass5_single_segment_all_unmatched() {
    // All lines unmatched - should have single segment
    let mut ifc = setup_pass5_test();
    let file1 = "DIFF1\nDIFF2\nDIFF3\n".as_bytes();
    let file2 = "OTHER1\nOTHER2\nOTHER3\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let header = ifc.tree_state.trees[0].start;
    let segment = ifc.tree_state.node[header].next;
    let trailer = ifc.tree_state.trees[0].end;

    // Should have only one segment between header and trailer
    assert_eq!(
        ifc.tree_state.node[segment].next, trailer,
        "Should have single segment"
    );
    assert_eq!(
        ifc.tree_state.node[segment].cost, -3,
        "Single segment should contain all 3 unmatched lines with cost -3"
    );
    assert!(
        ifc.tree_state.node[segment].cost < 0,
        "Should have negative cost"
    );
}

#[test]
fn test_pass5_header_trailer_links() {
    // Test that headers and trailers are linked correctly
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Headers should reference each other (line 0)
    assert_eq!(
        ifc.file_state.file_line[0][0].ptr0, 0,
        "File1 header should point to file2 header"
    );
    assert_eq!(
        ifc.file_state.file_line[1][0].ptr0, 0,
        "File2 header should point to file1 header"
    );

    // Trailers should reference each other
    let file1_t_lines_p = ifc.file_state.total_file_n_lines[0] + 1;
    let file2_t_lines_p = ifc.file_state.total_file_n_lines[1] + 1;
    assert_eq!(
        ifc.file_state.file_line[0][file1_t_lines_p].ptr0, file2_t_lines_p as LineCount,
        "File1 trailer should point to file2 trailer"
    );
    assert_eq!(
        ifc.file_state.file_line[1][file2_t_lines_p].ptr0, file1_t_lines_p as LineCount,
        "File2 trailer should point to file1 trailer"
    );
}

#[test]
fn test_pass5_branch_start_end_initially_null() {
    // Test that branch_start and branch_end are initially NULL_NODE
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let segment = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;

    assert_eq!(
        ifc.tree_state.node[segment].branch_start, NULL_NODE,
        "Initially should have no branch_start"
    );
    assert_eq!(
        ifc.tree_state.node[segment].branch_end, NULL_NODE,
        "Initially should have no branch_end"
    );
    assert!(
        ifc.leaf(segment),
        "Should be a leaf node initially"
    );
}

#[test]
fn test_pass5_segment_line_numbers() {
    // Test that segment line numbers are correct
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\nDIFF1\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nOTHER1\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let seg1 = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;
    let seg2 = ifc.tree_state.node[seg1].next;
    let seg3 = ifc.tree_state.node[seg2].next;

    // First segment starts at line 1
    assert_eq!(
        ifc.tree_state.node[seg1].linen, 1,
        "First segment should start at line 1"
    );

    // Second segment starts at line 2
    assert_eq!(
        ifc.tree_state.node[seg2].linen, 2,
        "Second segment should start at line 2"
    );

    // Third segment starts at line 3
    assert_eq!(
        ifc.tree_state.node[seg3].linen, 3,
        "Third segment should start at line 3"
    );
}

#[test]
fn test_pass5_matched_segment_consecutive_ptr0() {
    // Test that matched segments require consecutive ptr0 values
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let segment = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;

    // Segment should have cost 2 (both lines matched consecutively)
    assert_eq!(
        ifc.tree_state.node[segment].cost, 2,
        "Consecutive matched lines should form single segment with cost 2"
    );

    // Verify they're in same segment by checking line numbers
    assert_eq!(
        ifc.tree_state.node[segment].linen, 1,
        "Segment starts at line 1"
    );
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

#[test]
fn test_pass5_large_number_of_segments() {
    // Many alternating matched/unmatched segments
    let mut ifc = setup_pass5_test();
    let mut file1_content = String::new();
    let mut file2_content = String::new();
    for i in 0..50 {
        if i % 2 == 0 {
            file1_content.push_str(&format!("UNIQUE{}\n", i));
            file2_content.push_str(&format!("UNIQUE{}\n", i));
        } else {
            file1_content.push_str(&format!("DIFF{}\n", i));
            file2_content.push_str(&format!("OTHER{}\n", i));
        }
    }

    let file1 = file1_content.as_bytes();
    let file2 = file2_content.as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Should create many segments
    let mut segment_count = 0;
    let mut current = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;
    while current != ifc.tree_state.trees[0].end {
        segment_count += 1;
        current = ifc.tree_state.node[current].next;
    }

    assert!(
        segment_count > 25,
        "Should create many segments for alternating pattern"
    );
}

#[test]
fn test_pass5_very_long_segment() {
    // Very long segment of matched lines
    let mut ifc = setup_pass5_test();
    let mut file1_content = String::new();
    let mut file2_content = String::new();
    for i in 0..100 {
        file1_content.push_str(&format!("UNIQUE{}\n", i));
        file2_content.push_str(&format!("UNIQUE{}\n", i));
    }

    let file1 = file1_content.as_bytes();
    let file2 = file2_content.as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let header = ifc.tree_state.trees[0].start;
    let segment = ifc.tree_state.node[header].next;

    // Should have single segment with all 100 lines
    assert_eq!(
        ifc.tree_state.node[segment].cost, 100,
        "Single segment should contain all 100 lines with cost 100"
    );
}

#[test]
fn test_pass5_both_files_same_structure() {
    // Both files should have similar tree structure
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\nDIFF1\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nOTHER1\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Count segments in both files
    let mut file1_segments = 0;
    let mut current1 = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;
    while current1 != ifc.tree_state.trees[0].end {
        file1_segments += 1;
        current1 = ifc.tree_state.node[current1].next;
    }

    let mut file2_segments = 0;
    let mut current2 = ifc.tree_state.node[ifc.tree_state.trees[1].start].next;
    while current2 != ifc.tree_state.trees[1].end {
        file2_segments += 1;
        current2 = ifc.tree_state.node[current2].next;
    }

    assert_eq!(
        file1_segments, file2_segments,
        "Both files should have same number of segments"
    );
    assert_eq!(
        file1_segments, 3,
        "Should have 3 segments (matched, unmatched, matched)"
    );
}

#[test]
fn test_pass5_each_line_in_node_matched_segment() {
    // Test each_line_in_node with matched segment
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let segment = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;

    let mut line_count = 0;
    ifc.each_line_in_node(segment, false, 0, |which_file: FileIndex, _text: &str, _lineno: usize| {
        line_count += 1;
        assert_eq!(which_file, FileIndex::First, "Should be first file");
    });

    assert_eq!(
        line_count, 3,
        "Should iterate over 3 lines"
    );
}

#[test]
fn test_pass5_each_line_in_node_unmatched_segment() {
    // Test each_line_in_node with unmatched segment
    let mut ifc = setup_pass5_test();
    let file1 = "DIFF1\nDIFF2\n".as_bytes();
    let file2 = "OTHER1\nOTHER2\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let segment = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;

    let mut line_count = 0;
    ifc.each_line_in_node(segment, true, 0, |which_file: FileIndex, _text: &str, _lineno: usize| {
        line_count += 1;
        assert_eq!(which_file, FileIndex::First, "Should be first file");
    });

    assert_eq!(
        line_count, 2,
        "Should iterate over 2 lines (always=true uses absolute cost)"
    );
}

#[test]
fn test_pass5_each_line_in_node_starting_line() {
    // Test each_line_in_node with starting_line parameter
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let segment = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;

    let mut line_count = 0;
    ifc.each_line_in_node(segment, false, 3, |_which_file: FileIndex, _text: &str, lineno: usize| {
        line_count += 1;
        assert!(
            lineno >= 3,
            "Should start from line 3"
        );
    });

    assert_eq!(
        line_count, 2,
        "Should iterate over 2 lines (starting from line 3)"
    );
}

#[test]
fn test_pass5_count_node_matched() {
    // Test count_node with matched segment
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let segment = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;

    let mut kinds = LineKinds::default();
    ifc.count_node(segment, &mut kinds);

    assert_eq!(
        kinds.non_cosmetic, 3,
        "Should count 3 non-cosmetic lines"
    );
    // Should have no cosmetic lines (cosmetic_line always returns false)
    assert_eq!(
        kinds.cosmetic, 0,
        "Should have no cosmetic lines"
    );
}

#[test]
fn test_pass5_count_node_unmatched() {
    // Test count_node with unmatched segment
    // Note: count_node uses each_line_in_node with always=false, so negative cost segments
    // don't iterate (last = sline + cost where cost < 0, so last < sline, loop doesn't execute)
    let mut ifc = setup_pass5_test();
    let file1 = "DIFF1\nDIFF2\n".as_bytes();
    let file2 = "OTHER1\nOTHER2\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let segment = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;

    let mut kinds = LineKinds::default();
    ifc.count_node(segment, &mut kinds);

    // With always=false, negative cost segments don't iterate, so count should be 0
    assert_eq!(
        kinds.non_cosmetic, 0,
        "Unmatched segments with negative cost don't iterate when always=false"
    );
    assert_eq!(
        kinds.cosmetic, 0,
        "Should have no cosmetic lines"
    );
}

#[test]
fn test_pass5_node_with_zero_cost() {
    // Test nodes with zero cost (header and trailer)
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let header = ifc.tree_state.trees[0].start;
    let trailer = ifc.tree_state.trees[0].end;

    assert_eq!(
        ifc.tree_state.node[header].cost, 0,
        "Header should have cost 0"
    );
    assert_eq!(
        ifc.tree_state.node[trailer].cost, 0,
        "Trailer should have cost 0"
    );
}

#[test]
fn test_pass5_discontinuous_matched_lines() {
    // Matched lines that are not consecutive in terms of ptr0
    let mut ifc = setup_pass5_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nOTHER\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // UNIQUE_A and UNIQUE_B should be separate segments (ptr0 not consecutive)
    let header = ifc.tree_state.trees[0].start;
    let seg1 = ifc.tree_state.node[header].next;
    let seg2 = ifc.tree_state.node[seg1].next;

    // Each should be separate segment
    assert_eq!(
        ifc.tree_state.node[seg1].cost, 1,
        "First unique should be separate segment with cost 1"
    );
    assert_eq!(
        ifc.tree_state.node[seg2].cost, 1,
        "Second unique should be separate segment with cost 1"
    );
}

#[test]
fn test_pass5_identical_files() {
    // Identical files should have single matched segment
    let mut ifc = setup_pass5_test();
    let file1 = "A\nB\nC\n".as_bytes();
    let file2 = "A\nB\nC\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let header = ifc.tree_state.trees[0].start;
    let segment = ifc.tree_state.node[header].next;
    let trailer = ifc.tree_state.trees[0].end;

    // Should have single segment between header and trailer
    assert_eq!(
        ifc.tree_state.node[segment].next, trailer,
        "Should have single segment"
    );
    assert!(
        ifc.tree_state.node[segment].cost > 0,
        "Should be matched segment"
    );
}

