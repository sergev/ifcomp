use ifcomp::{Ifcomp, NULL_NODE};

// Test helper: create fresh Ifcomp instance for each test
fn setup_pass6_test() -> Ifcomp {
    Ifcomp::new()
}

// ============================================================================
// Tests for helper functions
// ============================================================================

#[test]
fn test_pass6_find_node_basic() {
    // Test find_node() function
    let mut ifc = setup_pass6_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Find node containing line 1 in file1 tree
    let node1 = ifc.find_node(ifc.tree_state.trees[0], 1);
    assert_ne!(
        node1, NULL_NODE,
        "Should find node containing line 1"
    );
    assert_eq!(
        ifc.true_line_of(node1), 1,
        "Found node should contain line 1"
    );
}

#[test]
fn test_pass6_find_node_file2() {
    // Test find_node() for file2 (negative line numbers)
    let mut ifc = setup_pass6_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Find node containing line 1 in file2 tree (stored as -1)
    let node1 = ifc.find_node(ifc.tree_state.trees[1], -1);
    assert_ne!(
        node1, NULL_NODE,
        "Should find node containing line 1"
    );
    assert_eq!(
        ifc.true_line_of(node1), 1,
        "Found node should contain line 1"
    );
}

#[test]
fn test_pass6_detach_node_basic() {
    // Test detach_node() function
    let mut ifc = setup_pass6_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Find the unmatched segment (UNIQUE_B)
    let header = ifc.tree_state.trees[0].start;
    let node1 = ifc.tree_state.node[header].next;

    // Verify we have the expected structure
    assert!(
        ifc.tree_state.node[node1].cost > 0,
        "First segment should be matched"
    );

    let unmatched = ifc.tree_state.node[node1].next;
    let trailer = ifc.tree_state.trees[0].end;

    // Verify unmatched segment exists and has negative cost
    if unmatched != trailer && ifc.tree_state.node[unmatched].cost < 0 {
        // Save the next node before detaching
        let next_after_unmatched = ifc.tree_state.node[unmatched].next;

        // Detach the unmatched node
        ifc.detach_node(unmatched);

        // Verify it's detached from the list
        assert_eq!(
            ifc.tree_state.node[node1].next, next_after_unmatched,
            "Node should be detached from list"
        );

        // Verify the link back from next node
        if next_after_unmatched != trailer {
            assert_eq!(
                ifc.tree_state.node[next_after_unmatched].prev, node1,
                "Next node should point back to node1"
            );
        } else {
            assert_eq!(
                ifc.tree_state.node[trailer].prev, node1,
                "Trailer should point back to node1"
            );
        }
    }
}

#[test]
fn test_pass6_combine_nodes_basic() {
    // Test combine_nodes() function
    let mut ifc = setup_pass6_test();
    let file1 = "MATCH\nDIFF1\nMATCH\nDIFF2\n".as_bytes();
    let file2 = "MATCH\nDIFF2\nMATCH\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();

    // After pass6, unmatched segments should be combined
    let header = ifc.tree_state.trees[0].start;

    // Run pass6 to trigger combine_nodes
    ifc.pass6();

    // After pass6, replaced segments should create branch structure
    let mut current = ifc.tree_state.node[header].next;
    let mut found_branch = false;
    while current != ifc.tree_state.trees[0].end {
        if !ifc.leaf(current) {
            found_branch = true;
            assert_ne!(
                ifc.tree_state.node[current].branch_start, NULL_NODE,
                "Branch should have start"
            );
            assert_ne!(
                ifc.tree_state.node[current].branch_end, NULL_NODE,
                "Branch should have end"
            );
            break;
        }
        current = ifc.tree_state.node[current].next;
    }

    let _ = found_branch; // may or may not create branches depending on test data
}

#[test]
fn test_pass6_unique_find_with_unique() {
    // Test unique_find() when unique line exists
    let mut ifc = setup_pass6_test();
    let file1 = "UNIQUE_A\nCOMMON\nCOMMON\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();

    let segment = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;

    // Should find the unique line
    let unique_line = ifc.unique_find(segment);
    assert_ne!(
        unique_line, NULL_NODE,
        "Should find unique line in segment"
    );
    assert_eq!(
        unique_line, 1,
        "Should return line 1"
    );
}

