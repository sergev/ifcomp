use ifcomp::{FileIndex, Ifcomp, LineCount, LineType};

// Test helper: create fresh Ifcomp instance for each test
fn setup_pass4_test() -> Ifcomp {
    Ifcomp::new()
}

// ============================================================================
// Tests for pass4() - Basic functionality
// ============================================================================

#[test]
fn test_pass4_single_match_before_unique() {
    // Unique pair preceded by matching duplicate line
    let mut ifc = setup_pass4_test();
    let file1 = "COMMON\nCOMMON\nUNIQUE_A\n".as_bytes();
    let file2 = "COMMON\nCOMMON\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Line 3 should be UNIQUE_TYPE (from pass2)
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 1-2 should be MATCH_TYPE (extended backward from unique)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::MatchType,
        "COMMON line 1 should be MATCH_TYPE (extended backward)"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON line 2 should be MATCH_TYPE (extended backward)"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::MatchType,
        "COMMON line 1 should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::MatchType,
        "COMMON line 2 should be MATCH_TYPE"
    );

    // Verify bidirectional links
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr0, 1,
        "File1 line 1 should point to file2 line 1"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr0, 2,
        "File1 line 2 should point to file2 line 2"
    );
}

#[test]
fn test_pass4_multiple_matches_before_unique() {
    // Unique pair preceded by multiple matching duplicate lines
    let mut ifc = setup_pass4_test();
    let file1 = "COMMON\nCOMMON\nCOMMON\nUNIQUE_A\n".as_bytes();
    let file2 = "COMMON\nCOMMON\nCOMMON\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Line 4 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 1-3 should be MATCH_TYPE (extended backward)
    for i in 1..=3 {
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr_type,
            LineType::MatchType,
            "COMMON line {} should be MATCH_TYPE",
            i
        );
        assert_eq!(
            ifc.file_state.file_line[1][i].ptr_type,
            LineType::MatchType,
            "COMMON line {} should be MATCH_TYPE",
            i
        );
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr0,
            i as LineCount,
            "Bidirectional link check"
        );
        assert_eq!(
            ifc.file_state.file_line[1][i].ptr0,
            i as LineCount,
            "Bidirectional link check"
        );
    }
}

#[test]
fn test_pass4_no_extension_text_mismatch() {
    // Unique pair preceded by non-matching duplicate lines
    let mut ifc = setup_pass4_test();
    let file1 = "DIFF1\nDIFF1\nUNIQUE_A\n".as_bytes();
    let file2 = "DIFF2\nDIFF2\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Line 3 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 1-2 should remain SYT_TYPE (text doesn't match)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::SytType,
        "DIFF1 should remain SYT_TYPE (no match)"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::SytType,
        "DIFF1 should remain SYT_TYPE (no match)"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::SytType,
        "DIFF2 should remain SYT_TYPE (no match)"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::SytType,
        "DIFF2 should remain SYT_TYPE (no match)"
    );
}

#[test]
fn test_pass4_extension_stops_at_beginning_of_file() {
    // Unique pair at beginning of file
    let mut ifc = setup_pass4_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Only one line - should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );
}

#[test]
fn test_pass4_extension_stops_at_already_unique() {
    // Unique pair preceded by another unique pair (not extending)
    let mut ifc = setup_pass4_test();
    let file1 = "UNIQUE_B\nUNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_B\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Both should be UNIQUE_TYPE (pass4 doesn't extend because line 1 is already unique)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_B should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE (not extended, already unique)"
    );
}

#[test]
fn test_pass4_extension_stops_at_already_matched() {
    // Test that pass4 correctly extends and second call doesn't change anything
    let mut ifc = setup_pass4_test();
    let file1 = "COMMON\nCOMMON\nUNIQUE_A\n".as_bytes();
    let file2 = "COMMON\nCOMMON\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // First pass4 - should mark COMMON as MATCH_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::MatchType,
        "Should have MATCH_TYPE after pass4"
    );

    // Call pass4 again - should not change anything
    ifc.pass4();
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::MatchType,
        "Second pass4 should not change already matched line"
    );
}

