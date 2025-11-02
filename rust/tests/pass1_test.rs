use ifcomp::{
    CompareResult, FileIndex, Ifcomp, LineCount, LineType, NULL_HASH_LIST, NULL_LINE_LIST,
    NULL_STRING_LIST, N_BUCKETS,
};

// Test helper: create fresh Ifcomp instance for each test
fn setup_pass1_test() -> Ifcomp {
    Ifcomp::new()
}

// ============================================================================
// Tests for hash_line()
// ============================================================================

#[test]
fn test_hash_line_empty_string() {
    let ifc = setup_pass1_test();
    let h1 = ifc.hash_line("");
    let h2 = ifc.hash_line("");
    assert_eq!(h1, h2, "Identical empty strings should produce same hash");
}

#[test]
fn test_hash_line_single_character() {
    let ifc = setup_pass1_test();
    let h = ifc.hash_line("A");
    assert_ne!(h, 0, "Single character should produce non-zero hash");
}

#[test]
fn test_hash_line_two_characters() {
    let ifc = setup_pass1_test();
    let h = ifc.hash_line("AB");
    assert_ne!(h, 0, "Two characters should produce non-zero hash");
}

#[test]
fn test_hash_line_odd_length() {
    let ifc = setup_pass1_test();
    let h = ifc.hash_line("ABC");
    assert_ne!(h, 0, "Odd length string should produce non-zero hash");
}

#[test]
fn test_hash_line_identical_strings() {
    let ifc = setup_pass1_test();
    let h1 = ifc.hash_line("TEST");
    let h2 = ifc.hash_line("TEST");
    assert_eq!(h1, h2, "Identical strings should produce same hash");
}

#[test]
fn test_hash_line_different_strings() {
    let ifc = setup_pass1_test();
    let h1 = ifc.hash_line("TEST1");
    let h2 = ifc.hash_line("TEST2");
    assert_ne!(h1, h2, "Different strings should produce different hashes");
}

#[test]
fn test_hash_line_long_string() {
    let ifc = setup_pass1_test();
    let long_str = "X".repeat(100);
    let h = ifc.hash_line(&long_str);
    assert_ne!(h, 0, "Long string should produce non-zero hash");
}

#[test]
fn test_hash_line_special_characters() {
    let ifc = setup_pass1_test();
    let h1 = ifc.hash_line("Hello\n");
    let h2 = ifc.hash_line("Hello\t");
    assert_ne!(h1, h2, "Different special characters should produce different hashes");
}

#[test]
fn test_hash_line_unicode_or_special_bytes() {
    let ifc = setup_pass1_test();
    let str1 = "\x00\x01";
    let str2 = "\x01\x00";
    let h1 = ifc.hash_line(str1);
    let h2 = ifc.hash_line(str2);
    assert_ne!(h1, h2, "Different byte sequences should produce different hashes");
}

// ============================================================================
// Tests for hashcode_compare()
// ============================================================================

#[test]
fn test_hashcode_compare_equal() {
    let h1: u64 = 0x12345678;
    let h2: u64 = 0x12345678;
    let result = Ifcomp::hashcode_compare(h1, h2);
    assert_eq!(result, CompareResult::Eq, "Expected Eq");
}

#[test]
fn test_hashcode_compare_less_than() {
    let h1: u64 = 0x1000;
    let h2: u64 = 0x2000;
    let result = Ifcomp::hashcode_compare(h1, h2);
    assert_eq!(result, CompareResult::Lt, "Expected Lt");
}

#[test]
fn test_hashcode_compare_greater_than() {
    let h1: u64 = 0x2000;
    let h2: u64 = 0x1000;
    let result = Ifcomp::hashcode_compare(h1, h2);
    assert_eq!(result, CompareResult::Gt, "Expected Gt");
}

// ============================================================================
// Tests for make_line_entry()
// ============================================================================

#[test]
fn test_make_line_entry_basic() {
    let mut ifc = setup_pass1_test();
    let entry = ifc.make_line_entry(10, NULL_LINE_LIST);
    assert!(entry >= 0, "Should return valid entry index");
    assert_eq!(
        ifc.line_matching_state.line_table[entry as usize].linen,
        10,
        "Expected line 10"
    );
    assert_eq!(
        ifc.line_matching_state.line_table[entry as usize].next,
        NULL_LINE_LIST,
        "Next should be NULL_LINE_LIST"
    );
}

