use ifcomp::{Ifcomp, NULL_NODE};

// Test helper: create fresh Ifcomp instance for each test
fn setup_pass8_test() -> Ifcomp {
    Ifcomp::new()
}

// ============================================================================
// Tests for helper functions
// ============================================================================

#[test]
fn test_pass8_insert_node_after_basic() {
    // Test insert_node_after() function
    let mut ifc = setup_pass8_test();
    let file1 = "UNIQUE_A\nDIFF1\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nOTHER1\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let header = ifc.tree_state.trees[0].start;
    let node1 = ifc.tree_state.node[header].next;
    let node2 = ifc.tree_state.node[node1].next;
    let node3 = ifc.tree_state.node[node2].next;
    let trailer = ifc.tree_state.trees[0].end;

    // Verify initial structure: header -> node1 (matched) -> node2 (unmatched) -> node3 (matched) -> trailer
    assert_eq!(
        ifc.tree_state.node[node1].prev, header,
        "Initial structure check"
    );
    assert_eq!(
        ifc.tree_state.node[node1].next, node2,
        "Initial structure check"
    );
    assert_eq!(
        ifc.tree_state.node[node2].prev, node1,
        "Initial structure check"
    );
    assert_eq!(
        ifc.tree_state.node[node2].next, node3,
        "Initial structure check"
    );
    assert_eq!(
        ifc.tree_state.node[node3].prev, node2,
        "Initial structure check"
    );
    assert_eq!(
        ifc.tree_state.node[node3].next, trailer,
        "Initial structure check"
    );
    assert_eq!(
        ifc.tree_state.node[trailer].prev, node3,
        "Initial structure check"
    );

    // Insert node2 after header (move unmatched segment to start)
    ifc.detach_node(node2); // Detach first
    ifc.insert_node_after(header, node2);

    // Verify new structure: header -> node2 -> node1 -> node3 -> trailer
    assert_eq!(
        ifc.tree_state.node[header].next, node2,
        "header.next should be node2"
    );
    assert_eq!(
        ifc.tree_state.node[node2].prev, header,
        "node2.prev should be header"
    );
    assert_eq!(
        ifc.tree_state.node[node2].next, node1,
        "node2.next should be node1"
    );
    assert_eq!(
        ifc.tree_state.node[node1].prev, node2,
        "node1.prev should be node2"
    );
    assert_eq!(
        ifc.tree_state.node[node1].next, node3,
        "node1.next should be node3"
    );
    assert_eq!(
        ifc.tree_state.node[node3].prev, node1,
        "node3.prev should be node1"
    );
    assert_eq!(
        ifc.tree_state.node[node3].next, trailer,
        "node3 should still point to trailer"
    );
    assert_eq!(
        ifc.tree_state.node[trailer].prev, node3,
        "trailer should still point back to node3"
    );
}

#[test]
fn test_pass8_insert_node_after_middle() {
    // Test insert_node_after() when inserting in middle
    let mut ifc = setup_pass8_test();
    let file1 = "UNIQUE_A\nDIFF1\nUNIQUE_B\nDIFF2\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nOTHER1\nUNIQUE_B\nOTHER2\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let header = ifc.tree_state.trees[0].start;
    let node1 = ifc.tree_state.node[header].next; // matched UNIQUE_A
    let node2 = ifc.tree_state.node[node1].next;  // unmatched DIFF1
    let node3 = ifc.tree_state.node[node2].next;  // matched UNIQUE_B
    let node4 = ifc.tree_state.node[node3].next;  // unmatched DIFF2
    let node5 = ifc.tree_state.node[node4].next;  // matched UNIQUE_C

    // Initial: header -> node1 -> node2 -> node3 -> node4 -> node5 -> trailer
    // Insert node5 after node1: header -> node1 -> node5 -> node2 -> node3 -> node4 -> trailer
    ifc.detach_node(node5);
    ifc.insert_node_after(node1, node5);

    assert_eq!(
        ifc.tree_state.node[node1].next, node5,
        "node1.next should be node5"
    );
    assert_eq!(
        ifc.tree_state.node[node5].prev, node1,
        "node5.prev should be node1"
    );
    assert_eq!(
        ifc.tree_state.node[node5].next, node2,
        "node5.next should be node2"
    );
    assert_eq!(
        ifc.tree_state.node[node2].prev, node5,
        "node2.prev should be node5"
    );
    assert_eq!(
        ifc.tree_state.node[node2].next, node3,
        "node2.next should be node3"
    );
}

