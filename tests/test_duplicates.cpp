#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test exactly 127 occurrences of same line (char limit)
TEST(IfcompDuplicates, Exactly127Occurrences)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string a = generate_file_with_duplicates("LINE", 127);
    std::string b = generate_file_with_duplicates("LINE", 127);

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test more than 127 occurrences (potential overflow)
// Note: file_nlines is char, so 128+ may cause issues
TEST(IfcompDuplicates, Over127Occurrences)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string a = generate_file_with_duplicates("LINE", 128);
    std::string b = generate_file_with_duplicates("LINE", 128);

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test file with only one unique line repeated 200 times
TEST(IfcompDuplicates, OneUniqueLine200Times)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string a = generate_file_with_duplicates("SAME", 200);
    std::string b = generate_file_with_duplicates("SAME", 200);

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test multiple lines each repeated many times
TEST(IfcompDuplicates, MultipleLinesManyRepeats)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string a = generate_file_with_duplicates("A\n", 50) +
                    generate_file_with_duplicates("B\n", 50) +
                    generate_file_with_duplicates("C\n", 50);
    std::string b = generate_file_with_duplicates("A\n", 50) +
                    generate_file_with_duplicates("B\n", 50) +
                    generate_file_with_duplicates("C\n", 50);

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test interleaved duplicates (A, B, A, B pattern)
TEST(IfcompDuplicates, InterleavedDuplicates)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string a;
    for (int i = 0; i < 50; i++) {
        a += "A\nB\n";
    }
    std::string b;
    for (int i = 0; i < 50; i++) {
        b += "A\nB\n";
    }

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test duplicated lines in different orders
TEST(IfcompDuplicates, DuplicatesDifferentOrder)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string a = "A\nA\nB\nB\nC\nC\n";
    std::string b = "C\nC\nB\nB\nA\nA\n";

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // All lines should be moved
    assert_statistics(result, 0, 0, 0, 0, 6, 3);

    driver.TearDown();
}

// Test some duplicates, some unique
TEST(IfcompDuplicates, MixedDuplicatesAndUnique)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string a = "A\nA\nUNIQUE1\nB\nB\n";
    std::string b = "A\nA\nUNIQUE2\nB\nB\n";

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // One replacement
    assert_statistics(result, 0, 0, 1, 1, 0, 1);

    driver.TearDown();
}

// Test removing duplicates
TEST(IfcompDuplicates, RemovingDuplicates)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string a = "LINE\nLINE\nLINE\n";
    std::string b = "LINE\n";

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 2, 0, 0, 0, 0, 1);

    driver.TearDown();
}

// Test adding duplicates
TEST(IfcompDuplicates, AddingDuplicates)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string a = "LINE\n";
    std::string b = "LINE\nLINE\nLINE\n";

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 2, 0, 0, 0, 1);

    driver.TearDown();
}