#[test]
fn test_make_line_entry_with_next() {
    let mut ifc = setup_pass1_test();
    let first = ifc.make_line_entry(1, NULL_LINE_LIST);
    let second = ifc.make_line_entry(2, first);
    assert_eq!(
        ifc.line_matching_state.line_table[second as usize].linen,
        2,
        "Expected line 2"
    );
    assert_eq!(
        ifc.line_matching_state.line_table[second as usize].next,
        first,
        "Next should point to first entry"
    );
}

#[test]
fn test_make_line_entry_chain() {
    let mut ifc = setup_pass1_test();
    let entry1 = ifc.make_line_entry(1, NULL_LINE_LIST);
    let entry2 = ifc.make_line_entry(2, entry1);
    let entry3 = ifc.make_line_entry(3, entry2);

    assert_eq!(
        ifc.line_matching_state.line_table[entry3 as usize].linen,
        3,
        "Expected line 3"
    );
    assert_eq!(
        ifc.line_matching_state.line_table[entry3 as usize].next,
        entry2,
        "entry3.Next should point to entry2"
    );
    assert_eq!(
        ifc.line_matching_state.line_table[entry2 as usize].linen,
        2,
        "Expected line 2"
    );
    assert_eq!(
        ifc.line_matching_state.line_table[entry2 as usize].next,
        entry1,
        "entry2.Next should point to entry1"
    );
    assert_eq!(
        ifc.line_matching_state.line_table[entry1 as usize].linen,
        1,
        "Expected line 1"
    );
    assert_eq!(
        ifc.line_matching_state.line_table[entry1 as usize].next,
        NULL_LINE_LIST,
        "entry1.Next should be NULL_LINE_LIST"
    );
}

// ============================================================================
// Tests for setup_distinct_text()
// ============================================================================

#[test]
fn test_setup_distinct_text_basic() {
    let mut ifc = setup_pass1_test();
    let si = ifc.setup_distinct_text("TEST".to_string(), 5, FileIndex::First);
    assert!(si > 0, "Should return valid string index");
    assert_eq!(
        ifc.line_matching_state.string_table[si].text,
        "TEST",
        "Expected 'TEST'"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[si].file_n_lines[0],
        1,
        "Expected 1 line in first file"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[si].file_n_lines[1],
        0,
        "Expected 0 lines in second file"
    );
    let line_entry = ifc.line_matching_state.string_table[si].file_list[0];
    assert_ne!(line_entry, NULL_LINE_LIST, "file_list[First] should be set");
    assert_eq!(
        ifc.line_matching_state.line_table[line_entry as usize].linen,
        5,
        "Expected line 5"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[si].file_list[1],
        NULL_LINE_LIST,
        "file_list[Second] should be NULL_LINE_LIST"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[si].next_text_with_same_hash,
        NULL_STRING_LIST,
        "next_text_with_same_hash should be NULL_STRING_LIST"
    );
}

#[test]
fn test_setup_distinct_text_second_file() {
    let mut ifc = setup_pass1_test();
    let si = ifc.setup_distinct_text("TEST".to_string(), 10, FileIndex::Second);
    assert_eq!(
        ifc.line_matching_state.string_table[si].file_n_lines[0],
        0,
        "Expected 0 lines in first file"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[si].file_n_lines[1],
        1,
        "Expected 1 line in second file"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[si].file_list[0],
        NULL_LINE_LIST,
        "file_list[First] should be NULL_LINE_LIST"
    );
    let line_entry = ifc.line_matching_state.string_table[si].file_list[1];
    assert_ne!(line_entry, NULL_LINE_LIST, "file_list[Second] should be set");
    assert_eq!(
        ifc.line_matching_state.line_table[line_entry as usize].linen,
        10,
        "Expected line 10"
    );
}

