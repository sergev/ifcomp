#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test exactly 127 occurrences of same line (char limit)
TEST_F(IfcompDriver, Exactly127Occurrences)
{
    std::string a = generate_file_with_duplicates("LINE", 127);
    std::string b = generate_file_with_duplicates("LINE", 127);

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 0, 0, 0, 0);
}

// Test more than 127 occurrences (potential overflow)
// Note: file_nlines is char, so 128+ may cause issues
TEST_F(IfcompDriver, Over127Occurrences)
{
    std::string a = generate_file_with_duplicates("LINE", 128);
    std::string b = generate_file_with_duplicates("LINE", 128);

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 0, 0, 0, 0);
}

// Test file with only one unique line repeated 200 times
TEST_F(IfcompDriver, OneUniqueLine200Times)
{
    std::string a = generate_file_with_duplicates("SAME", 200);
    std::string b = generate_file_with_duplicates("SAME", 200);

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 0, 0, 0, 0);
}

// Test multiple lines each repeated many times
TEST_F(IfcompDriver, MultipleLinesManyRepeats)
{
    std::string a = generate_file_with_duplicates("A\n", 50) +
                    generate_file_with_duplicates("B\n", 50) +
                    generate_file_with_duplicates("C\n", 50);
    std::string b = generate_file_with_duplicates("A\n", 50) +
                    generate_file_with_duplicates("B\n", 50) +
                    generate_file_with_duplicates("C\n", 50);

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 0, 0, 0, 0);
}

// Test interleaved duplicates (A, B, A, B pattern)
TEST_F(IfcompDriver, InterleavedDuplicates)
{
    std::string a;
    for (int i = 0; i < 50; i++) {
        a += "A\nB\n";
    }
    std::string b;
    for (int i = 0; i < 50; i++) {
        b += "A\nB\n";
    }

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 0, 0, 0, 0);
}

// Test duplicated lines in different orders
TEST_F(IfcompDriver, DuplicatesDifferentOrder)
{
    std::string a = "A\nA\nB\nB\nC\nC\n";
    std::string b = "C\nC\nB\nB\nA\nA\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    // All lines should be moved
    assert_statistics(result, 0, 0, 0, 0, 6, 3);
}

// Test some duplicates, some unique
TEST_F(IfcompDriver, MixedDuplicatesAndUnique)
{
    std::string a = "A\nA\nUNIQUE1\nB\nB\n";
    std::string b = "A\nA\nUNIQUE2\nB\nB\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    // One replacement
    assert_statistics(result, 0, 0, 1, 1, 0, 1);
}

// Test removing duplicates
TEST_F(IfcompDriver, RemovingDuplicates)
{
    std::string a = "LINE\nLINE\nLINE\n";
    std::string b = "LINE\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 2, 0, 0, 0, 0, 1);
}

// Test adding duplicates
TEST_F(IfcompDriver, AddingDuplicates)
{
    std::string a = "LINE\n";
    std::string b = "LINE\nLINE\nLINE\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 2, 0, 0, 0, 1);
}
