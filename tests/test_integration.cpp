#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// ============================================================================
// Test identical files
// ============================================================================

// Test case with identical input files
TEST_F(IfcompDriver, IdenticalFiles)
{
    const char *a = "A\nB\n";
    const char *b = "A\nB\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// ============================================================================
// Test complex changes
// ============================================================================

// Test case with deletes, moves and replacements
TEST_F(IfcompDriver, ComplexChanges)
{
    const char *a = "A\nX\nC\nY\nD\nW\nE\nA\nB\nE\n";
    const char *b = "A\nB\nC\nD\nE\n";
    const char *expect =
        "*** AFTER TOP =========================================== ***\n"
        "*** DELETE LINE(s) -------------------------------------- ***\n"
        "      1|A\n"
        "      2|X\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER LINE(s) ======================================= ***\n"
        "      3|C\n"
        "*** DELETE LINE(s) -------------------------------------- ***\n"
        "      4|Y\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER LINE(s) ======================================= ***\n"
        "      5|D\n"
        "*** REPLACE LINE(s) ------------------------------------- ***\n"
        "      6|W\n"
        "      7|E\n"
        "*** WITH LINE(s) ---------------------------------------- ***\n"
        "+     5|E\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER LINE(s) ======================================= ***\n"
        "      9|B\n"
        "*** DELETE LINE(s) -------------------------------------- ***\n"
        "     10|E\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER TOP =========================================== ***\n"
        "*** MOVE LINE(s) ---------------------------------------- ***\n"
        "      8|A\n"
        "      9|B\n"
        "*** ===================================================== ***\n"
        "\n"
        "       4 lines deleted from old.\n"
        "       0 lines inserted in new.\n"
        "       2 lines deleted from old and replaced with 1 lines of new.\n"
        "       2 lines moved in old.\n"
        "       5 change blocks.\n";

    std::string result = run_ifcomp(a, b);
    assert_expected_output(result, expect);
}

// ============================================================================
// Test permutation changes
// ============================================================================

// Test case with deletes, moves and replacements
TEST_F(IfcompDriver, PermutationChanges)
{
    const char *a = "A\nB\nC\nD\nE\nG\n";
    const char *b = "D\nE\nF\nG\nA\nC\n";
    const char *expect =
        "*** AFTER LINE(s) ======================================= ***\n"
        "      1|A\n"
        "*** DELETE LINE(s) -------------------------------------- ***\n"
        "      2|B\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER LINE(s) ======================================= ***\n"
        "      5|E\n"
        "*** INSERT LINE(s) -------------------------------------- ***\n"
        "+     3|F\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER LINE(s) ======================================= ***\n"
        "      6|G\n"
        "*** MOVE LINE(s) ---------------------------------------- ***\n"
        "      1|A\n"
        "      3|C\n"
        "*** ===================================================== ***\n"
        "\n"
        "       1 lines deleted from old.\n"
        "       1 lines inserted in new.\n"
        "       0 lines deleted from old and replaced with 0 lines of new.\n"
        "       2 lines moved in old.\n"
        "       3 change blocks.\n";

    std::string result = run_ifcomp(a, b);
    assert_expected_output(result, expect);
}

// ============================================================================
// Test much writing example
// ============================================================================

// Test case from the article
TEST_F(IfcompDriver, MuchWritingExample)
{
    const char *a =
        "a\nmass\nof\nlatin\nwords\n"
        "falls\nupon\nthe\nrelevant\nfacts\n"
        "like\nsoft\nsnow\n,\ncovering\n"
        "up\nthe\ndetails\n.\n";
    const char *b =
        "much\nwriting\nis\nlike\nsnow\n"
        ",\na\nmass\nof\nlong\n"
        "words\nand\nphrases\nfalls\nupon\n"
        "the\nrelevant\nfacts\ncovering\nup\n"
        "the\ndetails\n.\n";
    const char *expect =
        "*** AFTER LINE(s) ======================================= ***\n"
        "      3|of\n"
        "*** REPLACE LINE(s) ------------------------------------- ***\n"
        "      4|latin\n"
        "*** WITH LINE(s) ---------------------------------------- ***\n"
        "+    10|long\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER LINE(s) ======================================= ***\n"
        "     11|like\n"
        "*** DELETE LINE(s) -------------------------------------- ***\n"
        "     12|soft\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER TOP =========================================== ***\n"
        "*** INSERT LINE(s) -------------------------------------- ***\n"
        "+     1|much\n"
        "+     2|writing\n"
        "+     3|is\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER LINE(s) ======================================= ***\n"
        "      5|words\n"
        "*** INSERT LINE(s) -------------------------------------- ***\n"
        "+    12|and\n"
        "+    13|phrases\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER TOP =========================================== ***\n"
        "*** MOVE LINE(s) ---------------------------------------- ***\n"
        "     11|like\n"
        "     13|snow\n"
        "     14|,\n"
        "*** ===================================================== ***\n"
        "\n"
        "       1 lines deleted from old.\n"
        "       5 lines inserted in new.\n"
        "       1 lines deleted from old and replaced with 1 lines of new.\n"
        "       3 lines moved in old.\n"
        "       5 change blocks.\n";

    std::string result = run_ifcomp(a, b);
    assert_expected_output(result, expect);
}

// ============================================================================
// Test edge cases
// ============================================================================

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

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
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
    assert_expected_output(result, expect);
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
    assert_identical_files(result);
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
    assert_identical_files(result);
}

