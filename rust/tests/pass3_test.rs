use ifcomp::{FileIndex, Ifcomp, LineCount, LineType};

// Test helper: create fresh Ifcomp instance for each test
fn setup_pass3_test() -> Ifcomp {
    Ifcomp::new()
}

// ============================================================================
// Tests for pass3() - Basic functionality
// ============================================================================

#[test]
fn test_pass3_single_match_after_unique() {
    // Unique pair followed by duplicate line (which remains SYT_TYPE after pass2)
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nCOMMON\nCOMMON\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE (from pass2)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 2-3 should be MATCH_TYPE (extended from unique, COMMON is duplicate so remains SYT_TYPE after pass2)
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE (extended forward)"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE (extended forward)"
    );

    // Verify bidirectional links
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr0, 2,
        "File1 line 2 should point to file2 line 2"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr0, 2,
        "File2 line 2 should point to file1 line 2"
    );
}

#[test]
fn test_pass3_multiple_matches_after_unique() {
    // Unique pair followed by multiple duplicate lines (remain SYT_TYPE after pass2)
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nCOMMON\nCOMMON\nCOMMON\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON\nCOMMON\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 2-4 should be MATCH_TYPE (extended from unique)
    for i in 2..=4 {
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
fn test_pass3_no_extension_text_mismatch() {
    // Unique pair followed by non-matching line
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nDIFFERENT1\n".as_bytes();
    let file2 = "UNIQUE_A\nDIFFERENT2\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Line 2 should remain SYT_TYPE (text doesn't match)
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::SytType,
        "DIFFERENT1 should remain SYT_TYPE (no match)"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::SytType,
        "DIFFERENT2 should remain SYT_TYPE (no match)"
    );
}

#[test]
fn test_pass3_extension_stops_at_end_of_file() {
    // Unique pair at end of file
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

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
fn test_pass3_extension_stops_at_already_unique() {
    // Unique pair followed by another unique pair (not extending)
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Both should be UNIQUE_TYPE (pass3 doesn't extend because line 2 is already unique)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::UniqueType,
        "UNIQUE_B should be UNIQUE_TYPE (not extended, already unique)"
    );
}

#[test]
fn test_pass3_extension_stops_at_already_matched() {
    // Test that pass3 correctly extends and second call doesn't change anything
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nCOMMON\nCOMMON\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // First pass3 - should mark COMMON as MATCH_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "Should have MATCH_TYPE after pass3"
    );

    // Call pass3 again - should not change anything
    ifc.pass3();
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "Second pass3 should not change already matched line"
    );
}

#[test]
fn test_pass3_multiple_unique_pairs_with_extensions() {
    // Multiple unique pairs, each with forward extensions (using duplicate lines)
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nCOMMON1\nCOMMON1\nUNIQUE_B\nCOMMON2\nCOMMON2\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON1\nCOMMON1\nUNIQUE_B\nCOMMON2\nCOMMON2\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Lines 1 and 4 should be UNIQUE_TYPE
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

    // Lines 2-3 should be MATCH_TYPE (extended from UNIQUE_A)
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON1 line 2 should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::MatchType,
        "COMMON1 line 3 should be MATCH_TYPE"
    );

    // Lines 5-6 should be MATCH_TYPE (extended from UNIQUE_B)
    assert_eq!(
        ifc.file_state.file_line[0][5].ptr_type,
        LineType::MatchType,
        "COMMON2 line 5 should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][6].ptr_type,
        LineType::MatchType,
        "COMMON2 line 6 should be MATCH_TYPE"
    );
}

#[test]
fn test_pass3_partial_extension() {
    // Unique pair followed by matching duplicate lines, then non-matching
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nCOMMON\nCOMMON\nDIFFERENT1\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON\nCOMMON\nDIFFERENT2\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 2-3 should be MATCH_TYPE (extended, COMMON is duplicate so remains SYT_TYPE after pass2)
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

    // Line 4 should remain SYT_TYPE (extension stopped - text doesn't match)
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::SytType,
        "Extension should stop at DIFFERENT lines"
    );
    assert_eq!(
        ifc.file_state.file_line[1][4].ptr_type,
        LineType::SytType,
        "Extension should stop at DIFFERENT lines"
    );
}

