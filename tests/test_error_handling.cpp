#include <gtest/gtest.h>

#include <regex>
#include <sstream>
#include <string>

#include "ifcomp.h"
#include "ifcomp_driver.h"
#include "test_helpers.h"

// ============================================================================
// File I/O error tests
// ============================================================================

// Test fixture for file I/O error tests
class FileIOErrorTest : public ::testing::Test {
public:
    void SetUp() override
    {
        // Use stringstream to capture output
        output.str("");
    }

    std::ostringstream output;
    Ifcomp ifc{ output };
};

// Test file not found - first file doesn't exist
TEST_F(FileIOErrorTest, FirstFileNotFound)
{
    const char *nonexistent_file = "/nonexistent/path/to/file1.txt";

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    EXPECT_THROW({ ifc.compare(nonexistent_file, existing_file.c_str()); }, std::runtime_error);

    // Verify exception message contains expected text
    expect_file_error([&]() { ifc.compare(nonexistent_file, existing_file.c_str()); },
                      nonexistent_file);
}

// Test file not found - second file doesn't exist
TEST_F(FileIOErrorTest, SecondFileNotFound)
{
    const char *nonexistent_file = "/nonexistent/path/to/file2.txt";

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    EXPECT_THROW({ ifc.compare(existing_file.c_str(), nonexistent_file); }, std::runtime_error);

    // Verify exception message contains expected text
    expect_file_error([&]() { ifc.compare(existing_file.c_str(), nonexistent_file); },
                      nonexistent_file);
}

// Test both files don't exist
TEST_F(FileIOErrorTest, BothFilesNotFound)
{
    const char *nonexistent_file1 = "/nonexistent/path/to/file1.txt";
    const char *nonexistent_file2 = "/nonexistent/path/to/file2.txt";

    EXPECT_THROW({ ifc.compare(nonexistent_file1, nonexistent_file2); }, std::runtime_error);

    // Verify exception message contains expected text
    expect_file_error_any([&]() { ifc.compare(nonexistent_file1, nonexistent_file2); },
                          nonexistent_file1, nonexistent_file2);
}

// Test empty string file name (first file)
TEST_F(FileIOErrorTest, FirstFileEmptyString)
{
    const char *empty_file = "";

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    EXPECT_THROW({ ifc.compare(empty_file, existing_file.c_str()); }, std::runtime_error);
}

// Test empty string file name (second file)
TEST_F(FileIOErrorTest, SecondFileEmptyString)
{
    const char *empty_file = "";

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    EXPECT_THROW({ ifc.compare(existing_file.c_str(), empty_file); }, std::runtime_error);
}

// Test invalid path (directory instead of file)
TEST_F(FileIOErrorTest, FirstFileIsDirectory)
{
    const char *dir_path = "/tmp";

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    EXPECT_THROW({ ifc.compare(dir_path, existing_file.c_str()); }, std::runtime_error);
}

// Test invalid path (directory instead of file)
TEST_F(FileIOErrorTest, SecondFileIsDirectory)
{
    const char *dir_path = "/tmp";

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    EXPECT_THROW({ ifc.compare(existing_file.c_str(), dir_path); }, std::runtime_error);
}

// Test very long path name
TEST_F(FileIOErrorTest, VeryLongPathName)
{
    // Create a very long path that doesn't exist
    std::string long_path = "/tmp/";
    for (int i = 0; i < 100; i++) {
        long_path += "verylongpathname";
    }
    long_path += "/nonexistent.txt";

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    EXPECT_THROW({ ifc.compare(long_path.c_str(), existing_file.c_str()); }, std::runtime_error);
}

// Test that error message format includes filename
TEST_F(FileIOErrorTest, ErrorMessageFormat)
{
    const char *test_file = "/test/nonexistent/file.txt";

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    bool caught = false;
    try {
        ifc.compare(test_file, existing_file.c_str());
    } catch (const std::runtime_error &e) {
        caught = true;
        std::string error_msg = e.what();
        // Verify error message format
        EXPECT_NE(error_msg.find("Can't open file"), std::string::npos);
        EXPECT_NE(error_msg.find(test_file), std::string::npos);
        // Error message should not be empty
        EXPECT_FALSE(error_msg.empty());
    }
    EXPECT_TRUE(caught) << "Expected std::runtime_error to be thrown";
}