#[test]
fn test_setup_distinct_text_line_number_stored() {
    let mut ifc = setup_pass1_test();
    let si = ifc.setup_distinct_text("LINE".to_string(), 42, FileIndex::First);
    let line_entry = ifc.line_matching_state.string_table[si].file_list[0];
    assert_eq!(
        ifc.line_matching_state.line_table[line_entry as usize].linen,
        42,
        "Expected line 42"
    );
}

// ============================================================================
// Tests for setup_hash_node()
// ============================================================================

#[test]
fn test_setup_hash_node_basic() {
    let mut ifc = setup_pass1_test();
    let mut tip: usize = 0;
    let h = ifc.hash_line("TEST");
    let node_idx = ifc.setup_hash_node(&mut tip, "TEST".to_string(), 1, FileIndex::First, h);

    assert!(node_idx >= 0, "Should return valid node index");
    assert!(tip > 0, "tip should be set");
    assert_eq!(ifc.hash_state.hash_node[node_idx as usize].h, h, "Hash should match");
    assert_eq!(
        ifc.hash_state.hash_node[node_idx as usize].text_list,
        tip,
        "text_list should point to tip"
    );
    assert_eq!(
        ifc.hash_state.hash_node[node_idx as usize].next_in_bucket,
        NULL_HASH_LIST,
        "next_in_bucket should be NULL_HASH_LIST"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[tip].text,
        "TEST",
        "Expected 'TEST'"
    );
}

// ============================================================================
// Tests for add_linen_to_text_list()
// ============================================================================

#[test]
fn test_add_linen_to_text_list_first_file() {
    let mut ifc = setup_pass1_test();
    let si = ifc.setup_distinct_text("TEST".to_string(), 1, FileIndex::First);

    ifc.add_linen_to_text_list(si, 2, FileIndex::First);

    assert_eq!(
        ifc.line_matching_state.string_table[si].file_n_lines[0],
        2,
        "Expected 2 lines"
    );
    let list = ifc.line_matching_state.string_table[si].file_list[0];
    assert_ne!(list, NULL_LINE_LIST, "file_list should not be NULL");
    assert_eq!(
        ifc.line_matching_state.line_table[list as usize].linen,
        2,
        "Expected line 2 (most recent)"
    );
    let next_entry = ifc.line_matching_state.line_table[list as usize].next;
    assert_ne!(next_entry, NULL_LINE_LIST, "Next entry should exist");
    assert_eq!(
        ifc.line_matching_state.line_table[next_entry as usize].linen,
        1,
        "Expected line 1"
    );
}

#[test]
fn test_add_linen_to_text_list_second_file() {
    let mut ifc = setup_pass1_test();
    let si = ifc.setup_distinct_text("TEST".to_string(), 1, FileIndex::Second);

    ifc.add_linen_to_text_list(si, 5, FileIndex::Second);

    assert_eq!(
        ifc.line_matching_state.string_table[si].file_n_lines[1],
        2,
        "Expected 2 lines in second file"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[si].file_n_lines[0],
        0,
        "Expected 0 lines in first file"
    );
}

#[test]
fn test_add_linen_to_text_list_multiple_additions() {
    let mut ifc = setup_pass1_test();
    let si = ifc.setup_distinct_text("TEST".to_string(), 1, FileIndex::First);

    ifc.add_linen_to_text_list(si, 2, FileIndex::First);
    ifc.add_linen_to_text_list(si, 3, FileIndex::First);
    ifc.add_linen_to_text_list(si, 4, FileIndex::First);

    assert_eq!(
        ifc.line_matching_state.string_table[si].file_n_lines[0],
        4,
        "Expected 4 lines"
    );

    let mut list = ifc.line_matching_state.string_table[si].file_list[0];
    assert_eq!(
        ifc.line_matching_state.line_table[list as usize].linen,
        4,
        "Expected line 4"
    );
    list = ifc.line_matching_state.line_table[list as usize].next;
    assert_eq!(
        ifc.line_matching_state.line_table[list as usize].linen,
        3,
        "Expected line 3"
    );
    list = ifc.line_matching_state.line_table[list as usize].next;
    assert_eq!(
        ifc.line_matching_state.line_table[list as usize].linen,
        2,
        "Expected line 2"
    );
    list = ifc.line_matching_state.line_table[list as usize].next;
    assert_eq!(
        ifc.line_matching_state.line_table[list as usize].linen,
        1,
        "Expected line 1"
    );
    assert_eq!(
        ifc.line_matching_state.line_table[list as usize].next,
        NULL_LINE_LIST,
        "Last entry should have NULL_LINE_LIST as next"
    );
}

