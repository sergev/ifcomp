use ifcomp::{Ifcomp, LineCount, LineType};

// Test helper: create fresh Ifcomp instance for each test
fn setup_pass2_test() -> Ifcomp {
    Ifcomp::new()
}

// ============================================================================
// Tests for pass2() - Basic functionality
// ============================================================================

#[test]
fn test_pass2_single_unique_pair() {
    // Two identical files with one line each
    let mut ifc = setup_pass2_test();
    let file1 = "LINE1\n".as_bytes();
    let file2 = "LINE1\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // Both lines should be marked as UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "Line 1 in file1 should be marked as UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::UniqueType,
        "Line 1 in file2 should be marked as UNIQUE_TYPE"
    );

    // ptr0 should reference each other
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr0, 1,
        "File1 line 1 should point to file2 line 1"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr0, 1,
        "File2 line 1 should point to file1 line 1"
    );
}

#[test]
fn test_pass2_multiple_unique_pairs() {
    // Files with multiple unique pairs
    let mut ifc = setup_pass2_test();
    let file1 = "A\nB\nC\n".as_bytes();
    let file2 = "A\nB\nC\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // All lines should be marked as UNIQUE_TYPE
    for i in 1..=3 {
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr_type,
            LineType::UniqueType,
            "File1 line {} should be UNIQUE_TYPE",
            i
        );
        assert_eq!(
            ifc.file_state.file_line[1][i].ptr_type,
            LineType::UniqueType,
            "File2 line {} should be UNIQUE_TYPE",
            i
        );
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr0,
            i as LineCount,
            "File1 line {} should point to file2 line {}",
            i,
            i
        );
        assert_eq!(
            ifc.file_state.file_line[1][i].ptr0,
            i as LineCount,
            "File2 line {} should point to file1 line {}",
            i,
            i
        );
    }
}

#[test]
fn test_pass2_no_unique_pairs_all_duplicates() {
    // Files with duplicate lines - no unique pairs
    let mut ifc = setup_pass2_test();
    let file1 = "A\nA\n".as_bytes();
    let file2 = "A\nA\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // All lines should remain SYT_TYPE (not unique)
    for i in 1..=2 {
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr_type,
            LineType::SytType,
            "File1 line {} should remain SYT_TYPE (duplicate)",
            i
        );
        assert_eq!(
            ifc.file_state.file_line[1][i].ptr_type,
            LineType::SytType,
            "File2 line {} should remain SYT_TYPE (duplicate)",
            i
        );
    }
}

#[test]
fn test_pass2_no_unique_pairs_duplicate_in_first_file() {
    // Line appears twice in file1, once in file2
    let mut ifc = setup_pass2_test();
    let file1 = "A\nA\n".as_bytes();
    let file2 = "A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // None should be marked as unique (appears twice in file1)
    for i in 1..=2 {
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr_type,
            LineType::SytType,
            "File1 line {} should remain SYT_TYPE (duplicate in file1)",
            i
        );
    }
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::SytType,
        "File2 line 1 should remain SYT_TYPE (duplicate in file1)"
    );
}

#[test]
fn test_pass2_no_unique_pairs_duplicate_in_second_file() {
    // Line appears once in file1, twice in file2
    let mut ifc = setup_pass2_test();
    let file1 = "A\n".as_bytes();
    let file2 = "A\nA\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // None should be marked as unique (appears twice in file2)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::SytType,
        "File1 line 1 should remain SYT_TYPE (duplicate in file2)"
    );
    for i in 1..=2 {
        assert_eq!(
            ifc.file_state.file_line[1][i].ptr_type,
            LineType::SytType,
            "File2 line {} should remain SYT_TYPE (duplicate in file2)",
            i
        );
    }
}

#[test]
fn test_pass2_mixed_unique_and_duplicates() {
    // Some lines are unique pairs, some are duplicates
    let mut ifc = setup_pass2_test();
    let file1 = "UNIQUE1\nDUPLICATE\nDUPLICATE\nUNIQUE2\n".as_bytes();
    let file2 = "UNIQUE1\nDUPLICATE\nDUPLICATE\nUNIQUE2\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // Unique lines should be marked
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE1 in file1 should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::UniqueType,
        "UNIQUE2 in file1 should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE1 in file2 should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][4].ptr_type,
        LineType::UniqueType,
        "UNIQUE2 in file2 should be UNIQUE_TYPE"
    );

    // Duplicate lines should remain SYT_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::SytType,
        "DUPLICATE at file1 line 2 should remain SYT_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::SytType,
        "DUPLICATE at file1 line 3 should remain SYT_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::SytType,
        "DUPLICATE at file2 line 2 should remain SYT_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][3].ptr_type,
        LineType::SytType,
        "DUPLICATE at file2 line 3 should remain SYT_TYPE"
    );
}

