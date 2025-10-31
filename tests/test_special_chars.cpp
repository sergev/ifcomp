#include <gtest/gtest.h>

#include <sstream>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test lines with tabs
TEST(IfcompSpecialChars, LinesWithTabs)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "\tLINE1\nLINE2\t\n";
    const char *b = "\tLINE1\nLINE2\t\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test lines with multiple spaces
TEST(IfcompSpecialChars, LinesWithMultipleSpaces)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "LINE    WITH    SPACES\n";
    const char *b = "LINE    WITH    SPACES\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test ASCII control characters
TEST(IfcompSpecialChars, ASCIIControlChars)
{
    IfcompDriver driver;
    driver.SetUp();

    std::ostringstream a, b;
    // Use safe control chars that won't break string
    a << "LINE" << (char)1 << "CONTROL\n";
    b << "LINE" << (char)1 << "CONTROL\n";

    driver.create_file(driver.fname_a, a.str().c_str());
    driver.create_file(driver.fname_b, b.str().c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test mix of special characters
TEST(IfcompSpecialChars, MixedSpecialChars)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a =
        "!@#$%^&*()\n"
        "[]{}|\\:'\"<>?\n"
        "`~-_=+\n";
    const char *b =
        "!@#$%^&*()\n"
        "[]{}|\\:'\"<>?\n"
        "`~-_=+\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test UTF-8 multibyte characters
TEST(IfcompSpecialChars, UTF8MultibyteChars)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a =
        "Hello 世界\n"
        "こんにちは\n"
        "Привет\n";
    const char *b =
        "Hello 世界\n"
        "こんにちは\n"
        "Привет\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test UTF-8 with differences
TEST(IfcompSpecialChars, UTF8WithDifferences)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "Hello 世界\n";
    const char *b = "Hello 宇宙\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 1, 1, 0, 1);

    driver.TearDown();
}

// Test backslash handling
TEST(IfcompSpecialChars, BackslashHandling)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a =
        "path\\to\\file\n"
        "C:\\Windows\\System\n";
    const char *b =
        "path\\to\\file\n"
        "C:\\Windows\\System\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test quotes and apostrophes
TEST(IfcompSpecialChars, QuotesAndApostrophes)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a =
        "\"quoted text\"\n"
        "'single quotes'\n"
        "it's a test\n";
    const char *b =
        "\"quoted text\"\n"
        "'single quotes'\n"
        "it's a test\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test numeric strings
TEST(IfcompSpecialChars, NumericStrings)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a =
        "12345\n"
        "0xDEADBEEF\n"
        "3.14159\n";
    const char *b =
        "12345\n"
        "0xDEADBEEF\n"
        "3.14159\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test lines with carriage returns (CRLF vs LF)
// Note: IFCOMP strips newlines, so this should work
TEST(IfcompSpecialChars, CarriageReturns)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "LINE\r\nLINE\r\n";
    const char *b = "LINE\nLINE\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // Should match if trailing newlines are stripped
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}
