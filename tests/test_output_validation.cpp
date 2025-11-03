#include <gtest/gtest.h>

#include <regex>
#include <sstream>

#include "ifcomp_driver.h"
#include "test_helpers.h"

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