#[test]
fn test_pass4_multiple_unique_pairs_with_extensions() {
    // Multiple unique pairs, each with backward extensions (using duplicate lines)
    let mut ifc = setup_pass4_test();
    let file1 = "COMMON1\nCOMMON1\nUNIQUE_A\nCOMMON2\nCOMMON2\nUNIQUE_B\n".as_bytes();
    let file2 = "COMMON1\nCOMMON1\nUNIQUE_A\nCOMMON2\nCOMMON2\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Lines 3 and 6 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][6].ptr_type,
        LineType::UniqueType,
        "UNIQUE_B should be UNIQUE_TYPE"
    );

    // Lines 1-2 should be MATCH_TYPE (extended backward from UNIQUE_A)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::MatchType,
        "COMMON1 line 1 should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON1 line 2 should be MATCH_TYPE"
    );

    // Lines 4-5 should be MATCH_TYPE (extended backward from UNIQUE_B)
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::MatchType,
        "COMMON2 line 4 should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][5].ptr_type,
        LineType::MatchType,
        "COMMON2 line 5 should be MATCH_TYPE"
    );
}

#[test]
fn test_pass4_partial_extension() {
    // Unique pair preceded by matching duplicate lines, then non-matching
    let mut ifc = setup_pass4_test();
    let file1 = "DIFFERENT1\nCOMMON\nCOMMON\nUNIQUE_A\n".as_bytes();
    let file2 = "DIFFERENT2\nCOMMON\nCOMMON\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Line 4 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 2-3 should be MATCH_TYPE (extended backward, COMMON is duplicate)
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );

    // Line 1 should remain SYT_TYPE (extension stopped - text doesn't match)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::SytType,
        "Extension should stop at DIFFERENT lines"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::SytType,
        "Extension should stop at DIFFERENT lines"
    );
}

#[test]
fn test_pass4_no_unique_pairs_no_extension() {
    // No unique pairs - pass4 should do nothing
    let mut ifc = setup_pass4_test();
    let file1 = "COMMON\nCOMMON\n".as_bytes();
    let file2 = "COMMON\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // All lines should remain SYT_TYPE (no unique pairs to extend from)
    for i in 1..=2 {
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr_type,
            LineType::SytType,
            "Line {} should remain SYT_TYPE (no unique pairs)",
            i
        );
        assert_eq!(
            ifc.file_state.file_line[1][i].ptr_type,
            LineType::SytType,
            "Line {} should remain SYT_TYPE (no unique pairs)",
            i
        );
    }
}

#[test]
fn test_pass4_extension_from_last_unique_only() {
    // Last unique extends, first unique doesn't (preceded by already matched)
    let mut ifc = setup_pass4_test();
    let file1 = "UNIQUE_B\nCOMMON\nCOMMON\nUNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_B\nCOMMON\nCOMMON\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Lines 1 and 4 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_B should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 2-3 should be MATCH_TYPE (extended backward from UNIQUE_A)
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );

    // Extension stops at line 1 because it's already UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_B should be UNIQUE_TYPE (cannot extend from already matched line)"
    );
}

#[test]
fn test_pass4_different_file_lengths_extension() {
    // File1 longer than file2, but extension should still work for matching part
    let mut ifc = setup_pass4_test();
    let file1 = "EXTRA\nCOMMON\nCOMMON\nUNIQUE_A\n".as_bytes();
    let file2 = "COMMON\nCOMMON\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Line 4 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 2-3 should be MATCH_TYPE (extended backward, both files have COMMON)
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );

    // Extension should stop because file2 has no more lines
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::SytType,
        "EXTRA should remain SYT_TYPE (no corresponding line in file2)"
    );
}

#[test]
fn test_pass4_different_file_lengths_shorter_first() {
    // File2 longer than file1
    let mut ifc = setup_pass4_test();
    let file1 = "COMMON\nCOMMON\nUNIQUE_A\n".as_bytes();
    let file2 = "EXTRA\nCOMMON\nCOMMON\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Line 3 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 1-2 should be MATCH_TYPE (extended backward, both files have COMMON)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][3].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );

    // Extension stops because file1 has no more lines
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::SytType,
        "EXTRA should remain SYT_TYPE (no corresponding line in file1)"
    );
}

#[test]
fn test_pass4_mixed_pattern() {
    // Complex pattern with unique, matches, and non-matches (using duplicate lines)
    let mut ifc = setup_pass4_test();
    let file1 = "DIFF1\nCOMMON\nCOMMON\nUNIQUE_A\nCOMMON\nUNIQUE_B\n".as_bytes();
    let file2 = "DIFF2\nCOMMON\nCOMMON\nUNIQUE_A\nCOMMON\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Lines 4 and 6 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][6].ptr_type,
        LineType::UniqueType,
        "UNIQUE_B should be UNIQUE_TYPE"
    );

    // Lines 2-3 should be MATCH_TYPE (extended backward from UNIQUE_A)
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );

    // Line 1 should remain SYT_TYPE (extension stopped)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::SytType,
        "DIFF1 should remain SYT_TYPE"
    );

    // Line 5 should be MATCH_TYPE (extended backward from UNIQUE_B)
    assert_eq!(
        ifc.file_state.file_line[0][5].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][5].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
}

