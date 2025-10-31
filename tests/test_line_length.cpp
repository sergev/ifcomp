#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test line exactly 4095 bytes (one less than buffer)
TEST(IfcompLineLength, Line4095Bytes)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string long_line = generate_long_line(4095);
    std::string a = long_line + "\n";
    std::string b = long_line + "\n";

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test line exactly 4096 bytes (buffer size)
TEST(IfcompLineLength, Line4096Bytes)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string long_line = generate_long_line(4096);
    std::string a = long_line + "\n";
    std::string b = long_line + "\n";

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test line over 4096 bytes (truncation behavior)
// IFCOMP buffer is 4096, so lines longer will be truncated
TEST(IfcompLineLength, LineOver4096Bytes)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string long_line = generate_long_line(5000);
    std::string a = long_line.substr(0, 4096) + "\n";
    std::string b = long_line.substr(0, 4096) + "\n";

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    // Should match because buffer truncates to same length
    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test mix of very short and very long lines
TEST(IfcompLineLength, MixShortAndLongLines)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string short_line = "A\n";
    std::string long_line = generate_long_line(4000) + "\n";

    std::string a = short_line + long_line + short_line;
    std::string b = short_line + long_line + short_line;

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test very long line followed by very short line
TEST(IfcompLineLength, LongLineFollowedByShort)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string long_line = generate_long_line(4095);
    std::string a = long_line + "\nX\n";
    std::string b = long_line + "\nY\n";

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 1, 1, 0, 1);

    driver.TearDown();
}

// Test multiple very long lines
TEST(IfcompLineLength, MultipleLongLines)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string line1 = generate_long_line(3000) + "\n";
    std::string line2 = generate_long_line(3500) + "\n";

    std::string a = line1 + line2;
    std::string b = line1 + line2;

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test lines of maximum safe size
TEST(IfcompLineLength, MaxSafeLineSize)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string line = generate_long_line(4094);
    std::string a = line + "\n";
    std::string b = line + "\n";

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}