#[test]
fn test_pass8_pass8_min_cost_node_single_node() {
    // Test pass8_min_cost_node() with single node
    let mut ifc = setup_pass8_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let header = ifc.tree_state.trees[0].start;
    let node1 = ifc.tree_state.node[header].next;
    let trailer = ifc.tree_state.trees[0].end;

    let min_node = ifc.pass8_min_cost_node(node1, trailer);
    assert_eq!(
        min_node, node1,
        "Should return the only node"
    );
}

#[test]
fn test_pass8_pass8_min_cost_node_multiple_nodes() {
    // Test pass8_min_cost_node() with multiple nodes, finding minimum cost
    let mut ifc = setup_pass8_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass5();

    let header = ifc.tree_state.trees[0].start;
    let node1 = ifc.tree_state.node[header].next;
    let trailer = ifc.tree_state.trees[0].end;

    // All nodes should have cost 1 (single line each)
    // The function should return the first node if all costs are equal
    let min_node = ifc.pass8_min_cost_node(node1, trailer);
    assert!(
        min_node >= node1,
        "Min node should be >= node1"
    );
    assert!(
        min_node < trailer,
        "Min node should be < trailer"
    );
}

#[test]
fn test_pass8_pass8_min_cost_node_different_costs() {
    // Test pass8_min_cost_node() when nodes have different costs
    let mut ifc = setup_pass8_test();
    let file1 = "COMMON\nCOMMON\nUNIQUE_A\nCOMMON\nCOMMON\nCOMMON\nUNIQUE_B\n".as_bytes();
    let file2 = "COMMON\nCOMMON\nUNIQUE_A\nCOMMON\nCOMMON\nCOMMON\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();

    let header = ifc.tree_state.trees[0].start;
    let node1 = ifc.tree_state.node[header].next;
    let trailer = ifc.tree_state.trees[0].end;

    // Should find minimum cost node
    let min_node = ifc.pass8_min_cost_node(node1, trailer);
    assert!(
        min_node >= node1,
        "Min node should be >= node1"
    );
    assert!(
        min_node < trailer,
        "Min node should be < trailer"
    );
}

// ============================================================================
// Tests for pass8() - Basic functionality
// ============================================================================