#[test]
fn test_pass2_interleaved_unique_and_non_unique() {
    // Unique and non-unique lines interleaved
    let mut ifc = setup_pass2_test();
    let file1 = "UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\nCOMMON\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // Unique lines should be marked
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::UniqueType,
        "UNIQUE_B should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][4].ptr_type,
        LineType::UniqueType,
        "UNIQUE_B should be UNIQUE_TYPE"
    );

    // COMMON lines (appearing 4 times total) should remain SYT_TYPE
    for i in 2..=3 {
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr_type,
            LineType::SytType,
            "COMMON at file1 line {} should remain SYT_TYPE",
            i
        );
        assert_eq!(
            ifc.file_state.file_line[1][i].ptr_type,
            LineType::SytType,
            "COMMON at file2 line {} should remain SYT_TYPE",
            i
        );
    }
    assert_eq!(
        ifc.file_state.file_line[0][5].ptr_type,
        LineType::SytType,
        "COMMON at file1 line 5 should remain SYT_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][5].ptr_type,
        LineType::SytType,
        "COMMON at file2 line 5 should remain SYT_TYPE"
    );
}

#[test]
fn test_pass2_different_order_but_same_content() {
    // Same content but different order - should still match unique pairs
    let mut ifc = setup_pass2_test();
    let file1 = "A\nB\nC\n".as_bytes();
    let file2 = "C\nB\nA\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // All lines are unique pairs (each appears once in each file)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "A should be marked as UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::UniqueType,
        "B should be marked as UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::UniqueType,
        "C should be marked as UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::UniqueType,
        "C should be marked as UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::UniqueType,
        "B should be marked as UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][3].ptr_type,
        LineType::UniqueType,
        "A should be marked as UNIQUE_TYPE"
    );

    // Check that ptr0 points to correct lines (different positions)
    // A in file1 (line 1) should point to A in file2 (line 3)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr0, 3,
        "File1 A (line 1) should point to file2 A (line 3)"
    );
    assert_eq!(
        ifc.file_state.file_line[1][3].ptr0, 1,
        "File2 A (line 3) should point to file1 A (line 1)"
    );
}

#[test]
fn test_pass2_completely_different_files() {
    // Two completely different files - no matches
    let mut ifc = setup_pass2_test();
    let file1 = "A\nB\n".as_bytes();
    let file2 = "X\nY\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // No lines should be marked as unique (none appear in both files)
    for i in 1..=2 {
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr_type,
            LineType::SytType,
            "File1 line {} should remain SYT_TYPE (no match)",
            i
        );
        assert_eq!(
            ifc.file_state.file_line[1][i].ptr_type,
            LineType::SytType,
            "File2 line {} should remain SYT_TYPE (no match)",
            i
        );
    }
}

#[test]
fn test_pass2_partial_overlap() {
    // Some lines match, some don't
    let mut ifc = setup_pass2_test();
    let file1 = "COMMON1\nUNIQUE_A\nCOMMON2\nUNIQUE_B\n".as_bytes();
    let file2 = "COMMON1\nCOMMON2\nUNIQUE_A\nDIFFERENT\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // COMMON lines appear twice (once in each file) - should be unique pairs
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "COMMON1 should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::UniqueType,
        "COMMON2 should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::UniqueType,
        "COMMON1 should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::UniqueType,
        "COMMON2 should be UNIQUE_TYPE"
    );

    // UNIQUE_A appears once in each file - should be unique pair
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A in file1 should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][3].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A in file2 should be UNIQUE_TYPE"
    );

    // UNIQUE_B and DIFFERENT appear only in one file - should remain SYT_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::SytType,
        "UNIQUE_B should remain SYT_TYPE (only in file1)"
    );
    assert_eq!(
        ifc.file_state.file_line[1][4].ptr_type,
        LineType::SytType,
        "DIFFERENT should remain SYT_TYPE (only in file2)"
    );
}