#[test]
fn test_pass3_no_unique_pairs_no_extension() {
    // No unique pairs - pass3 should do nothing
    let mut ifc = setup_pass3_test();
    let file1 = "COMMON\nCOMMON\n".as_bytes();
    let file2 = "COMMON\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

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
fn test_pass3_extension_from_first_unique_only() {
    // First unique extends, second unique doesn't (preceded by already matched)
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 2-3 should be MATCH_TYPE (extended from UNIQUE_A)
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

    // Line 4 should be UNIQUE_TYPE (not extended because lines 2-3 are already MATCH_TYPE)
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::UniqueType,
        "UNIQUE_B should be UNIQUE_TYPE (cannot extend from already matched line)"
    );
}

#[test]
fn test_pass3_different_file_lengths_extension() {
    // File1 longer than file2, but extension should still work for matching part
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nCOMMON\nCOMMON\nEXTRA\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 2-3 should be MATCH_TYPE (extended, both files have COMMON)
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][3].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );

    // Extension should stop because file2 has no more lines
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::SytType,
        "EXTRA should remain SYT_TYPE (no corresponding line in file2)"
    );
}

#[test]
fn test_pass3_different_file_lengths_shorter_first() {
    // File2 longer than file1
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nCOMMON\nCOMMON\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON\nCOMMON\nEXTRA\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 2-3 should be MATCH_TYPE (extended, both files have COMMON)
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
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
        ifc.file_state.file_line[1][4].ptr_type,
        LineType::SytType,
        "EXTRA should remain SYT_TYPE (no corresponding line in file1)"
    );
}

#[test]
fn test_pass3_mixed_pattern() {
    // Complex pattern with unique, matches, and non-matches (using duplicate lines)
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nCOMMON\nCOMMON\nDIFF1\nUNIQUE_B\nCOMMON\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON\nCOMMON\nDIFF2\nUNIQUE_B\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Lines 1 and 5 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][5].ptr_type,
        LineType::UniqueType,
        "UNIQUE_B should be UNIQUE_TYPE"
    );

    // Lines 2-3 should be MATCH_TYPE (extended from UNIQUE_A)
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

    // Line 4 should remain SYT_TYPE (extension stopped)
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::SytType,
        "DIFF1 should remain SYT_TYPE"
    );

    // Line 6 should be MATCH_TYPE (extended from UNIQUE_B)
    assert_eq!(
        ifc.file_state.file_line[0][6].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][6].ptr_type,
        LineType::MatchType,
        "COMMON should be MATCH_TYPE"
    );
}

#[test]
fn test_pass3_empty_lines_in_extension() {
    // Unique pair followed by empty matching lines (duplicates remain SYT_TYPE)
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\n\n\n".as_bytes();
    let file2 = "UNIQUE_A\n\n\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 2-3 should be MATCH_TYPE (empty lines match)
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "Empty line 2 should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::MatchType,
        "Empty line 3 should be MATCH_TYPE"
    );
}

#[test]
fn test_pass3_long_extension() {
    // Unique pair followed by many duplicate matching lines (remain SYT_TYPE)
    let mut ifc = setup_pass3_test();
    let mut file1_content = String::new();
    let mut file2_content = String::new();
    file1_content.push_str("UNIQUE_A\n");
    file2_content.push_str("UNIQUE_A\n");
    for _i in 0..50 {
        file1_content.push_str("COMMON\n");
        file2_content.push_str("COMMON\n");
    }

    let file1 = file1_content.as_bytes();
    let file2 = file2_content.as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 2-51 should be MATCH_TYPE
    for i in 2..=51 {
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
fn test_pass3_no_extension_immediate_mismatch() {
    // Unique pair followed immediately by non-matching duplicate lines
    // Use different unique anchors, but same pattern - extension should stop
    let mut ifc = setup_pass3_test();
    let file1 = "ANCHOR1\nDIFF1\nDIFF1\n".as_bytes();
    let file2 = "ANCHOR2\nDIFF2\nDIFF2\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // ANCHOR1/ANCHOR2 are different, so no unique pairs
    // All lines should remain SYT_TYPE (no unique pairs, so no extension)
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::SytType,
        "ANCHOR1 should remain SYT_TYPE (different from ANCHOR2)"
    );
    assert_eq!(
        ifc.file_state.file_line[1][1].ptr_type,
        LineType::SytType,
        "ANCHOR2 should remain SYT_TYPE (different from ANCHOR1)"
    );
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::SytType,
        "DIFF1 should remain SYT_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr_type,
        LineType::SytType,
        "DIFF2 should remain SYT_TYPE"
    );
}

#[test]
fn test_pass3_extension_across_duplicate_lines() {
    // Unique pair, then duplicate lines that match
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nCOMMON\nCOMMON\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 2-3 should be MATCH_TYPE (extended, even though COMMON is duplicate)
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "COMMON line 2 should be MATCH_TYPE (extended)"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::MatchType,
        "COMMON line 3 should be MATCH_TYPE (extended)"
    );
}

