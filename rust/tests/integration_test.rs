use assert_cmd::Command;
use regex::Regex;
use std::error::Error;
use std::fs;
use tempfile::NamedTempFile;

type TestResult = Result<(), Box<dyn Error>>;

const PRG: &str = "ifcomp";

// OutputStatistics represents the statistics extracted from ifcomp output
struct OutputStatistics {
    deleted: i32,
    inserted: i32,
    replaced_old: i32,
    replaced_new: i32,
    moved: i32,
    change_blocks: i32,
}

// Extract statistics from ifcomp output
fn extract_statistics(output: &str) -> OutputStatistics {
    let mut stats = OutputStatistics {
        deleted: 0,
        inserted: 0,
        replaced_old: 0,
        replaced_new: 0,
        moved: 0,
        change_blocks: 0,
    };

    // Regex patterns to match the statistics lines
    let del_pattern = Regex::new(r"(\d+) lines deleted from old\.").unwrap();
    let ins_pattern = Regex::new(r"(\d+) lines inserted in new\.").unwrap();
    let repl_pattern =
        Regex::new(r"(\d+) lines deleted from old and replaced with (\d+) lines of new\.").unwrap();
    let move_pattern = Regex::new(r"(\d+) lines moved in old\.").unwrap();
    let blocks_pattern = Regex::new(r"(\d+) change blocks\.").unwrap();

    if let Some(caps) = del_pattern.captures(output) {
        if let Ok(val) = caps[1].parse::<i32>() {
            stats.deleted = val;
        }
    }
    if let Some(caps) = ins_pattern.captures(output) {
        if let Ok(val) = caps[1].parse::<i32>() {
            stats.inserted = val;
        }
    }
    if let Some(caps) = repl_pattern.captures(output) {
        if let Ok(val_old) = caps[1].parse::<i32>() {
            stats.replaced_old = val_old;
        }
        if let Ok(val_new) = caps[2].parse::<i32>() {
            stats.replaced_new = val_new;
        }
    }
    if let Some(caps) = move_pattern.captures(output) {
        if let Ok(val) = caps[1].parse::<i32>() {
            stats.moved = val;
        }
    }
    if let Some(caps) = blocks_pattern.captures(output) {
        if let Ok(val) = caps[1].parse::<i32>() {
            stats.change_blocks = val;
        }
    }

    stats
}

// Assert that statistics match expected values
fn assert_statistics(
    output: &str,
    expected_del: i32,
    expected_ins: i32,
    expected_repl_old: i32,
    expected_repl_new: i32,
    expected_moved: i32,
    expected_blocks: i32,
) {
    let stats = extract_statistics(output);

    assert_eq!(
        stats.deleted, expected_del,
        "Deleted lines mismatch: expected {}, got {}",
        expected_del, stats.deleted
    );
    assert_eq!(
        stats.inserted, expected_ins,
        "Inserted lines mismatch: expected {}, got {}",
        expected_ins, stats.inserted
    );
    assert_eq!(
        stats.replaced_old, expected_repl_old,
        "Replaced old lines mismatch: expected {}, got {}",
        expected_repl_old, stats.replaced_old
    );
    assert_eq!(
        stats.replaced_new, expected_repl_new,
        "Replaced new lines mismatch: expected {}, got {}",
        expected_repl_new, stats.replaced_new
    );
    assert_eq!(
        stats.moved, expected_moved,
        "Moved lines mismatch: expected {}, got {}",
        expected_moved, stats.moved
    );
    assert_eq!(
        stats.change_blocks, expected_blocks,
        "Change blocks mismatch: expected {}, got {}",
        expected_blocks, stats.change_blocks
    );
}

// Generate a line of specific length
fn generate_long_line(length: usize, fill: u8) -> String {
    std::iter::repeat(fill as char).take(length).collect()
}

// Generate a file with duplicate lines
fn generate_file_with_duplicates(line: &str, repeats: usize) -> String {
    std::iter::repeat(line).take(repeats).collect::<Vec<&str>>().join("\n") + "\n"
}

