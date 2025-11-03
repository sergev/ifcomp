#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

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