#[test]
fn test_pass2_three_occurrences_no_unique() {
    // Line appears 3 times in file1, 3 times in file2
    let mut ifc = setup_pass2_test();
    let file1 = "A\nA\nA\n".as_bytes();
    let file2 = "A\nA\nA\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // No lines should be marked as unique (all appear 3 times)
    for i in 1..=3 {
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr_type,
            LineType::SytType,
            "File1 line {} should remain SYT_TYPE (3 occurrences)",
            i
        );
        assert_eq!(
            ifc.file_state.file_line[1][i].ptr_type,
            LineType::SytType,
            "File2 line {} should remain SYT_TYPE (3 occurrences)",
            i
        );
    }
}

#[test]
fn test_pass2_empty_lines() {
    // Files with empty lines
    let mut ifc = setup_pass2_test();
    let file1 = "\n\n".as_bytes();
    let file2 = "\n\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // Empty lines appearing twice in each file should remain SYT_TYPE
    for i in 1..=2 {
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr_type,
            LineType::SytType,
            "Empty line {} in file1 should remain SYT_TYPE (duplicate)",
            i
        );
        assert_eq!(
            ifc.file_state.file_line[1][i].ptr_type,
            LineType::SytType,
            "Empty line {} in file2 should remain SYT_TYPE (duplicate)",
            i
        );
    }
}

#[test]
fn test_pass2_single_empty_line_unique() {
    // Single empty line in each file
    let mut ifc = setup_pass2_test();
    let file1 = "\n".as_bytes();
    let file2 = "\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // Should be marked as unique pair
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "Single empty line should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::UniqueType,
        "Single empty line should be UNIQUE_TYPE"
    );
}

#[test]
fn test_pass2_long_lines_unique() {
    // Long lines that are unique pairs
    let mut ifc = setup_pass2_test();
    let long_line = "X".repeat(1000);
    let input_str = format!("{}\n", long_line);
    let file1 = input_str.as_bytes();
    let input_str2 = format!("{}\n", long_line);
    let file2 = input_str2.as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // Should be marked as unique pair
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "Long unique line should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::UniqueType,
        "Long unique line should be UNIQUE_TYPE"
    );
}

#[test]
fn test_pass2_special_characters_unique() {
    // Lines with special characters that are unique pairs
    let mut ifc = setup_pass2_test();
    let file1 = "LINE\tWITH\tTABS\nLINE WITH SPACES\n".as_bytes();
    let file2 = "LINE\tWITH\tTABS\nLINE WITH SPACES\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // Both should be marked as unique pairs
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "Line with tabs should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::UniqueType,
        "Line with spaces should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::UniqueType,
        "Line with tabs should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::UniqueType,
        "Line with spaces should be UNIQUE_TYPE"
    );
}

#[test]
fn test_pass2_bidirectional_linking() {
    // Verify that unique pairs have bidirectional links
    let mut ifc = setup_pass2_test();
    let file1 = "A\nB\n".as_bytes();
    let file2 = "B\nA\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // A in file1 (line 1) should point to A in file2 (line 2)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr0, 2,
        "File1 A should point to file2 line 2"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr0, 1,
        "File2 A should point to file1 line 1"
    );

    // B in file1 (line 2) should point to B in file2 (line 1)
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr0, 1,
        "File1 B should point to file2 line 1"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr0, 2,
        "File2 B should point to file1 line 2"
    );
}

#[test]
fn test_pass2_large_number_of_unique_pairs() {
    // Many unique pairs
    let mut ifc = setup_pass2_test();
    let mut file1_content = String::new();
    let mut file2_content = String::new();
    for i in 0..100 {
        file1_content.push_str(&format!("LINE{}\n", i));
        file2_content.push_str(&format!("LINE{}\n", i));
    }

    let file1 = file1_content.as_bytes();
    let file2 = file2_content.as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // All lines should be marked as unique pairs
    for i in 1..=100 {
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr_type,
            LineType::UniqueType,
            "Line {} in file1 should be UNIQUE_TYPE",
            i
        );
        assert_eq!(
            ifc.file_state.file_line[1][i].ptr_type,
            LineType::UniqueType,
            "Line {} in file2 should be UNIQUE_TYPE",
            i
        );
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr0,
            i as LineCount,
            "File1 line {} should point to file2 line {}",
            i,
            i
        );
        assert_eq!(
            ifc.file_state.file_line[1][i].ptr0,
            i as LineCount,
            "File2 line {} should point to file1 line {}",
            i,
            i
        );
    }
}