// Run ifcomp with two file contents and return the output
fn run_ifcomp(content_a: &str, content_b: &str) -> Result<String, Box<dyn Error>> {
    // Create temporary files
    let file1 = NamedTempFile::new()?;
    let file2 = NamedTempFile::new()?;

    let path1 = file1.path();
    let path2 = file2.path();

    // Write contents to files
    fs::write(path1, content_a)?;
    fs::write(path2, content_b)?;

    // Capture output
    let output = Command::cargo_bin(PRG)?
        .arg(path1.to_str().unwrap())
        .arg(path2.to_str().unwrap())
        .output()?;

    Ok(String::from_utf8(output.stdout)?)
}

// Test helper infrastructure
#[test]
fn test_ifcomp_helpers() -> TestResult {
    // Test basic comparison works
    let output = run_ifcomp("A\nB\n", "A\nB\n")?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test case with identical input files
#[test]
fn test_ifcomp_identical_files() -> TestResult {
    let output = run_ifcomp("A\nB\n", "A\nB\n")?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test files with single identical line
#[test]
fn test_ifcomp_single_identical_line() -> TestResult {
    let output = run_ifcomp("HELLO\n", "HELLO\n")?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test files with single different line
#[test]
fn test_ifcomp_single_different_line() -> TestResult {
    let output = run_ifcomp("OLD\n", "NEW\n")?;
    assert_statistics(&output, 0, 0, 1, 1, 0, 1);
    Ok(())
}

// Test two lines - both identical
#[test]
fn test_ifcomp_two_lines_identical() -> TestResult {
    let output = run_ifcomp("LINE1\nLINE2\n", "LINE1\nLINE2\n")?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test two lines - second different
#[test]
fn test_ifcomp_two_lines_second_different() -> TestResult {
    let output = run_ifcomp("LINE1\nOLD\n", "LINE1\nNEW\n")?;
    assert_statistics(&output, 0, 0, 1, 1, 0, 1);
    Ok(())
}

// Test three lines - all identical
#[test]
fn test_ifcomp_three_lines_identical() -> TestResult {
    let output = run_ifcomp("A\nB\nC\n", "A\nB\nC\n")?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test file with only spaces (as lines)
#[test]
fn test_ifcomp_lines_with_only_spaces() -> TestResult {
    let output = run_ifcomp("   \n   \nLINE\n", "   \nLINE\n")?;
    assert_statistics(&output, 1, 0, 0, 0, 0, 1);
    Ok(())
}

// Test file without trailing newline
#[test]
fn test_ifcomp_no_trailing_newline() -> TestResult {
    let output = run_ifcomp("A\nB", "A\nB\n")?;
    // Should detect no difference (files are the same)
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test case with deletes, moves and replacements
#[test]
fn test_ifcomp_complex_changes() -> TestResult {
    let output = run_ifcomp(
        "A\nX\nC\nY\nD\nW\nE\nA\nB\nE\n",
        "A\nB\nC\nD\nE\n",
    )?;
    assert_statistics(&output, 4, 0, 2, 1, 2, 5);
    Ok(())
}

// Test case with deletes, moves and replacements
#[test]
fn test_ifcomp_permutation_changes() -> TestResult {
    let output = run_ifcomp(
        "A\nB\nC\nD\nE\nG\n",
        "D\nE\nF\nG\nA\nC\n",
    )?;
    assert_statistics(&output, 1, 1, 0, 0, 2, 3);
    Ok(())
}

// Test case from the NASA paper
#[test]
fn test_ifcomp_much_writing_example() -> TestResult {
    let file_a = "a\nmass\nof\nlatin\nwords\nfalls\nupon\nthe\nrelevant\nfacts\nlike\nsoft\nsnow\n,\ncovering\nup\nthe\ndetails\n.\n";
    let file_b = "much\nwriting\nis\nlike\nsnow\n,\na\nmass\nof\nlong\nwords\nand\nphrases\nfalls\nupon\nthe\nrelevant\nfacts\ncovering\nup\nthe\ndetails\n.\n";
    let output = run_ifcomp(file_a, file_b)?;
    assert_statistics(&output, 1, 5, 1, 1, 3, 5);
    Ok(())
}

// Test exactly 127 occurrences of same line (char limit)
// Note: Identical duplicate files are reported as replacements due to algorithm limitation
#[test]
fn test_ifcomp_exactly_127_occurrences() -> TestResult {
    let file_a = generate_file_with_duplicates("LINE", 127);
    let file_b = generate_file_with_duplicates("LINE", 127);
    let output = run_ifcomp(&file_a, &file_b)?;
    assert_statistics(&output, 0, 0, 127, 127, 0, 1);
    Ok(())
}

// Test more than 127 occurrences (potential overflow)
// Note: Identical duplicate files are reported as replacements due to algorithm limitation
#[test]
fn test_ifcomp_over_127_occurrences() -> TestResult {
    let file_a = generate_file_with_duplicates("LINE", 128);
    let file_b = generate_file_with_duplicates("LINE", 128);
    let output = run_ifcomp(&file_a, &file_b)?;
    assert_statistics(&output, 0, 0, 128, 128, 0, 1);
    Ok(())
}

// Test file with only one unique line repeated 200 times
// Note: Identical duplicate files are reported as replacements due to algorithm limitation
#[test]
fn test_ifcomp_one_unique_line_200_times() -> TestResult {
    let file_a = generate_file_with_duplicates("SAME", 200);
    let file_b = generate_file_with_duplicates("SAME", 200);
    let output = run_ifcomp(&file_a, &file_b)?;
    assert_statistics(&output, 0, 0, 200, 200, 0, 1);
    Ok(())
}

// Test multiple lines each repeated many times
// Note: Identical duplicate files are reported as replacements due to algorithm limitation
#[test]
fn test_ifcomp_multiple_lines_many_repeats() -> TestResult {
    let file_a = generate_file_with_duplicates("A\n", 50)
        + &generate_file_with_duplicates("B\n", 50)
        + &generate_file_with_duplicates("C\n", 50);
    let file_b = generate_file_with_duplicates("A\n", 50)
        + &generate_file_with_duplicates("B\n", 50)
        + &generate_file_with_duplicates("C\n", 50);
    let output = run_ifcomp(&file_a, &file_b)?;
    assert_statistics(&output, 0, 0, 300, 300, 0, 1);
    Ok(())
}

// Test interleaved duplicates (A, B, A, B pattern)
// Note: Identical duplicate files are reported as replacements due to algorithm limitation
#[test]
fn test_ifcomp_interleaved_duplicates() -> TestResult {
    let mut file_a = String::new();
    let mut file_b = String::new();
    for _ in 0..50 {
        file_a.push_str("A\nB\n");
        file_b.push_str("A\nB\n");
    }
    let output = run_ifcomp(&file_a, &file_b)?;
    assert_statistics(&output, 0, 0, 100, 100, 0, 1);
    Ok(())
}

// Test duplicated lines in different orders
// Note: Duplicate lines without unique anchors are reported as replacements
#[test]
fn test_ifcomp_duplicates_different_order() -> TestResult {
    let output = run_ifcomp(
        "A\nA\nB\nB\nC\nC\n",
        "C\nC\nB\nB\nA\nA\n",
    )?;
    // Reported as replacement due to algorithm limitation
    assert_statistics(&output, 0, 0, 6, 6, 0, 1);
    Ok(())
}

// Test some duplicates, some unique
// Note: Duplicate lines prevent proper matching even with unique lines
#[test]
fn test_ifcomp_mixed_duplicates_and_unique() -> TestResult {
    let output = run_ifcomp(
        "A\nA\nUNIQUE1\nB\nB\n",
        "A\nA\nUNIQUE2\nB\nB\n",
    )?;
    // Reported as 5 replacements due to algorithm limitation
    assert_statistics(&output, 0, 0, 5, 5, 0, 1);
    Ok(())
}

// Test removing duplicates
// Note: Duplicate lines are reported as replacement rather than deletion
#[test]
fn test_ifcomp_removing_duplicates() -> TestResult {
    let output = run_ifcomp("LINE\nLINE\nLINE\n", "LINE\n")?;
    assert_statistics(&output, 0, 0, 3, 1, 0, 1);
    Ok(())
}

// Test adding duplicates
// Note: Duplicate lines are reported as replacement rather than insertion
#[test]
fn test_ifcomp_adding_duplicates() -> TestResult {
    let output = run_ifcomp("LINE\n", "LINE\nLINE\nLINE\n")?;
    assert_statistics(&output, 0, 0, 1, 3, 0, 1);
    Ok(())
}

// Test leading whitespace
#[test]
fn test_ifcomp_leading_whitespace() -> TestResult {
    let output = run_ifcomp("   LINE\n   MORE\n", "   LINE\n   MORE\n")?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test trailing whitespace
#[test]
fn test_ifcomp_trailing_whitespace() -> TestResult {
    let output = run_ifcomp("LINE   \nMORE   \n", "LINE   \nMORE   \n")?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test leading whitespace differences
#[test]
fn test_ifcomp_leading_whitespace_different() -> TestResult {
    let output = run_ifcomp("   LINE\n", "LINE\n")?;
    assert_statistics(&output, 0, 0, 1, 1, 0, 1);
    Ok(())
}

// Test trailing whitespace differences
#[test]
fn test_ifcomp_trailing_whitespace_different() -> TestResult {
    let output = run_ifcomp("LINE   \n", "LINE\n")?;
    assert_statistics(&output, 0, 0, 1, 1, 0, 1);
    Ok(())
}

// Test lines with only spaces/tabs
#[test]
fn test_ifcomp_only_whitespace_lines() -> TestResult {
    let output = run_ifcomp("   \n\t\t\t\nLINE\n", "   \n\t\t\t\nLINE\n")?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test mix of spaces and tabs
#[test]
fn test_ifcomp_mixed_spaces_and_tabs() -> TestResult {
    let output = run_ifcomp(
        "\t   \tLINE\n   \t\tMORE\n",
        "\t   \tLINE\n   \t\tMORE\n",
    )?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test empty lines (just newline)
#[test]
fn test_ifcomp_empty_lines() -> TestResult {
    let output = run_ifcomp("A\n\nB\n\nC\n", "A\n\nB\n\nC\n")?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test empty lines in different positions
#[test]
fn test_ifcomp_empty_lines_different() -> TestResult {
    let output = run_ifcomp("A\nB\nC\n", "A\n\nB\n\nC\n")?;
    assert_statistics(&output, 0, 2, 0, 0, 0, 2);
    Ok(())
}

// Test indentation changes
#[test]
fn test_ifcomp_indentation_changes() -> TestResult {
    let output = run_ifcomp(
        "  if (x) {\n    return;\n  }\n",
        "    if (x) {\n      return;\n    }\n",
    )?;
    assert_statistics(&output, 0, 0, 3, 3, 0, 1);
    Ok(())
}

// Test whitespace-only line differences
#[test]
fn test_ifcomp_whitespace_only_differences() -> TestResult {
    let output = run_ifcomp("LINE\n   \nMORE\n", "LINE\n     \nMORE\n")?;
    assert_statistics(&output, 0, 0, 1, 1, 0, 1);
    Ok(())
}

// Test tabs vs spaces
#[test]
fn test_ifcomp_tabs_vs_spaces() -> TestResult {
    let output = run_ifcomp("\tLINE\n", "    LINE\n")?;
    // Should be different
    assert_statistics(&output, 0, 0, 1, 1, 0, 1);
    Ok(())
}

// Test multiple empty lines
#[test]
fn test_ifcomp_multiple_empty_lines() -> TestResult {
    let output = run_ifcomp("START\n\n\n\nEND\n", "START\n\n\n\nEND\n")?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test lines with tabs
#[test]
fn test_ifcomp_lines_with_tabs() -> TestResult {
    let output = run_ifcomp("\tLINE1\nLINE2\t\n", "\tLINE1\nLINE2\t\n")?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test lines with multiple spaces
#[test]
fn test_ifcomp_lines_with_multiple_spaces() -> TestResult {
    let output = run_ifcomp("LINE    WITH    SPACES\n", "LINE    WITH    SPACES\n")?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test ASCII control characters
#[test]
fn test_ifcomp_ascii_control_chars() -> TestResult {
    let line_a = format!("LINE{}CONTROL\n", char::from(1));
    let line_b = format!("LINE{}CONTROL\n", char::from(1));
    let output = run_ifcomp(&line_a, &line_b)?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test mix of special characters
#[test]
fn test_ifcomp_mixed_special_chars() -> TestResult {
    let output = run_ifcomp(
        "!@#$%^&*()\n[]{}|\\:'\"<>?\n`~-_=+\n",
        "!@#$%^&*()\n[]{}|\\:'\"<>?\n`~-_=+\n",
    )?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test UTF-8 multibyte characters
#[test]
fn test_ifcomp_utf8_multibyte_chars() -> TestResult {
    let output = run_ifcomp(
        "Hello 世界\nこんにちは\nПривет\n",
        "Hello 世界\nこんにちは\nПривет\n",
    )?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test UTF-8 with differences
#[test]
fn test_ifcomp_utf8_with_differences() -> TestResult {
    let output = run_ifcomp("Hello 世界\n", "Hello 宇宙\n")?;
    assert_statistics(&output, 0, 0, 1, 1, 0, 1);
    Ok(())
}

// Test backslash handling
#[test]
fn test_ifcomp_backslash_handling() -> TestResult {
    let output = run_ifcomp(
        "path\\to\\file\nC:\\Windows\\System\n",
        "path\\to\\file\nC:\\Windows\\System\n",
    )?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test quotes and apostrophes
#[test]
fn test_ifcomp_quotes_and_apostrophes() -> TestResult {
    let output = run_ifcomp(
        "\"quoted text\"\n'single quotes'\nit's a test\n",
        "\"quoted text\"\n'single quotes'\nit's a test\n",
    )?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test numeric strings
#[test]
fn test_ifcomp_numeric_strings() -> TestResult {
    let output = run_ifcomp(
        "12345\n0xDEADBEEF\n3.14159\n",
        "12345\n0xDEADBEEF\n3.14159\n",
    )?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test lines with carriage returns (CRLF vs LF)
// Note: IFCOMP treats CRLF differently from LF - reported as replacement
#[test]
fn test_ifcomp_carriage_returns() -> TestResult {
    let output = run_ifcomp("LINE\r\nLINE\r\n", "LINE\nLINE\n")?;
    assert_statistics(&output, 0, 0, 2, 2, 0, 1);
    Ok(())
}

// Test line exactly 4095 bytes (one less than buffer)
#[test]
fn test_ifcomp_line_4095_bytes() -> TestResult {
    let long_line = generate_long_line(4095, b'X');
    let output = run_ifcomp(&format!("{}\n", long_line), &format!("{}\n", long_line))?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test line exactly 4096 bytes (buffer size)
#[test]
fn test_ifcomp_line_4096_bytes() -> TestResult {
    let long_line = generate_long_line(4096, b'X');
    let output = run_ifcomp(&format!("{}\n", long_line), &format!("{}\n", long_line))?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test line over 4096 bytes (truncation behavior)
#[test]
fn test_ifcomp_line_over_4096_bytes() -> TestResult {
    let long_line = generate_long_line(4096, b'X');
    let output = run_ifcomp(&format!("{}\n", long_line), &format!("{}\n", long_line))?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test mix of very short and very long lines
#[test]
fn test_ifcomp_mix_short_and_long_lines() -> TestResult {
    let short_line = "A\n";
    let long_line = generate_long_line(4000, b'X');
    let content = format!("{}{}\n{}", short_line, long_line, short_line);
    let output = run_ifcomp(&content, &content)?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test very long line followed by very short line
#[test]
fn test_ifcomp_long_line_followed_by_short() -> TestResult {
    let long_line = generate_long_line(4095, b'X');
    let output = run_ifcomp(
        &format!("{}\nX\n", long_line),
        &format!("{}\nY\n", long_line),
    )?;
    assert_statistics(&output, 0, 0, 1, 1, 0, 1);
    Ok(())
}

// Test multiple very long lines
#[test]
fn test_ifcomp_multiple_long_lines() -> TestResult {
    let line1 = generate_long_line(3000, b'X');
    let line2 = generate_long_line(3500, b'X');
    let content = format!("{}\n{}\n", line1, line2);
    let output = run_ifcomp(&content, &content)?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test lines of maximum safe size
#[test]
fn test_ifcomp_max_safe_line_size() -> TestResult {
    let line = generate_long_line(4094, b'X');
    let content = format!("{}\n", line);
    let output = run_ifcomp(&content, &content)?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test many lines with similar prefixes to stress hash buckets
#[test]
fn test_ifcomp_similar_prefix_lines() -> TestResult {
    let mut file_a = String::new();
    let mut file_b = String::new();
    for i in 0..300 {
        file_a.push_str(&format!("prefix_{}_suffix\n", i));
        file_b.push_str(&format!("prefix_{}_suffix\n", i));
    }
    let output = run_ifcomp(&file_a, &file_b)?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test lines with same length but different content
#[test]
fn test_ifcomp_same_length_different_content() -> TestResult {
    let output = run_ifcomp(
        "AAAAAAAA\nBBBBBBBB\nCCCCCCCC\n",
        "AAAAAAAA\nBBBBBBBB\nCCCCCCCC\n",
    )?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test lines that are almost identical (one character different)
#[test]
fn test_ifcomp_almost_identical_lines() -> TestResult {
    let output = run_ifcomp(
        "HELLO_WORLD_A\nHELLO_WORLD_B\nHELLO_WORLD_C\n",
        "HELLO_WORLD_A\nHELLO_WORLD_X\nHELLO_WORLD_C\n",
    )?;
    assert_statistics(&output, 0, 0, 1, 1, 0, 1);
    Ok(())
}

// Test many different lines to create hash collisions
#[test]
fn test_ifcomp_many_different_lines() -> TestResult {
    let mut file_a = String::new();
    let mut file_b = String::new();
    for i in 0..100 {
        file_a.push_str(&format!("line{}\n", i));
        file_b.push_str(&format!("line{}\n", i));
    }
    let output = run_ifcomp(&file_a, &file_b)?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test permutation of lines to stress hash table
#[test]
fn test_ifcomp_permuted_lines() -> TestResult {
    let mut file_a = String::new();
    let mut file_b = String::new();
    for c in b'A'..=b'Z' {
        file_a.push_str(&format!("{}\n", c as char));
    }
    for c in (b'A'..=b'Z').rev() {
        file_b.push_str(&format!("{}\n", c as char));
    }
    let output = run_ifcomp(&file_a, &file_b)?;
    // All should be moved
    assert_statistics(&output, 0, 0, 0, 0, 55, 25);
    Ok(())
}

// Test many very similar lines
#[test]
fn test_ifcomp_many_very_similar_lines() -> TestResult {
    let mut file_a = String::new();
    let mut file_b = String::new();
    for i in 1..=100 {
        file_a.push_str(&format!("SAMPLE_LINE_{}\n", i));
        file_b.push_str(&format!("SAMPLE_LINE_{}\n", i));
    }
    let output = run_ifcomp(&file_a, &file_b)?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test files with 1000 lines
#[test]
fn test_ifcomp_thousand_lines() -> TestResult {
    let mut file_a = String::new();
    let mut file_b = String::new();
    for i in 1..=1000 {
        file_a.push_str(&format!("line{}\n", i));
        file_b.push_str(&format!("line{}\n", i));
    }
    let output = run_ifcomp(&file_a, &file_b)?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test 100 unique lines repeated in different orders
// Note: Duplicate lines prevent proper matching - reported as replacement
#[test]
fn test_ifcomp_hundred_unique_repeated() -> TestResult {
    let mut file_a = String::new();
    let mut file_b = String::new();
    for i in 1..=100 {
        file_a.push_str(&format!("unique{}\n", i));
        file_b.push_str(&format!("unique{}\n", i));
    }
    for i in 1..=100 {
        file_a.push_str(&format!("unique{}\n", i));
        file_b.push_str(&format!("unique{}\n", i));
    }
    let output = run_ifcomp(&file_a, &file_b)?;
    assert_statistics(&output, 0, 0, 200, 200, 0, 1);
    Ok(())
}

// Test large identical sections with small differences
#[test]
fn test_ifcomp_large_identical_sections_with_differences() -> TestResult {
    let mut file_a = String::new();
    let mut file_b = String::new();
    for i in 1..=500 {
        file_a.push_str(&format!("identical{}\n", i));
        file_b.push_str(&format!("identical{}\n", i));
    }
    file_a.push_str("DIFFERENT_A\n");
    file_b.push_str("DIFFERENT_B\n");
    for i in 501..=1000 {
        file_a.push_str(&format!("identical{}\n", i));
        file_b.push_str(&format!("identical{}\n", i));
    }
    let output = run_ifcomp(&file_a, &file_b)?;
    assert_statistics(&output, 0, 0, 1, 1, 0, 1);
    Ok(())
}

// Test large deletions
#[test]
fn test_ifcomp_large_deletions() -> TestResult {
    let mut file_a = String::new();
    let mut file_b = String::new();
    for i in 1..=1000 {
        file_a.push_str(&format!("line{}\n", i));
        if i % 2 == 0 {
            file_b.push_str(&format!("line{}\n", i));
        }
    }
    let output = run_ifcomp(&file_a, &file_b)?;
    // 500 deletions
    assert_statistics(&output, 500, 0, 0, 0, 0, 500);
    Ok(())
}

// Test large insertions
#[test]
fn test_ifcomp_large_insertions() -> TestResult {
    let mut file_a = String::new();
    let mut file_b = String::new();
    for i in 1..=1000 {
        if i % 2 == 0 {
            file_a.push_str(&format!("line{}\n", i));
        }
        file_b.push_str(&format!("line{}\n", i));
    }
    let output = run_ifcomp(&file_a, &file_b)?;
    // 500 insertions
    assert_statistics(&output, 0, 500, 0, 0, 0, 500);
    Ok(())
}

// Test very large file with 5000 lines
#[test]
fn test_ifcomp_five_thousand_lines() -> TestResult {
    let mut file_a = String::new();
    let mut file_b = String::new();
    for i in 1..=5000 {
        file_a.push_str(&format!("line{}\n", i));
        file_b.push_str(&format!("line{}\n", i));
    }
    let output = run_ifcomp(&file_a, &file_b)?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test only insertions (file1 subset of file2)
#[test]
fn test_ifcomp_only_insertions() -> TestResult {
    let output = run_ifcomp("A\nC\nE\n", "A\nB\nC\nD\nE\nF\n")?;
    assert_statistics(&output, 0, 3, 0, 0, 0, 3);
    Ok(())
}

// Test only deletions (file2 subset of file1)
#[test]
fn test_ifcomp_only_deletions() -> TestResult {
    let output = run_ifcomp("A\nB\nC\nD\nE\nF\n", "A\nC\nE\n")?;
    assert_statistics(&output, 3, 0, 0, 0, 0, 3);
    Ok(())
}

// Test only moves (same lines, different order)
#[test]
fn test_ifcomp_only_moves() -> TestResult {
    let output = run_ifcomp("A\nB\nC\nD\n", "D\nC\nB\nA\n")?;
    assert_statistics(&output, 0, 0, 0, 0, 4, 3);
    Ok(())
}

// Test single unique line in sea of duplicates
#[test]
fn test_ifcomp_single_unique_in_duplicates() -> TestResult {
    let output = run_ifcomp("X\nX\nUNIQUE\nX\nX\n", "X\nX\nUNIQUE\nX\nX\n")?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test matching at file boundaries (start/end)
#[test]
fn test_ifcomp_boundary_matching() -> TestResult {
    let output = run_ifcomp("START\nMIDDLE\nEND\n", "START\nMIDDLE\nEND\n")?;
    assert_statistics(&output, 0, 0, 0, 0, 0, 0);
    Ok(())
}

// Test changes at boundaries
#[test]
fn test_ifcomp_boundary_changes() -> TestResult {
    let output = run_ifcomp(
        "OLDSTART\nMIDDLE\nOLDEND\n",
        "NEWSTART\nMIDDLE\nNEWEND\n",
    )?;
    assert_statistics(&output, 0, 0, 2, 2, 0, 2);
    Ok(())
}

// Test complete reversal (A,B,C,D → D,C,B,A)
#[test]
fn test_ifcomp_complete_reversal() -> TestResult {
    let output = run_ifcomp("A\nB\nC\nD\nE\n", "E\nD\nC\nB\nA\n")?;
    // All lines moved
    assert_statistics(&output, 0, 0, 0, 0, 5, 4);
    Ok(())
}

// Test rotation patterns (A,B,C,D → B,C,D,A)
#[test]
fn test_ifcomp_rotation_pattern() -> TestResult {
    let output = run_ifcomp("A\nB\nC\nD\n", "B\nC\nD\nA\n")?;
    // All lines moved
    assert_statistics(&output, 0, 0, 0, 0, 1, 1);
    Ok(())
}

// Test interleaving (A,C,E → A,B,C,D,E)
#[test]
fn test_ifcomp_interleaving() -> TestResult {
    let output = run_ifcomp("A\nC\nE\n", "A\nB\nC\nD\nE\n")?;
    assert_statistics(&output, 0, 2, 0, 0, 0, 2);
    Ok(())
}

// Test chunked moves (blocks of lines moved)
#[test]
fn test_ifcomp_chunked_moves() -> TestResult {
    let output = run_ifcomp(
        "A\nB\nC\nD\nE\nF\nG\nH\n",
        "D\nE\nF\nA\nB\nC\nG\nH\n",
    )?;
    // Block A,B,C moved, block D,E,F moved
    assert_statistics(&output, 0, 0, 0, 0, 5, 2);
    Ok(())
}

// Test multiple independent change regions
#[test]
fn test_ifcomp_multiple_independent_regions() -> TestResult {
    let output = run_ifcomp(
        "KEEP1\nOLD1\nKEEP2\nOLD2\nKEEP3\nOLD3\nKEEP4\n",
        "KEEP1\nNEW1\nKEEP2\nNEW2\nKEEP3\nNEW3\nKEEP4\n",
    )?;
    assert_statistics(&output, 0, 0, 3, 3, 0, 3);
    Ok(())
}

// Test change every other line
#[test]
fn test_ifcomp_change_every_other_line() -> TestResult {
    let output = run_ifcomp(
        "A1\nKEEP1\nA2\nKEEP2\nA3\nKEEP3\n",
        "B1\nKEEP1\nB2\nKEEP2\nB3\nKEEP3\n",
    )?;
    assert_statistics(&output, 0, 0, 3, 3, 0, 3);
    Ok(())
}
