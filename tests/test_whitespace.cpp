#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test leading whitespace
TEST_F(IfcompDriver, LeadingWhitespace)
{
    const char *a = "   LINE\n   MORE\n";
    const char *b = "   LINE\n   MORE\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// Test trailing whitespace
TEST_F(IfcompDriver, TrailingWhitespace)
{
    const char *a = "LINE   \nMORE   \n";
    const char *b = "LINE   \nMORE   \n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// Test leading whitespace differences
TEST_F(IfcompDriver, LeadingWhitespaceDifferent)
{
    const char *a = "   LINE\n";
    const char *b = "LINE\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 1, 1, 0, 1);
}

// Test trailing whitespace differences
TEST_F(IfcompDriver, TrailingWhitespaceDifferent)
{
    const char *a = "LINE   \n";
    const char *b = "LINE\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 1, 1, 0, 1);
}

// Test lines with only spaces/tabs
TEST_F(IfcompDriver, OnlyWhitespaceLines)
{
    const char *a = "   \n\t\t\t\nLINE\n";
    const char *b = "   \n\t\t\t\nLINE\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// Test mix of spaces and tabs
TEST_F(IfcompDriver, MixedSpacesAndTabs)
{
    const char *a =
        "\t   \tLINE\n"
        "   \t\tMORE\n";
    const char *b =
        "\t   \tLINE\n"
        "   \t\tMORE\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// Test empty lines (just newline)
TEST_F(IfcompDriver, EmptyLines)
{
    const char *a = "A\n\nB\n\nC\n";
    const char *b = "A\n\nB\n\nC\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// Test empty lines in different positions
TEST_F(IfcompDriver, EmptyLinesDifferent)
{
    const char *a = "A\nB\nC\n";
    const char *b = "A\n\nB\n\nC\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 2, 0, 0, 0, 2);
}

// Test indentation changes
TEST_F(IfcompDriver, IndentationChanges)
{
    const char *a =
        "  if (x) {\n"
        "    return;\n"
        "  }\n";
    const char *b =
        "    if (x) {\n"
        "      return;\n"
        "    }\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 3, 3, 0, 1);
}

// Test whitespace-only line differences
TEST_F(IfcompDriver, WhitespaceOnlyDifferences)
{
    const char *a = "LINE\n   \nMORE\n";
    const char *b = "LINE\n     \nMORE\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 1, 1, 0, 1);
}

// Test tabs vs spaces
TEST_F(IfcompDriver, TabsVsSpaces)
{
    const char *a = "\tLINE\n";
    const char *b = "    LINE\n";

    std::string result = run_ifcomp(a, b);
    // Should be different
    assert_statistics(result, 0, 0, 1, 1, 0, 1);
}

// Test multiple empty lines
TEST_F(IfcompDriver, MultipleEmptyLines)
{
    const char *a = "START\n\n\n\nEND\n";
    const char *b = "START\n\n\n\nEND\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}