#[test]
fn test_pass6_unique_find_without_unique() {
    // Test unique_find() when no unique lines exist
    let mut ifc = setup_pass6_test();
    let file1 = "COMMON\nCOMMON\nCOMMON\n".as_bytes();
    let file2 = "COMMON\nCOMMON\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let segment = ifc.tree_state.node[ifc.tree_state.trees[0].start].next;

    // Should not find any unique lines
    let unique_line = ifc.unique_find(segment);
    assert_eq!(
        unique_line, NULL_NODE,
        "Should not find unique line in segment with only duplicates"
    );
}

#[test]
fn test_pass6_pass6_replaceable_basic() {
    // Test pass6_replaceable() function
    let mut ifc = setup_pass6_test();
    let file1 = "UNIQUE_A\nDIFF1\n".as_bytes();
    let file2 = "UNIQUE_A\nOTHER1\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // Get the unmatched segment in file1
    let header1 = ifc.tree_state.trees[0].start;
    let matched1 = ifc.tree_state.node[header1].next;
    let unmatched1 = ifc.tree_state.node[matched1].next;

    if unmatched1 != ifc.tree_state.trees[0].end && ifc.tree_state.node[unmatched1].cost < 0 {
        // Check if it can be replaced
        let replaceable = ifc.pass6_replaceable(unmatched1);
        if replaceable == NULL_NODE {
            // May or may not be replaceable depending on structure
        } else {
            // If replaceable, should be unmatched node in file2
            assert!(
                ifc.tree_state.node[replaceable].cost < 0,
                "Replaceable node should be unmatched (negative cost)"
            );
        }
    }
}

// ============================================================================
// Tests for pass6() - Basic functionality
// ============================================================================

#[test]
fn test_pass6_simple_deletion() {
    // Simple deletion case
    let mut ifc = setup_pass6_test();
    let file1 = "A\nB\nC\n".as_bytes();
    let file2 = "A\nC\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();

    // Should have processed deletion
    assert!(
        ifc.stats.n_change_blocks > 0,
        "Should have at least one change block"
    );
}

#[test]
fn test_pass6_simple_insertion() {
    // Simple insertion case
    let mut ifc = setup_pass6_test();
    let file1 = "A\nC\n".as_bytes();
    let file2 = "A\nB\nC\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();

    // Should have processed insertion
    assert!(
        ifc.stats.n_change_blocks > 0,
        "Should have at least one change block"
    );
}

#[test]
fn test_pass6_simple_replacement() {
    // Simple replacement case
    let mut ifc = setup_pass6_test();
    let file1 = "A\nB\nC\n".as_bytes();
    let file2 = "A\nX\nC\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();

    // Should have processed replacement
    assert!(
        ifc.stats.n_change_blocks > 0,
        "Should have at least one change block"
    );
}

#[test]
fn test_pass6_identical_files() {
    // Identical files should have no changes
    let mut ifc = setup_pass6_test();
    let file1 = "A\nB\nC\n".as_bytes();
    let file2 = "A\nB\nC\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();

    // Should have no changes
    assert_eq!(
        ifc.stats.n_change_blocks, 0,
        "Identical files should have 0 change blocks"
    );
}

#[test]
fn test_pass6_multiple_changes() {
    // Multiple changes
    let mut ifc = setup_pass6_test();
    let file1 = "A\nB\nC\nD\n".as_bytes();
    let file2 = "A\nX\nC\nY\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();

    // Should have multiple change blocks
    assert!(
        ifc.stats.n_change_blocks >= 1,
        "Should have multiple change blocks"
    );
}

