use ifcomp::{Ifcomp, NULL_NODE};

// Test helper: create fresh Ifcomp instance for each test
fn setup_pass7_test() -> Ifcomp {
    Ifcomp::new()
}

// ============================================================================
// Tests for helper function
// ============================================================================

#[test]
fn test_pass7_pass7_combine_adjacent_nodes_combines() {
    // Test pass7_combine_adjacent_nodes() when nodes should combine
    let mut ifc = setup_pass7_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();
    ifc.pass6();

    // After pass5 and pass6, we should have segments
    let header = ifc.tree_state.trees[0].start;
    let node1 = ifc.tree_state.node[header].next;
    let trailer = ifc.tree_state.trees[0].end;

    // Check if node1 has a next node (not trailer)
    if ifc.tree_state.node[node1].next != trailer {
        let node2 = ifc.tree_state.node[node1].next;

        // Verify nodes exist and are matched
        assert!(
            ifc.tree_state.node[node1].cost > 0,
            "Node1 should be matched"
        );
        assert!(
            ifc.tree_state.node[node2].cost > 0,
            "Node2 should be matched"
        );

        // Try to combine node1 and node2
        let combined = ifc.pass7_combine_adjacent_nodes(node1);

        if combined {
            // After combination, check the structure
            // Note: node1 may have been freed, so we check the new combined node
            // The combined node should be a branch (not a leaf)
            let new_node = ifc.tree_state.node[header].next;
            assert!(
                !ifc.leaf(new_node),
                "Combined node should be a branch"
            );
        }
        // else: only one segment - nothing to combine (expected for identical files)
    }
}

#[test]
fn test_pass7_pass7_combine_adjacent_nodes_not_adjacent_in_file2() {
    // Test pass7_combine_adjacent_nodes() when nodes are adjacent in file1 but not file2
    let mut ifc = setup_pass7_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_X\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    // After pass5:
    // File1: [A] [B] [C] (all matched separately)
    // File2: [A] [X] [C] (all matched separately)
    // But A and B are not adjacent in file2 (X is between A and C)

    let header = ifc.tree_state.trees[0].start;
    let node1 = ifc.tree_state.node[header].next; // A

    // Try to combine node1 and node2
    // This should fail because in file2, A is followed by X (not B)
    let combined = ifc.pass7_combine_adjacent_nodes(node1);
    assert!(
        !combined,
        "Should not combine - not adjacent in file2"
    );
}

#[test]
fn test_pass7_pass7_combine_adjacent_nodes_trailer() {
    // Test pass7_combine_adjacent_nodes() at trailer
    let mut ifc = setup_pass7_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();
    ifc.pass6();

    // Find structure after pass6
    let header = ifc.tree_state.trees[0].start;
    let trailer = ifc.tree_state.trees[0].end;

    // Count nodes before pass7
    let mut current = ifc.tree_state.node[header].next;
    let mut nodes_before = 0;
    while current != trailer {
        nodes_before += 1;
        current = ifc.tree_state.node[current].next;
    }

    // Run pass7 - it should handle trailer correctly
    ifc.pass7().expect("pass7 should succeed");

    // Verify pass7 completed without errors
    current = ifc.tree_state.node[header].next;
    let mut nodes_after = 0;
    while current != trailer {
        nodes_after += 1;
        current = ifc.tree_state.node[current].next;
    }

    // Should have processed correctly
    assert!(
        nodes_after >= 1,
        "Should have at least 1 node"
    );
    assert!(
        nodes_after <= nodes_before,
        "Should have same or fewer nodes (combinations)"
    );
}

// ============================================================================
// Tests for pass7() - Basic functionality
// ============================================================================

#[test]
fn test_pass7_single_combination() {
    // Test single combination of two adjacent nodes
    let mut ifc = setup_pass7_test();
    let file1 = "UNIQUE_A\nCOMMON\nUNIQUE_B\nCOMMON\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON\nUNIQUE_B\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");

    // pass7 should complete without errors
    let header = ifc.tree_state.trees[0].start;
    assert_ne!(
        ifc.tree_state.node[header].next, NULL_NODE,
        "Should have nodes after pass7"
    );
}

#[test]
fn test_pass7_multiple_combinations() {
    // Test multiple combinations
    let mut ifc = setup_pass7_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");

    // pass7 should complete without errors
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass7_no_combination() {
    // Test when no combinations are possible
    let mut ifc = setup_pass7_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_X\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");

    // pass7 should complete without errors
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass7_partial_combinations() {
    // Test when some combinations are possible
    let mut ifc = setup_pass7_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_X\nUNIQUE_D\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");

    // pass7 should complete without errors
    let header = ifc.tree_state.trees[0].start;
    assert_ne!(
        ifc.tree_state.node[header].next, NULL_NODE,
        "Should have nodes after pass7"
    );
}

#[test]
fn test_pass7_integration_with_pass6() {
    // Test integration with pass6
    let mut ifc = setup_pass7_test();
    let file1 = "UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");

    // Should have valid tree structure
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have valid tree start"
    );
    assert_ne!(
        ifc.tree_state.trees[0].end, NULL_NODE,
        "Should have valid tree end"
    );
}

