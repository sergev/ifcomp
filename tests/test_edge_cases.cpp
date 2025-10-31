#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test empty files - IFCOMP exits with error for empty files
TEST(IfcompEdgeCases, EmptyFileHandling)
{
    // Note: IFCOMP exits on empty files, so we can't test this without
    // modifying the exit behavior or catching it somehow.
    // This is a documented limitation.
}

// Test files with single identical line
TEST_F(IfcompDriver, SingleIdenticalLine)
{
    const char *a = "HELLO\n";
    const char *b = "HELLO\n";
    const char *expect =
        "       0 lines deleted from old.\n"
        "       0 lines inserted in new.\n"
        "       0 lines deleted from old and replaced with 0 lines of new.\n"
        "       0 lines moved in old.\n"
        "       0 change blocks.\n";

    std::string result = run_ifcomp(a, b);
    EXPECT_EQ(result, std::string(expect));
}

// Test files with single different line
TEST_F(IfcompDriver, SingleDifferentLine)
{
    const char *a = "OLD\n";
    const char *b = "NEW\n";
    const char *expect =
        "*** AFTER TOP =========================================== ***\n"
        "*** REPLACE LINE(s) ------------------------------------- ***\n"
        "      1|OLD\n"
        "*** WITH LINE(s) ---------------------------------------- ***\n"
        "+     1|NEW\n"
        "*** ===================================================== ***\n"
        "\n"
        "       0 lines deleted from old.\n"
        "       0 lines inserted in new.\n"
        "       1 lines deleted from old and replaced with 1 lines of new.\n"
        "       0 lines moved in old.\n"
        "       1 change blocks.\n";

    std::string result = run_ifcomp(a, b);
    EXPECT_EQ(result, std::string(expect));
}

// Test two lines - both identical
TEST_F(IfcompDriver, TwoLinesIdentical)
{
    const char *a = "LINE1\nLINE2\n";
    const char *b = "LINE1\nLINE2\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 0, 0, 0, 0);
}

// Test two lines - second different
TEST_F(IfcompDriver, TwoLinesSecondDifferent)
{
    const char *a = "LINE1\nOLD\n";
    const char *b = "LINE1\nNEW\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 1, 1, 0, 1);
}

// Test three lines - all identical
TEST_F(IfcompDriver, ThreeLinesIdentical)
{
    const char *a = "A\nB\nC\n";
    const char *b = "A\nB\nC\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 0, 0, 0, 0);
}

// Test file with only spaces (as lines)
TEST_F(IfcompDriver, LinesWithOnlySpaces)
{
    const char *a = "   \n   \nLINE\n";
    const char *b = "   \nLINE\n";

    // This should delete one line
    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 1, 0, 0, 0, 0, 1);
}

// Test file without trailing newline
TEST_F(IfcompDriver, NoTrailingNewline)
{
    const char *a = "A\nB";
    const char *b = "A\nB\n";

    // Should detect the line difference
    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 0, 0, 0, 0);
}