// ============================================================================
// Tests for enter_line()
// ============================================================================

#[test]
fn test_enter_line_first_entry_in_bucket() {
    let mut ifc = setup_pass1_test();
    let h = ifc.hash_line("TEST");
    let mut result_hash_node: i16 = 0;
    let mut result_string_index: usize = 0;

    ifc.enter_line("TEST", h, 1, FileIndex::First, &mut result_hash_node, &mut result_string_index);

    assert!(result_hash_node >= 0, "Should return valid hash node index");
    assert!(result_string_index > 0, "Should return valid string index");

    let bucket = (h % N_BUCKETS as u64) as usize;
    assert_eq!(
        ifc.hash_state.sec_hash_start_node[bucket],
        result_hash_node,
        "Bucket should point to result hash node"
    );
    assert_eq!(
        ifc.hash_state.hash_node[result_hash_node as usize].text_list,
        result_string_index,
        "Hash node text_list should point to result string index"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[result_string_index].text,
        "TEST",
        "Expected 'TEST'"
    );
}

#[test]
fn test_enter_line_duplicate_line_same_file() {
    let mut ifc = setup_pass1_test();
    let h = ifc.hash_line("TEST");
    let mut result_hash_node1: i16 = 0;
    let mut result_hash_node2: i16 = 0;
    let mut result_string_index1: usize = 0;
    let mut result_string_index2: usize = 0;

    ifc.enter_line("TEST", h, 1, FileIndex::First, &mut result_hash_node1, &mut result_string_index1);
    ifc.enter_line("TEST", h, 2, FileIndex::First, &mut result_hash_node2, &mut result_string_index2);

    assert_eq!(
        result_string_index1, result_string_index2,
        "Duplicate line should reuse same string entry"
    );
    assert_eq!(
        result_hash_node1, result_hash_node2,
        "Duplicate line should reuse same hash node"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[result_string_index1].file_n_lines[0],
        2,
        "Expected 2 occurrences"
    );
}

#[test]
fn test_enter_line_exact_match_reuses_string() {
    let mut ifc = setup_pass1_test();
    let h = ifc.hash_line("SAME");
    let mut node1: i16 = 0;
    let mut node2: i16 = 0;
    let mut si1: usize = 0;
    let mut si2: usize = 0;

    ifc.enter_line("SAME", h, 1, FileIndex::First, &mut node1, &mut si1);
    ifc.enter_line("SAME", h, 2, FileIndex::First, &mut node2, &mut si2);

    assert_eq!(si1, si2, "Exact text match should reuse string entry");
    assert_eq!(node1, node2, "Exact text match should use same hash node");
}

// ============================================================================
// Tests for read_lines()
// ============================================================================

#[test]
fn test_read_lines_single_line() {
    let mut ifc = setup_pass1_test();
    let input = "LINE1\n".as_bytes();
    ifc.read_lines(FileIndex::First, input).expect("read_lines should succeed");

    assert_eq!(ifc.file_state.total_file_n_lines[0], 1, "Expected 1 line");
    assert!(ifc.file_state.file_line[0].len() >= 2, "FileLine should have index 0 + line 1");
    assert_ne!(
        ifc.file_state.file_line[0][1].file_line_text,
        NULL_STRING_LIST,
        "file_line_text should be set"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[ifc.file_state.file_line[0][1].file_line_text].text,
        "LINE1",
        "Expected 'LINE1'"
    );
    assert_eq!(ifc.file_state.file_line[0][1].linen, 1, "Expected line 1");
    assert_eq!(
        ifc.file_state.file_line[0][1].ptr_type,
        LineType::SytType,
        "Expected SYT_TYPE"
    );
}