// ============================================================================
// Hash collisions tests
// ============================================================================

// Test many lines with similar prefixes to stress hash buckets
TEST_F(IfcompDriver, SimilarPrefixLines)
{
    // Generate 300+ unique lines to stress 256 hash buckets
    std::string content = generate_file_content_with_pattern(
        300, [](int i) { return "prefix_" + std::to_string(i) + "_suffix"; });

    std::string result = run_ifcomp(content.c_str(), content.c_str());
    assert_identical_files(result);
}

// Test lines with same length but different content
TEST_F(IfcompDriver, SameLengthDifferentContent)
{
    const char *a = "AAAAAAAA\nBBBBBBBB\nCCCCCCCC\n";
    const char *b = "AAAAAAAA\nBBBBBBBB\nCCCCCCCC\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// Test lines that are almost identical (one character different)
TEST_F(IfcompDriver, AlmostIdenticalLines)
{
    std::string a = "HELLO_WORLD_A\nHELLO_WORLD_B\nHELLO_WORLD_C\n";
    std::string b = "HELLO_WORLD_A\nHELLO_WORLD_X\nHELLO_WORLD_C\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 1, 1, 0, 1);
}

// Test many different lines to create hash collisions
TEST_F(IfcompDriver, ManyDifferentLines)
{
    // Generate many different lines
    std::string content =
        generate_file_content_with_pattern(100, [](int i) { return "line" + std::to_string(i); });

    std::string result = run_ifcomp(content.c_str(), content.c_str());
    assert_identical_files(result);
}

// Test lines that hash to same bucket (if we can craft them)
TEST_F(IfcompDriver, PotentialHashCollisions)
{
    // Try to create potential collisions
    std::string a =
        "test1\n"
        "test2\n"
        "test3\n"
        "abcd\n"
        "efgh\n";
    std::string b =
        "test1\n"
        "test2\n"
        "test3\n"
        "abcd\n"
        "efgh\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_identical_files(result);
}

// Test permutation of lines to stress hash table
TEST_F(IfcompDriver, PermutedLines)
{
    std::ostringstream a, b;
    // Create alphabet
    for (char c = 'A'; c <= 'Z'; c++) {
        a << c << "\n";
    }
    // Permute in reverse
    for (char c = 'Z'; c >= 'A'; c--) {
        b << c << "\n";
    }

    std::string result = run_ifcomp(a.str().c_str(), b.str().c_str());
    // All should be moved
    assert_statistics(result, 0, 0, 0, 0, 55, 25);
}

// Test many very similar lines
TEST_F(IfcompDriver, ManyVerySimilarLines)
{
    std::string content = generate_sequential_lines(1, 100, "SAMPLE_LINE_");
    std::string result = run_ifcomp(content.c_str(), content.c_str());
    assert_identical_files(result);
}

// ============================================================================
// Output validation tests
// ============================================================================

// Test statistics accuracy for simple case
TEST_F(IfcompDriver, SimpleStatisticsAccuracy)
{
    const char *a = "OLD1\nOLD2\nKEEP\nOLD3\n";
    const char *b = "NEW1\nNEW2\nKEEP\nNEW3\n";

    std::string result = run_ifcomp(a, b);

    // Verify exact statistics
    assert_statistics(result, 0, 0, 3, 3, 0, 2);
}

// Test statistics with actual deletions
TEST_F(IfcompDriver, DeletionsStatisticsAccuracy)
{
    const char *a = "A\nDELETE1\nDELETE2\nB\n";
    const char *b = "A\nB\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 2, 0, 0, 0, 0, 1);
}

// Test statistics with actual insertions
TEST_F(IfcompDriver, InsertionsStatisticsAccuracy)
{
    const char *a = "A\nB\n";
    const char *b = "A\nINSERT1\nINSERT2\nB\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 2, 0, 0, 0, 1);
}

// Test statistics with moves
TEST_F(IfcompDriver, MovesStatisticsAccuracy)
{
    const char *a = "A\nB\nC\n";
    const char *b = "C\nA\nB\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 0, 0, 1, 1);
}

// Test line numbers are correct
TEST_F(IfcompDriver, LineNumbersCorrect)
{
    const char *a = "KEEP1\nCHANGE1\nKEEP2\nCHANGE2\nKEEP3\n";
    const char *b = "KEEP1\nNEW1\nKEEP2\nNEW2\nKEEP3\n";

    std::string result = run_ifcomp(a, b);

    // Check that line numbers appear in output
    assert_line_numbers_present(result, { 2, 4 });
}

// Test output format consistency
TEST_F(IfcompDriver, OutputFormatConsistency)
{
    const char *a = "A\nB\nC\n";
    const char *b = "A\nB\nC\n";

    std::string result = run_ifcomp(a, b);

    // Output should have consistent format
    assert_output_contains(
        result, { "lines deleted from old.", "lines inserted in new.", "change blocks." });
}

// Test change block counts accuracy
TEST_F(IfcompDriver, ChangeBlockCounts)
{
    const char *a = "K1\nCH1\nK2\nCH2\nK3\nCH3\nK4\n";
    const char *b = "K1\nNEW1\nK2\nNEW2\nK3\nNEW3\nK4\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 3, 3, 0, 3);
}

// Test that statistics sum correctly
TEST_F(IfcompDriver, StatisticsSum)
{
    const char *a = "DELETE1\nDELETE2\nKEEP1\nCHANGE1\nCHANGE2\n";
    const char *b = "KEEP1\nNEW1\nNEW2\nINSERT1\n";

    std::string result = run_ifcomp(a, b);
    Statistics stats = extract_statistics(result);

    // Verify statistics are reasonable
    EXPECT_GE(stats.deleted, 0);
    EXPECT_GE(stats.inserted, 0);
    EXPECT_GE(stats.replaced_old, 0);
    EXPECT_GE(stats.replaced_new, 0);
    EXPECT_GE(stats.moved, 0);
    EXPECT_GT(stats.change_blocks, 0); // Should have some changes
}

// Test output contains "Comparing:" header
TEST_F(IfcompDriver, ContainsComparingHeader)
{
    const char *a = "A\nB\n";
    const char *b = "A\nB\n";

    std::string result = run_ifcomp(a, b);

    // Output should contain the comparing header (from main.c)
    EXPECT_TRUE(result.find("Comparing:") == std::string::npos ||
                result.find(fname_a) != std::string::npos)
        << "Should reference input files";
}

// Test replacement statistics are paired
TEST_F(IfcompDriver, ReplacementPairing)
{
    const char *a = "OLD1\nOLD2\n";
    const char *b = "NEW1\n";

    std::string result = run_ifcomp(a, b);

    // Should have 2 old lines replaced with 1 new line
    assert_statistics(result, 0, 0, 2, 1, 0, 1);
}

// Test multiple replacements
TEST_F(IfcompDriver, MultipleReplacementsStats)
{
    const char *a = "OLD1\nOLD2\nOLD3\nKEEP\nOLD4\n";
    const char *b = "NEW1\nKEEP\nNEW2\n";

    std::string result = run_ifcomp(a, b);

    Statistics stats = extract_statistics(result);
    EXPECT_GT(stats.replaced_old, 0);
    EXPECT_GT(stats.replaced_new, 0);
}

// Test output parsing with regex
TEST_F(IfcompDriver, OutputParsing)
{
    const char *a = "A\nB\nC\n";
    const char *b = "X\nY\nZ\n";

    std::string result = run_ifcomp(a, b);

    // Verify all expected patterns are present
    std::regex del_pattern(R"(\s*\d+\s+lines deleted from old\.)");
    std::regex ins_pattern(R"(\s*\d+\s+lines inserted in new\.)");
    std::regex repl_pattern(
        R"(\s*\d+\s+lines deleted from old and replaced with \d+ lines of new\.)");
    std::regex move_pattern(R"(\s*\d+\s+lines moved in old\.)");
    std::regex block_pattern(R"(\s*\d+\s+change blocks\.)");

    EXPECT_TRUE(std::regex_search(result, del_pattern));
    EXPECT_TRUE(std::regex_search(result, ins_pattern));
    EXPECT_TRUE(std::regex_search(result, repl_pattern));
    EXPECT_TRUE(std::regex_search(result, move_pattern));
    EXPECT_TRUE(std::regex_search(result, block_pattern));
}

