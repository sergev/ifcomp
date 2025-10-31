#include <gtest/gtest.h>

#include <regex>
#include <sstream>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test statistics accuracy for simple case
TEST(IfcompOutputValidation, SimpleStatisticsAccuracy)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "OLD1\nOLD2\nKEEP\nOLD3\n";
    const char *b = "NEW1\nNEW2\nKEEP\nNEW3\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();

    // Verify exact statistics
    assert_statistics(result, 0, 0, 3, 3, 0, 3);

    driver.TearDown();
}

// Test statistics with actual deletions
TEST(IfcompOutputValidation, DeletionsStatisticsAccuracy)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nDELETE1\nDELETE2\nB\n";
    const char *b = "A\nB\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 2, 0, 0, 0, 0, 2);

    driver.TearDown();
}

// Test statistics with actual insertions
TEST(IfcompOutputValidation, InsertionsStatisticsAccuracy)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\n";
    const char *b = "A\nINSERT1\nINSERT2\nB\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 2, 0, 0, 0, 2);

    driver.TearDown();
}

// Test statistics with moves
TEST(IfcompOutputValidation, MovesStatisticsAccuracy)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\n";
    const char *b = "C\nA\nB\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 3, 1);

    driver.TearDown();
}

// Test line numbers are correct
TEST(IfcompOutputValidation, LineNumbersCorrect)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "KEEP1\nCHANGE1\nKEEP2\nCHANGE2\nKEEP3\n";
    const char *b = "KEEP1\nNEW1\nKEEP2\nNEW2\nKEEP3\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();

    // Check that line numbers appear in output
    EXPECT_TRUE(result.find("2|") != std::string::npos) << "Should show line 2";
    EXPECT_TRUE(result.find("4|") != std::string::npos) << "Should show line 4";

    driver.TearDown();
}

// Test output format consistency
TEST(IfcompOutputValidation, OutputFormatConsistency)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\n";
    const char *b = "A\nB\nC\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();

    // Output should have consistent format
    EXPECT_TRUE(result.find("lines deleted from old.") != std::string::npos);
    EXPECT_TRUE(result.find("lines inserted in new.") != std::string::npos);
    EXPECT_TRUE(result.find("change blocks.") != std::string::npos);

    driver.TearDown();
}

// Test change block counts accuracy
TEST(IfcompOutputValidation, ChangeBlockCounts)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "K1\nCH1\nK2\nCH2\nK3\nCH3\nK4\n";
    const char *b = "K1\nNEW1\nK2\nNEW2\nK3\nNEW3\nK4\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 3, 3, 0, 3);

    driver.TearDown();
}

// Test that statistics sum correctly
TEST(IfcompOutputValidation, StatisticsSum)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "DELETE1\nDELETE2\nKEEP1\nCHANGE1\nCHANGE2\n";
    const char *b = "KEEP1\nNEW1\nNEW2\nINSERT1\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    Statistics stats = extract_statistics(result);

    // Verify statistics are reasonable
    EXPECT_GE(stats.deleted, 0);
    EXPECT_GE(stats.inserted, 0);
    EXPECT_GE(stats.replaced_old, 0);
    EXPECT_GE(stats.replaced_new, 0);
    EXPECT_GE(stats.moved, 0);
    EXPECT_GT(stats.change_blocks, 0); // Should have some changes

    driver.TearDown();
}

// Test identical files produce correct stats
TEST(IfcompOutputValidation, IdenticalFilesStats)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\nD\n";
    const char *b = "A\nB\nC\nD\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test output contains "Comparing:" header
TEST(IfcompOutputValidation, ContainsComparingHeader)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\n";
    const char *b = "A\nB\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();

    // Output should contain the comparing header (from main.c)
    EXPECT_TRUE(result.find("Comparing:") == std::string::npos ||
                result.find(driver.fname_a) != std::string::npos)
        << "Should reference input files";

    driver.TearDown();
}

// Test replacement statistics are paired
TEST(IfcompOutputValidation, ReplacementPairing)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "OLD1\nOLD2\n";
    const char *b = "NEW1\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();

    // Should have 2 old lines replaced with 1 new line
    assert_statistics(result, 0, 0, 2, 1, 0, 1);

    driver.TearDown();
}

// Test multiple replacements
TEST(IfcompOutputValidation, MultipleReplacementsStats)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "OLD1\nOLD2\nOLD3\nKEEP\nOLD4\n";
    const char *b = "NEW1\nKEEP\nNEW2\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();

    Statistics stats = extract_statistics(result);
    EXPECT_GT(stats.replaced_old, 0);
    EXPECT_GT(stats.replaced_new, 0);

    driver.TearDown();
}

// Test output parsing with regex
TEST(IfcompOutputValidation, OutputParsing)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\n";
    const char *b = "X\nY\nZ\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();

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

    driver.TearDown();
}