#[test]
fn test_pass4_empty_lines_in_extension() {
    // Unique pair preceded by empty matching lines (duplicates remain SYT_TYPE)
    let mut ifc = setup_pass4_test();
    let file1 = "\n\nUNIQUE_A\n".as_bytes();
    let file2 = "\n\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Line 3 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 1-2 should be MATCH_TYPE (empty lines match)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::MatchType,
        "Empty line 1 should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "Empty line 2 should be MATCH_TYPE"
    );
}

#[test]
fn test_pass4_long_extension() {
    // Unique pair preceded by many duplicate matching lines (remain SYT_TYPE)
    let mut ifc = setup_pass4_test();
    let mut file1_content = String::new();
    let mut file2_content = String::new();
    for _i in 0..50 {
        file1_content.push_str("COMMON\n");
        file2_content.push_str("COMMON\n");
    }
    file1_content.push_str("UNIQUE_A\n");
    file2_content.push_str("UNIQUE_A\n");

    let file1 = file1_content.as_bytes();
    let file2 = file2_content.as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Line 51 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][51].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 1-50 should be MATCH_TYPE (extended backward)
    for i in 1..=50 {
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr_type,
            LineType::MatchType,
            "Line {} should be MATCH_TYPE",
            i
        );
        assert_eq!(
            ifc.file_state.file_line[1][i].ptr_type,
            LineType::MatchType,
            "Line {} should be MATCH_TYPE",
            i
        );
    }
}

#[test]
fn test_pass4_no_extension_immediate_mismatch() {
    // Unique pair preceded immediately by non-matching duplicate lines
    let mut ifc = setup_pass4_test();
    let file1 = "DIFF1\nDIFF1\nUNIQUE_A\n".as_bytes();
    let file2 = "DIFF2\nDIFF2\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // UNIQUE_A should be UNIQUE_TYPE (from pass2)
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][3].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 1-2 should remain SYT_TYPE (text doesn't match, so no extension)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::SytType,
        "No extension when text doesn't match"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::SytType,
        "No extension when text doesn't match"
    );
}

#[test]
fn test_pass4_extension_across_duplicate_lines() {
    // Unique pair, preceded by duplicate lines that match
    let mut ifc = setup_pass4_test();
    let file1 = "COMMON\nCOMMON\nUNIQUE_A\n".as_bytes();
    let file2 = "COMMON\nCOMMON\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Line 3 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 1-2 should be MATCH_TYPE (extended backward, even though COMMON is duplicate)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::MatchType,
        "COMMON line 1 should be MATCH_TYPE (extended)"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON line 2 should be MATCH_TYPE (extended)"
    );
}

#[test]
fn test_pass4_multiple_sequential_unique_pairs() {
    // Multiple unique pairs in sequence
    let mut ifc = setup_pass4_test();
    let file1 = "UNIQUE_C\nUNIQUE_B\nUNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_C\nUNIQUE_B\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // All should be UNIQUE_TYPE (pass4 doesn't extend because previous line is already unique)
    for i in 1..=3 {
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr_type,
            LineType::UniqueType,
            "Line {} should be UNIQUE_TYPE",
            i
        );
    }
}

#[test]
fn test_pass4_unique_then_extension() {
    // Unique pair, then backward extension from duplicate lines
    let mut ifc = setup_pass4_test();
    let file1 = "COMMON\nCOMMON\nUNIQUE_B\n".as_bytes();
    let file2 = "COMMON\nCOMMON\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Line 3 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::UniqueType,
        "UNIQUE_B should be UNIQUE_TYPE"
    );

    // Lines 1-2 should be MATCH_TYPE (extended backward from UNIQUE_B)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
}

#[test]
fn test_pass4_bidirectional_linking_extension() {
    // Verify bidirectional links are created correctly for extended matches
    let mut ifc = setup_pass4_test();
    let file1 = "COMMON\nCOMMON\nUNIQUE_A\n".as_bytes();
    let file2 = "COMMON\nCOMMON\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Check bidirectional links for extended matches
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr0, 1,
        "File1 COMMON line 1 should point to file2 line 1"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr0, 1,
        "File2 COMMON line 1 should point to file1 line 1"
    );

    assert_eq!(
        ifc.file_state.file_line[0][2].ptr0, 2,
        "File1 COMMON line 2 should point to file2 line 2"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr0, 2,
        "File2 COMMON line 2 should point to file1 line 2"
    );
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