#[test]
fn test_read_lines_multiple_lines() {
    let mut ifc = setup_pass1_test();
    let input = "LINE1\nLINE2\nLINE3\n".as_bytes();
    ifc.read_lines(FileIndex::First, input).expect("read_lines should succeed");

    assert_eq!(ifc.file_state.total_file_n_lines[0], 3, "Expected 3 lines");
    assert_eq!(
        ifc.line_matching_state.string_table[ifc.file_state.file_line[0][1].file_line_text].text,
        "LINE1",
        "Expected 'LINE1'"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[ifc.file_state.file_line[0][2].file_line_text].text,
        "LINE2",
        "Expected 'LINE2'"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[ifc.file_state.file_line[0][3].file_line_text].text,
        "LINE3",
        "Expected 'LINE3'"
    );
}

#[test]
fn test_read_lines_empty_lines() {
    let mut ifc = setup_pass1_test();
    let input = "\n\nLINE\n".as_bytes();
    ifc.read_lines(FileIndex::First, input).expect("read_lines should succeed");

    assert_eq!(ifc.file_state.total_file_n_lines[0], 3, "Expected 3 lines");
    assert_eq!(
        ifc.line_matching_state.string_table[ifc.file_state.file_line[0][1].file_line_text].text,
        "",
        "Expected empty line"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[ifc.file_state.file_line[0][2].file_line_text].text,
        "",
        "Expected empty line"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[ifc.file_state.file_line[0][3].file_line_text].text,
        "LINE",
        "Expected 'LINE'"
    );
}

#[test]
fn test_read_lines_no_trailing_newline() {
    let mut ifc = setup_pass1_test();
    let input = "LINE1\nLINE2".as_bytes();
    ifc.read_lines(FileIndex::First, input).expect("read_lines should succeed");

    assert_eq!(ifc.file_state.total_file_n_lines[0], 2, "Expected 2 lines");
    assert_eq!(
        ifc.line_matching_state.string_table[ifc.file_state.file_line[0][1].file_line_text].text,
        "LINE1",
        "Expected 'LINE1'"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[ifc.file_state.file_line[0][2].file_line_text].text,
        "LINE2",
        "Expected 'LINE2'"
    );
}

#[test]
fn test_read_lines_long_line() {
    let mut ifc = setup_pass1_test();
    let long_line = "X".repeat(1000);
    let input_str = format!("{}\n", long_line);
    let input = input_str.as_bytes();
    ifc.read_lines(FileIndex::First, input).expect("read_lines should succeed");

    assert_eq!(ifc.file_state.total_file_n_lines[0], 1, "Expected 1 line");
    assert_eq!(
        ifc.line_matching_state.string_table[ifc.file_state.file_line[0][1].file_line_text].text,
        long_line,
        "Long line text should match"
    );
}

#[test]
fn test_read_lines_special_characters() {
    let mut ifc = setup_pass1_test();
    let input = "LINE\tWITH\tTABS\nLINE WITH SPACES\n".as_bytes();
    ifc.read_lines(FileIndex::First, input).expect("read_lines should succeed");

    assert_eq!(ifc.file_state.total_file_n_lines[0], 2, "Expected 2 lines");
    assert_eq!(
        ifc.line_matching_state.string_table[ifc.file_state.file_line[0][1].file_line_text].text,
        "LINE\tWITH\tTABS",
        "Expected line with tabs"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[ifc.file_state.file_line[0][2].file_line_text].text,
        "LINE WITH SPACES",
        "Expected line with spaces"
    );
}

// ============================================================================
// Tests for pass1()
// ============================================================================

#[test]
fn test_pass1_two_identical_files() {
    let mut ifc = setup_pass1_test();
    let file1 = "A\nB\nC\n".as_bytes();
    let file2 = "A\nB\nC\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");

    assert_eq!(ifc.file_state.total_file_n_lines[0], 3, "Expected 3 lines in first file");
    assert_eq!(ifc.file_state.total_file_n_lines[1], 3, "Expected 3 lines in second file");

    let si1a = ifc.file_state.file_line[0][1].file_line_text;
    let si2a = ifc.file_state.file_line[1][1].file_line_text;
    assert_eq!(
        ifc.line_matching_state.string_table[si1a].text,
        "A",
        "Expected 'A'"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[si2a].text,
        "A",
        "Expected 'A'"
    );
    assert!(si1a > 0 && si2a > 0, "String indices should be valid");
}