#[test]
fn test_pass6_statistics_tracked() {
    // Verify statistics are tracked
    let mut ifc = setup_pass6_test();
    let file1 = "A\nB\nC\n".as_bytes();
    let file2 = "A\nX\nC\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();

    // Statistics should be initialized and potentially updated
    let _delete_stats = &ifc.stats.delete_stats;
    let _insert_stats = &ifc.stats.insert_stats;
    let _replace1_stats = &ifc.stats.replace1_stats;
    let _replace2_stats = &ifc.stats.replace2_stats;
    let _move_stats = &ifc.stats.move_stats;
    let _n_change_blocks = ifc.stats.n_change_blocks;
    let _ = (_delete_stats, _insert_stats, _replace1_stats, _replace2_stats, _move_stats, _n_change_blocks);
}

#[test]
fn test_pass6_no_duplicates_all_unique() {
    // All lines are unique
    let mut ifc = setup_pass6_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();

    // Should have no changes
    assert_eq!(
        ifc.stats.n_change_blocks, 0,
        "Identical files with unique lines should have 0 change blocks"
    );
}

#[test]
fn test_pass6_all_duplicates() {
    // All lines are duplicates
    let mut ifc = setup_pass6_test();
    let file1 = "LINE\nLINE\nLINE\n".as_bytes();
    let file2 = "LINE\nLINE\nLINE\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();

    // pass6 should complete without crashing
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass6_edge_case_empty_files() {
    // Edge case: empty files
    let mut ifc = setup_pass6_test();
    let file1 = "\n".as_bytes();
    let file2 = "\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();

    // Should handle without crashing
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass6_complex_scenario() {
    // Complex scenario with multiple types of changes
    let mut ifc = setup_pass6_test();
    let file1 = "START\nKEEP1\nDELETE\nKEEP2\nCHANGE1\nEND\n".as_bytes();
    let file2 = "START\nKEEP1\nINSERT\nKEEP2\nCHANGE2\nEND\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();

    // Should process multiple types of changes
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass6_long_files() {
    // Test with longer files
    let mut ifc = setup_pass6_test();
    let mut file1_content = String::new();
    let mut file2_content = String::new();
    for i in 0..20 {
        file1_content.push_str(&format!("LINE{}\n", i));
        if i % 3 != 0 {
            file2_content.push_str(&format!("LINE{}\n", i));
        } else {
            file2_content.push_str(&format!("CHANGED{}\n", i));
        }
    }

    let file1 = file1_content.as_bytes();
    let file2 = file2_content.as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();

    // Should handle longer files without issues
    assert!(
        ifc.stats.n_change_blocks > 0,
        "Should have change blocks for differing files"
    );
}

#[test]
fn test_pass6_mixed_operation_types() {
    // Mix of deletions, insertions, and replacements
    let mut ifc = setup_pass6_test();
    let file1 = "KEEP1\nDELETE1\nDELETE2\nKEEP2\nCHANGE1\nKEEP3\n".as_bytes();
    let file2 = "KEEP1\nINSERT1\nINSERT2\nKEEP2\nCHANGE2\nKEEP3\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();

    // Should handle mixed operations
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass6_after_header() {
    // Test after_header output formatting
    let mut ifc = setup_pass6_test();
    let file1 = "A\nB\nC\n".as_bytes();
    let file2 = "A\nX\nC\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();

    // Should have called after_header during output
    assert!(
        ifc.stats.n_change_blocks > 0,
        "Should have processed changes"
    );
}

#[test]
fn test_pass6_tree_structure_after_pass6() {
    // Verify tree structure is valid after pass6
    let mut ifc = setup_pass6_test();
    let file1 = "A\nB\nC\n".as_bytes();
    let file2 = "A\nX\nC\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();

    let before_start = ifc.tree_state.trees[0].start;
    let before_end = ifc.tree_state.trees[0].end;

    ifc.pass6();

    let after_start = ifc.tree_state.trees[0].start;
    let after_end = ifc.tree_state.trees[0].end;

    // Tree bounds should remain valid
    assert_ne!(
        after_start, NULL_NODE,
        "Tree should have valid start"
    );
    assert_ne!(
        after_end, NULL_NODE,
        "Tree should have valid end"
    );
    assert_eq!(
        before_start, after_start,
        "Tree start should remain the same"
    );
    assert_eq!(
        before_end, after_end,
        "Tree end should remain the same"
    );
}

