#include <gtest/gtest.h>

#include <sstream>

#include "ifcomp_driver.h"
#include "test_helpers.h"

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