#[test]
fn test_pass2_one_file_larger() {
    // File1 has more lines than file2, but some are unique pairs
    let mut ifc = setup_pass2_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // UNIQUE_A and UNIQUE_B should be marked as unique pairs
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::UniqueType,
        "UNIQUE_B should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::UniqueType,
        "UNIQUE_B should be UNIQUE_TYPE"
    );

    // UNIQUE_C only in file1 - should remain SYT_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::SytType,
        "UNIQUE_C should remain SYT_TYPE (only in file1)"
    );
}

#[test]
fn test_pass2_one_file_smaller() {
    // File2 has more lines than file1, but some are unique pairs
    let mut ifc = setup_pass2_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // UNIQUE_A and UNIQUE_B should be marked as unique pairs
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::UniqueType,
        "UNIQUE_B should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::UniqueType,
        "UNIQUE_B should be UNIQUE_TYPE"
    );

    // UNIQUE_C only in file2 - should remain SYT_TYPE
    assert_eq!(
        ifc.file_state.file_line[1][3].ptr_type,
        LineType::SytType,
        "UNIQUE_C should remain SYT_TYPE (only in file2)"
    );
}

#[test]
fn test_pass2_multiple_occurrences_in_both_files() {
    // Line appears twice in file1, twice in file2 - should NOT be unique
    let mut ifc = setup_pass2_test();
    let file1 = "A\nB\nA\n".as_bytes();
    let file2 = "A\nB\nA\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // A appears twice in each file - should remain SYT_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::SytType,
        "A at file1 line 1 should remain SYT_TYPE (2 occurrences)"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::SytType,
        "A at file1 line 3 should remain SYT_TYPE (2 occurrences)"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::SytType,
        "A at file2 line 1 should remain SYT_TYPE (2 occurrences)"
    );
    assert_eq!(
        ifc.file_state.file_line[1][3].ptr_type,
        LineType::SytType,
        "A at file2 line 3 should remain SYT_TYPE (2 occurrences)"
    );

    // B appears once in each file - should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::UniqueType,
        "B should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::UniqueType,
        "B should be UNIQUE_TYPE"
    );
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

#[test]
fn test_pass2_all_lines_unique() {
    // Every line is unique - large test
    let mut ifc = setup_pass2_test();
    let mut file1_content = String::new();
    let mut file2_content = String::new();
    for i in 0..50 {
        file1_content.push_str(&format!("UNIQUE{}_FILE1\n", i));
        file2_content.push_str(&format!("UNIQUE{}_FILE2\n", i));
    }

    let file1 = file1_content.as_bytes();
    let file2 = file2_content.as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // No lines should be marked as unique (none appear in both files)
    for i in 1..=50 {
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr_type,
            LineType::SytType,
            "File1 line {} should remain SYT_TYPE (no match)",
            i
        );
        assert_eq!(
            ifc.file_state.file_line[1][i].ptr_type,
            LineType::SytType,
            "File2 line {} should remain SYT_TYPE (no match)",
            i
        );
    }
}

#[test]
fn test_pass2_complex_pattern() {
    // Complex pattern with various scenarios
    let mut ifc = setup_pass2_test();
    let file1 = "UNIQUE1\nCOMMON\nCOMMON\nUNIQUE2\nCOMMON\nUNIQUE3\nDIFF1\n".as_bytes();
    let file2 = "COMMON\nUNIQUE1\nUNIQUE2\nCOMMON\nCOMMON\nDIFF2\nUNIQUE3\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();

    // COMMON appears 3 times in file1, 3 times in file2 - should remain SYT_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::SytType,
        "COMMON should remain SYT_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::SytType,
        "COMMON should remain SYT_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][5].ptr_type,
        LineType::SytType,
        "COMMON should remain SYT_TYPE"
    );

    // UNIQUE lines should be marked
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE1 should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::UniqueType,
        "UNIQUE2 should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][6].ptr_type,
        LineType::UniqueType,
        "UNIQUE3 should be UNIQUE_TYPE"
    );

    // DIFF lines only in one file - should remain SYT_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][7].ptr_type,
        LineType::SytType,
        "DIFF1 should remain SYT_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][6].ptr_type,
        LineType::SytType,
        "DIFF2 should remain SYT_TYPE"
    );
}