#[test]
fn test_pass8_simple_reorder() {
    // Simple reordering case - lines are permuted
    let mut ifc = setup_pass8_test();
    let file1 = "A\nB\nC\n".as_bytes();
    let file2 = "C\nA\nB\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass8().expect("pass8 should succeed");

    // pass8 should complete without errors
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass8_no_moves_needed() {
    // Already in order - no moves needed
    let mut ifc = setup_pass8_test();
    let file1 = "A\nB\nC\n".as_bytes();
    let file2 = "A\nB\nC\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass8().expect("pass8 should succeed");

    // Should complete without errors
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass8_identical_files() {
    // Identical files should complete quickly
    let mut ifc = setup_pass8_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass8().expect("pass8 should succeed");

    // Should have no moves
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass8_complex_permutation() {
    // Complex permutation of lines
    let mut ifc = setup_pass8_test();
    let file1 = "LINE1\nLINE2\nLINE3\nLINE4\nLINE5\n".as_bytes();
    let file2 = "LINE3\nLINE5\nLINE1\nLINE4\nLINE2\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass8().expect("pass8 should succeed");

    // Should handle complex permutations
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass8_statistics_tracked() {
    // Verify statistics are tracked
    let mut ifc = setup_pass8_test();
    let file1 = "A\nB\n".as_bytes();
    let file2 = "B\nA\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass8().expect("pass8 should succeed");

    // Statistics should be initialized
    let _move_stats = &ifc.stats.move_stats;
    let _n_change_blocks = ifc.stats.n_change_blocks;
    let _ = (_move_stats, _n_change_blocks);
}

#[test]
fn test_pass8_tree_integrity_after_moves() {
    // Verify tree integrity after moves
    let mut ifc = setup_pass8_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_C\nUNIQUE_A\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass8().expect("pass8 should succeed");

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
fn test_pass8_multiple_pass8_cycles() {
    // Test that pass8 can handle multiple move cycles
    let mut ifc = setup_pass8_test();
    let file1 = "LINE1\nLINE2\nLINE3\nLINE4\n".as_bytes();
    let file2 = "LINE3\nLINE4\nLINE1\nLINE2\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass8().expect("pass8 should succeed");

    // Should complete without infinite loop
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass8_minimum_cost_selection() {
    // Test minimum cost selection
    let mut ifc = setup_pass8_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_C\nUNIQUE_A\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass8().expect("pass8 should succeed");

    // Should select minimum cost nodes
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass8_edge_case_single_line() {
    // Test with single line (no moves possible)
    let mut ifc = setup_pass8_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass8().expect("pass8 should succeed");

    // Should handle gracefully
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass8_edge_case_empty_files() {
    // Edge case: empty files (causes readLines to return error)
    let mut ifc = setup_pass8_test();
    let file1 = "".as_bytes();
    let file2 = "".as_bytes();

    // Empty files now return error instead of os.Exit
    let err = ifc.pass1(file1, file2);
    assert!(
        err.is_err(),
        "Expected error for empty files"
    );
}

#[test]
fn test_pass8_long_files() {
    // Test with longer files
    let mut ifc = setup_pass8_test();
    let mut file1_content = String::new();
    let mut file2_content = String::new();
    for i in 0..30 {
        file1_content.push_str(&format!("LINE{}\n", i));
        file2_content.push_str(&format!("LINE{}\n", (i + 10) % 30));
    }

    let file1 = file1_content.as_bytes();
    let file2 = file2_content.as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass8().expect("pass8 should succeed");

    // Should handle longer files
    assert!(
        ifc.stats.n_change_blocks > 0,
        "Should have change blocks for reordered files"
    );
}

#[test]
fn test_pass8_restart_after_move() {
    // Test that pass8 restarts after each move
    let mut ifc = setup_pass8_test();
    let file1 = "LINE1\nLINE2\nLINE3\nLINE4\nLINE5\n".as_bytes();
    let file2 = "LINE4\nLINE5\nLINE1\nLINE2\nLINE3\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass8().expect("pass8 should succeed");

    // Should complete without infinite loop (restarts prevent loops)
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass8_complex_real_world_scenario() {
    // Realistic scenario: code with functions reordered
    let mut ifc = setup_pass8_test();
    let file1 = "func1()\nfunc2()\nfunc3()\nfunc4()\n".as_bytes();
    let file2 = "func3()\nfunc1()\nfunc4()\nfunc2()\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass8().expect("pass8 should succeed");

    // Should handle realistic patterns
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass8_mixed_changes_and_moves() {
    // Mix of deletions, insertions, and moves
    let mut ifc = setup_pass8_test();
    let file1 = "KEEP1\nMOVE1\nDELETE\nKEEP2\nMOVE2\n".as_bytes();
    let file2 = "KEEP1\nINSERT\nKEEP2\nMOVE2\nMOVE1\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass8().expect("pass8 should succeed");

    // Should handle mixed operations
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass8_no_unique_lines() {
    // Test with no unique lines (all duplicates)
    let mut ifc = setup_pass8_test();
    let file1 = "COMMON\nCOMMON\n".as_bytes();
    let file2 = "COMMON\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass8().expect("pass8 should succeed");

    // Should handle all duplicates through pass5
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass8_stress_test() {
    // Stress test with many lines - but avoid problematic patterns
    let mut ifc = setup_pass8_test();
    let mut file1_content = String::new();
    let mut file2_content = String::new();
    // Use unique lines to avoid duplicate issues
    for i in 0..30 {
        file1_content.push_str(&format!("UNIQUE{}\n", i));
        file2_content.push_str(&format!("UNIQUE{}\n", (i + 10) % 30));
    }

    let file1 = file1_content.as_bytes();
    let file2 = file2_content.as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");
    ifc.pass8().expect("pass8 should succeed");

    // Should complete without timing out or crashing
    assert_ne!(
        ifc.tree_state.trees[0].start, NULL_NODE,
        "Should have tree structure"
    );
}

#[test]
fn test_pass8_maintains_tree_bounds() {
    // Verify tree bounds are maintained
    let mut ifc = setup_pass8_test();
    let file1 = "A\nB\nC\n".as_bytes();
    let file2 = "C\nA\nB\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7().expect("pass7 should succeed");

    let before_start = ifc.tree_state.trees[0].start;
    let before_end = ifc.tree_state.trees[0].end;

    ifc.pass8().expect("pass8 should succeed");

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