#[test]
fn test_pass3_multiple_sequential_unique_pairs() {
    // Multiple unique pairs in sequence
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();
    let file2 = "UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // All should be UNIQUE_TYPE (pass3 doesn't extend because next line is already unique)
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
fn test_pass3_extension_then_unique() {
    // Extension followed by unique pair (using duplicate lines)
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Lines 1 and 4 should be UNIQUE_TYPE
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

    // Lines 2-3 should be MATCH_TYPE (extended from UNIQUE_A)
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

    // Extension stops at line 4 because it's already UNIQUE_TYPE
}

#[test]
fn test_pass3_bidirectional_linking_extension() {
    // Verify bidirectional links are created correctly for extended matches
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nCOMMON\nCOMMON\n".as_bytes();
    let file2 = "UNIQUE_A\nCOMMON\nCOMMON\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Check bidirectional links for extended matches
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr0, 2,
        "File1 COMMON line 2 should point to file2 line 2"
    );
    assert_eq!(
        ifc.file_state.file_line[1][2].ptr0, 2,
        "File2 COMMON line 2 should point to file1 line 2"
    );

    assert_eq!(
        ifc.file_state.file_line[0][3].ptr0, 3,
        "File1 COMMON line 3 should point to file2 line 3"
    );
    assert_eq!(
        ifc.file_state.file_line[1][3].ptr0, 3,
        "File2 COMMON line 3 should point to file1 line 3"
    );
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

#[test]
fn test_pass3_single_line_files() {
    // Single line in each file - unique pair
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\n".as_bytes();
    let file2 = "UNIQUE_A\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

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
fn test_pass3_extension_with_special_characters() {
    // Extension with lines containing special characters (duplicates remain SYT_TYPE)
    let mut ifc = setup_pass3_test();
    let file1 = "UNIQUE_A\nTAB_LINE\t\tTAB\nTAB_LINE\t\tTAB\nSPACE_LINE SPACE\nSPACE_LINE SPACE\n".as_bytes();
    let file2 = "UNIQUE_A\nTAB_LINE\t\tTAB\nTAB_LINE\t\tTAB\nSPACE_LINE SPACE\nSPACE_LINE SPACE\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // Lines 2-3 should be MATCH_TYPE (special characters match, duplicates)
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "Tab line 2 should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::MatchType,
        "Tab line 3 should be MATCH_TYPE"
    );
    // Lines 4-5 should be MATCH_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::MatchType,
        "Space line 4 should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][5].ptr_type,
        LineType::MatchType,
        "Space line 5 should be MATCH_TYPE"
    );
}

#[test]
fn test_pass3_very_long_extension() {
    // Test extension with 100 duplicate matching lines (remain SYT_TYPE)
    let mut ifc = setup_pass3_test();
    let mut file1_content = String::new();
    let mut file2_content = String::new();
    file1_content.push_str("UNIQUE_A\n");
    file2_content.push_str("UNIQUE_A\n");
    // Use same line repeated to make it duplicate
    for _i in 0..100 {
        file1_content.push_str("COMMON_LINE\n");
        file2_content.push_str("COMMON_LINE\n");
    }

    let file1 = file1_content.as_bytes();
    let file2 = file2_content.as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::UniqueType,
        "UNIQUE_A should be UNIQUE_TYPE"
    );

    // All subsequent lines should be MATCH_TYPE
    for i in 2..=101 {
        assert_eq!(
            ifc.file_state.file_line[0][i].ptr_type,
            LineType::MatchType,
            "Line {} should be MATCH_TYPE",
            i
        );
    }
}

#[test]
fn test_pass3_complex_real_world_scenario() {
    // Realistic scenario: code with function headers and bodies
    let mut ifc = setup_pass3_test();
    let file1 = "void func1()\n{\n    return;\n}\nvoid func2()\n{\n    return;\n}\n".as_bytes();
    let file2 = "void func1()\n{\n    return;\n}\nvoid func2()\n{\n    return;\n}\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");
    ifc.pass2();
    ifc.pass3();

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

    // Function bodies should be MATCH_TYPE (extended from unique signatures)
    assert_eq!(
        ifc.file_state.file_line[0][2].ptr_type,
        LineType::MatchType,
        "Opening brace should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][3].ptr_type,
        LineType::MatchType,
        "Return statement should be MATCH_TYPE"
    );
    assert_eq!(
        ifc.file_state.file_line[0][4].ptr_type,
        LineType::MatchType,
        "Closing brace should be MATCH_TYPE"
    );
}