#[test]
fn test_pass1_two_different_files() {
    let mut ifc = setup_pass1_test();
    let file1 = "A\nB\n".as_bytes();
    let file2 = "C\nD\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");

    assert_eq!(ifc.file_state.total_file_n_lines[0], 2, "Expected 2 lines in first file");
    assert_eq!(ifc.file_state.total_file_n_lines[1], 2, "Expected 2 lines in second file");

    let si_a = ifc.file_state.file_line[0][1].file_line_text;
    let si_c = ifc.file_state.file_line[1][1].file_line_text;
    assert_ne!(si_a, si_c, "Different lines should map to different string entries");
}

#[test]
fn test_pass1_partial_overlap() {
    let mut ifc = setup_pass1_test();
    let file1 = "A\nB\nC\n".as_bytes();
    let file2 = "A\nX\nC\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");

    assert_eq!(ifc.file_state.total_file_n_lines[0], 3, "Expected 3 lines in first file");
    assert_eq!(ifc.file_state.total_file_n_lines[1], 3, "Expected 3 lines in second file");

    let si_a1 = ifc.file_state.file_line[0][1].file_line_text;
    let si_a2 = ifc.file_state.file_line[1][1].file_line_text;
    assert_eq!(
        ifc.line_matching_state.string_table[si_a1].text,
        "A",
        "Expected 'A'"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[si_a2].text,
        "A",
        "Expected 'A'"
    );

    let si_c1 = ifc.file_state.file_line[0][3].file_line_text;
    let si_c2 = ifc.file_state.file_line[1][3].file_line_text;
    assert_eq!(
        ifc.line_matching_state.string_table[si_c1].text,
        "C",
        "Expected 'C'"
    );
    assert_eq!(
        ifc.line_matching_state.string_table[si_c2].text,
        "C",
        "Expected 'C'"
    );
}

#[test]
fn test_pass1_clears_hash_nodes_after_completion() {
    let mut ifc = setup_pass1_test();
    let file1 = "A\nB\n".as_bytes();
    let file2 = "C\nD\n".as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");

    assert_eq!(
        ifc.hash_state.hash_node.len(),
        0,
        "pass1 should clear hash_node after completion"
    );
}

#[test]
fn test_pass1_file_with_many_lines() {
    let mut ifc = setup_pass1_test();
    let mut file1_content = String::new();
    let mut file2_content = String::new();
    for i in 0..100 {
        file1_content.push_str(&format!("LINE{}\n", i));
        file2_content.push_str(&format!("LINE{}\n", i));
    }

    let file1 = file1_content.as_bytes();
    let file2 = file2_content.as_bytes();

    ifc.pass1(file1, file2).expect("pass1 should succeed");

    assert_eq!(ifc.file_state.total_file_n_lines[0], 100, "Expected 100 lines in first file");
    assert_eq!(ifc.file_state.total_file_n_lines[1], 100, "Expected 100 lines in second file");
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

#[test]
fn test_hash_line_all_ascii_characters() {
    let ifc = setup_pass1_test();
    let mut all_chars = String::new();
    for i in 1..128 {
        all_chars.push(i as u8 as char);
    }
    let h = ifc.hash_line(&all_chars);
    assert_ne!(h, 0, "All ASCII characters string should produce non-zero hash");
}

#[test]
fn test_make_line_entry_many_entries() {
    let mut ifc = setup_pass1_test();
    let mut prev = NULL_LINE_LIST;
    for i in 1..=100 {
        prev = ifc.make_line_entry(i as LineCount, prev);
    }

    let mut current = prev;
    for i in (1..=100).rev() {
        assert_eq!(
            ifc.line_matching_state.line_table[current as usize].linen,
            i as LineCount,
            "Expected line {}",
            i
        );
        current = ifc.line_matching_state.line_table[current as usize].next;
    }
    assert_eq!(current, NULL_LINE_LIST, "Last entry should have NULL_LINE_LIST as next");
}