#[test]
fn test_pass4_single_line_files() {
    // Single line in each file - unique pair
    let mut ifc = setup_pass4_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Should be UNIQUE_TYPE (no extension possible)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );
}

#[test]
fn test_pass4_extension_with_special_characters() {
    // Extension with lines containing special characters (duplicates remain SYT_TYPE)
    let mut ifc = setup_pass4_test();
    let file1 = "TAB_LINE\t\tTAB\nTAB_LINE\t\tTAB\nSPACE_LINE SPACE\nSPACE_LINE SPACE\nUNIQUE_A\n".as_bytes();
    let file2 = "TAB_LINE\t\tTAB\nTAB_LINE\t\tTAB\nSPACE_LINE SPACE\nSPACE_LINE SPACE\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Line 5 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][5].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 1-4 should be MATCH_TYPE (special characters match, duplicates)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::MatchType,
        "Tab line 1 should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "Tab line 2 should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::MatchType,
        "Space line 3 should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::MatchType,
        "Space line 4 should be MATCH_TYPE"
    );
}

#[test]
fn test_pass4_very_long_extension() {
    // Test extension with 100 duplicate matching lines (remain SYT_TYPE)
    let mut ifc = setup_pass4_test();
    let mut file1_content = String::new();
    let mut file2_content = String::new();
    // Use same line repeated to make it duplicate
    for _i in 0..100 {
        file1_content.push_str("COMMON_LINE\n");
        file2_content.push_str("COMMON_LINE\n");
    }
    file1_content.push_str("UNIQUE_A\n");
    file2_content.push_str("UNIQUE_A\n");

    let file1 = file1_content.as_bytes();
    let file2 = file2_content.as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Line 101 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][101].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // All previous lines should be MATCH_TYPE
    for i in 1..=100 {
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr_type,
            LineType::MatchType,
            "Line {} should be MATCH_TYPE",
            i
        );
    }
}

#[test]
fn test_pass4_complex_real_world_scenario() {
    // Realistic scenario: code with function headers and bodies
    let mut ifc = setup_pass4_test();
    let file1 = "void func1()\n{\n    return;\n}\nvoid func2()\n{\n    return;\n}\n".as_bytes();
    let file2 = "void func1()\n{\n    return;\n}\nvoid func2()\n{\n    return;\n}\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Function signatures should be unique pairs
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "func1 signature should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][5].ptr_type,
        LineType::UniqueType,
        "func2 signature should be UNIQUE_TYPE"
    );

    // Function bodies should be MATCH_TYPE (extended backward from unique signatures)
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::MatchType,
        "Closing brace should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::MatchType,
        "Return statement should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "Opening brace should be MATCH_TYPE"
    );
}

#[test]
fn test_pass4_combined_with_pass3() {
    // Test that pass3 and pass4 work together correctly
    let mut ifc = setup_pass4_test();
    let file1 = "COMMON1\nCOMMON1\nUNIQUE_A\nCOMMON2\nCOMMON2\n".as_bytes();
    let file2 = "COMMON1\nCOMMON1\nUNIQUE_A\nCOMMON2\nCOMMON2\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3(); // Forward extension
    ifc.pass4(); // Backward extension

    // Line 3 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 1-2 should be MATCH_TYPE (extended backward by pass4)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::MatchType,
        "COMMON1 should be MATCH_TYPE (extended backward)"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON1 should be MATCH_TYPE (extended backward)"
    );

    // Lines 4-5 should be MATCH_TYPE (extended forward by pass3)
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::MatchType,
        "COMMON2 should be MATCH_TYPE (extended forward)"
    );
    assert_eq!(
        ifc.file_state.file_line[0][5].ptr_type,
        LineType::MatchType,
        "COMMON2 should be MATCH_TYPE (extended forward)"
    );
}

#[test]
fn test_pass4_stops_at_zero() {
    // Test that extension stops correctly at beginning (m > 0 check)
    let mut ifc = setup_pass4_test();
    let file1 = "COMMON\nCOMMON\nUNIQUE_A\n".as_bytes();
    let file2 = "COMMON\nCOMMON\nUNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass4();

    // Line 3 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 1-2 should be MATCH_TYPE (extended backward from UNIQUE_A)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::MatchType,
        "COMMON line 1 should be MATCH_TYPE (extended backward)"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON line 2 should be MATCH_TYPE (extended backward)"
    );

    // Verify extension stops at beginning (m > 0 check ensures we don't go below 1)
}
