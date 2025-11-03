#include <gtest/gtest.h>

#include <sstream>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// ============================================================================
// Whitespace tests
// ============================================================================

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

// ============================================================================
// Special characters tests
// ============================================================================

// Test lines with tabs
TEST_F(IfcompDriver, LinesWithTabs)
{
    const char *a = "\tLINE1\nLINE2\t\n";
    const char *b = "\tLINE1\nLINE2\t\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// Test lines with multiple spaces
TEST_F(IfcompDriver, LinesWithMultipleSpaces)
{
    const char *a = "LINE    WITH    SPACES\n";
    const char *b = "LINE    WITH    SPACES\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// Test ASCII control characters
TEST_F(IfcompDriver, ASCIIControlChars)
{
    std::ostringstream a, b;
    // Use safe control chars that won't break string
    a << "LINE" << (char)1 << "CONTROL\n";
    b << "LINE" << (char)1 << "CONTROL\n";

    std::string result = run_ifcomp(a.str().c_str(), b.str().c_str());
    assert_identical_files(result);
}

// Test mix of special characters
TEST_F(IfcompDriver, MixedSpecialChars)
{
    const char *a =
        "!@#$%^&*()\n"
        "[]{}|\\:'\"<>?\n"
        "`~-_=+\n";
    const char *b =
        "!@#$%^&*()\n"
        "[]{}|\\:'\"<>?\n"
        "`~-_=+\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// Test UTF-8 multibyte characters
TEST_F(IfcompDriver, UTF8MultibyteChars)
{
    const char *a =
        "Hello 世界\n"
        "こんにちは\n"
        "Привет\n";
    const char *b =
        "Hello 世界\n"
        "こんにちは\n"
        "Привет\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// Test UTF-8 with differences
TEST_F(IfcompDriver, UTF8WithDifferences)
{
    const char *a = "Hello 世界\n";
    const char *b = "Hello 宇宙\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 1, 1, 0, 1);
}

// Test backslash handling
TEST_F(IfcompDriver, BackslashHandling)
{
    const char *a =
        "path\\to\\file\n"
        "C:\\Windows\\System\n";
    const char *b =
        "path\\to\\file\n"
        "C:\\Windows\\System\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// Test quotes and apostrophes
TEST_F(IfcompDriver, QuotesAndApostrophes)
{
    const char *a =
        "\"quoted text\"\n"
        "'single quotes'\n"
        "it's a test\n";
    const char *b =
        "\"quoted text\"\n"
        "'single quotes'\n"
        "it's a test\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// Test numeric strings
TEST_F(IfcompDriver, NumericStrings)
{
    const char *a =
        "12345\n"
        "0xDEADBEEF\n"
        "3.14159\n";
    const char *b =
        "12345\n"
        "0xDEADBEEF\n"
        "3.14159\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// Test lines with carriage returns (CRLF vs LF)
// Note: IFCOMP treats CRLF differently from LF - reported as replacement
TEST_F(IfcompDriver, CarriageReturns)
{
    const char *a = "LINE\r\nLINE\r\n";
    const char *b = "LINE\nLINE\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 2, 2, 0, 1);
}

// ============================================================================
// Line length tests
// ============================================================================

// Test line exactly 4095 bytes (one less than buffer)
TEST_F(IfcompDriver, Line4095Bytes)
{
    std::string long_line = generate_long_line(4095);
    std::string a = long_line + "\n";
    std::string b = long_line + "\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_identical_files(result);
}

// Test line exactly 4096 bytes (buffer size)
TEST_F(IfcompDriver, Line4096Bytes)
{
    std::string long_line = generate_long_line(4096);
    std::string a = long_line + "\n";
    std::string b = long_line + "\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_identical_files(result);
}

// Test line over 4096 bytes (truncation behavior)
// IFCOMP buffer is 4096, so lines longer will be truncated
TEST_F(IfcompDriver, LineOver4096Bytes)
{
    std::string long_line = generate_long_line(5000);
    std::string a = long_line.substr(0, 4096) + "\n";
    std::string b = long_line.substr(0, 4096) + "\n";

    // Should match because buffer truncates to same length
    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_identical_files(result);
}

// Test mix of very short and very long lines
TEST_F(IfcompDriver, MixShortAndLongLines)
{
    std::string short_line = "A\n";
    std::string long_line = generate_long_line(4000) + "\n";

    std::string a = short_line + long_line + short_line;
    std::string b = short_line + long_line + short_line;

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_identical_files(result);
}

// Test very long line followed by very short line
TEST_F(IfcompDriver, LongLineFollowedByShort)
{
    std::string long_line = generate_long_line(4095);
    std::string a = long_line + "\nX\n";
    std::string b = long_line + "\nY\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 1, 1, 0, 1);
}

// Test multiple very long lines
TEST_F(IfcompDriver, MultipleLongLines)
{
    std::string line1 = generate_long_line(3000) + "\n";
    std::string line2 = generate_long_line(3500) + "\n";

    std::string a = line1 + line2;
    std::string b = line1 + line2;

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_identical_files(result);
}

// Test lines of maximum safe size
TEST_F(IfcompDriver, MaxSafeLineSize)
{
    std::string line = generate_long_line(4094);
    std::string a = line + "\n";
    std::string b = line + "\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_identical_files(result);
}