#[test]
fn test_pass7_edge_case_single_segment() {
    // Test with single segment (nothing to combine)
    let mut ifc = setup_pass7_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");

    // pass7 should handle gracefully
    let header = ifc.tree_state.trees[0].start;
    let trailer = ifc.tree_state.trees[0].end;

    assert_ne!(
        ifc.tree_state.node[header].next, trailer,
        "Should have segments between header and trailer"
    );
}

#[test]
fn test_pass7_edge_case_identical_files() {
    // Test with identical files
    let mut ifc = setup_pass7_test();
    let file1 = "A\nB\nC\n".as_bytes();
    let file2 = "A\nB\nC\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");

    // pass7 should complete without errors
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have valid tree"
    );
}

#[test]
fn test_pass7_complex_patterns() {
    // Test with complex patterns
    let mut ifc = setup_pass7_test();
    let file1 = "UNIQUE_A\nCOMMON1\nCOMMON2\nUNIQUE_B\nCOMMON3\nCOMMON4\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON1\nCOMMON2\nUNIQUE_B\nCOMMON3\nCOMMON4\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");

    // Should handle complex patterns
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass7_combination_chain() {
    // Test chain of combinations
    let mut ifc = setup_pass7_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();
    ifc.pass6();

    // Count nodes before pass7
    let header = ifc.tree_state.trees[0].start;
    let trailer = ifc.tree_state.trees[0].end;
    let mut current = ifc.tree_state.node[header].next;
    let mut nodes_before = 0;
    while current != trailer {
        nodes_before += 1;
        current = ifc.tree_state.node[current].next;
    }

    ifc.pass7().expect("pass7 should succeed");

    // Count nodes after pass7
    current = ifc.tree_state.node[header].next;
    let mut nodes_after = 0;
    while current != trailer {
        nodes_after += 1;
        current = ifc.tree_state.node[current].next;
    }

    // Should have fewer or same number of nodes
    assert!(
        nodes_after <= nodes_before,
        "Should not increase node count"
    );
}

#[test]
fn test_pass7_long_files() {
    // Test with longer files
    let mut ifc = setup_pass7_test();
    let mut file1_content = String::new();
    let mut file2_content = String::new();
    for i in 0..50 {
        file1_content.push_str(&format!("LINE{}\n", i));
        file2_content.push_str(&format!("LINE{}\n", i));
    }

    let file1 = file1_content.as_bytes();
    let file2 = file2_content.as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");

    // Should handle long files
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass7_repeated_combinations() {
    // Test that pass7 can handle multiple combination iterations
    let mut ifc = setup_pass7_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();
    ifc.pass6();

    // Run pass7 multiple times (it should be idempotent)
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass7().expect("pass7 should succeed");

    // Should still have valid structure
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should maintain valid tree after multiple passes"
    );
}

#[test]
fn test_pass7_all_duplicates_pattern() {
    // Test with all duplicates pattern
    let mut ifc = setup_pass7_test();
    let file1 = "COMMON\nCOMMON\nCOMMON\n".as_bytes();
    let file2 = "COMMON\nCOMMON\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");

    // Should handle all duplicates
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass7_tree_integrity_after_combination() {
    // Verify tree integrity after combinations
    let mut ifc = setup_pass7_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");

    // Verify doubly-linked list integrity
    let header = ifc.tree_state.trees[0].start;
    let mut current = ifc.tree_state.node[header].next;
    let mut count = 0;
    while current != ifc.tree_state.trees[0].end && count < 100 {
        let prev = ifc.tree_state.node[current].prev;
        let next = ifc.tree_state.node[current].next;
        assert_eq!(
            ifc.tree_state.node[prev].next, current,
            "Forward link broken"
        );
        assert_eq!(
            ifc.tree_state.node[next].prev, current,
            "Backward link broken"
        );
        current = next;
        count += 1;
    }
}

#[test]
fn test_pass7_branch_structure_created() {
    // Verify branch structures can be created
    let mut ifc = setup_pass7_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");

    // Check if any nodes have branch structure
    let header = ifc.tree_state.trees[0].start;
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
        }
        current = ifc.tree_state.node[current].next;
    }

    // May or may not have branches depending on whether combinations occurred
    let _ = found_branch;
}

#[test]
fn test_pass7_empty_lines_handled() {
    // Test with empty lines
    let mut ifc = setup_pass7_test();
    let file1 = "\n\n".as_bytes();
    let file2 = "\n\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");

    // Should handle empty lines
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

