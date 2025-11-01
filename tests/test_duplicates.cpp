#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test exactly 127 occurrences of same line (char limit)
// Note: Identical duplicate files are reported as replacements due to algorithm limitation
TEST_F(IfcompDriver, Exactly127Occurrences)
{
    std::string a = generate_file_with_duplicates("LINE", 127);
    std::string b = generate_file_with_duplicates("LINE", 127);

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 127, 127, 0, 1);
}

// Test more than 127 occurrences (potential overflow)
// Note: file_nlines is char, so 128+ may cause issues
// Note: Identical duplicate files are reported as replacements due to algorithm limitation
TEST_F(IfcompDriver, Over127Occurrences)
{
    std::string a = generate_file_with_duplicates("LINE", 128);
    std::string b = generate_file_with_duplicates("LINE", 128);

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 128, 128, 0, 1);
}

// Test file with only one unique line repeated 200 times
// Note: Identical duplicate files are reported as replacements due to algorithm limitation
TEST_F(IfcompDriver, OneUniqueLine200Times)
{
    std::string a = generate_file_with_duplicates("SAME", 200);
    std::string b = generate_file_with_duplicates("SAME", 200);

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 200, 200, 0, 1);
}

// Test multiple lines each repeated many times
// Note: Identical duplicate files are reported as replacements due to algorithm limitation
TEST_F(IfcompDriver, MultipleLinesManyRepeats)
{
    std::string a = generate_file_with_duplicates("A\n", 50) +
                    generate_file_with_duplicates("B\n", 50) +
                    generate_file_with_duplicates("C\n", 50);
    std::string b = generate_file_with_duplicates("A\n", 50) +
                    generate_file_with_duplicates("B\n", 50) +
                    generate_file_with_duplicates("C\n", 50);

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 300, 300, 0, 1);
}

// Test interleaved duplicates (A, B, A, B pattern)
// Note: Identical duplicate files are reported as replacements due to algorithm limitation
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
    assert_statistics(result, 0, 0, 100, 100, 0, 1);
}

// Test duplicated lines in different orders
// Note: Duplicate lines without unique anchors are reported as replacements
TEST_F(IfcompDriver, DuplicatesDifferentOrder)
{
    std::string a = "A\nA\nB\nB\nC\nC\n";
    std::string b = "C\nC\nB\nB\nA\nA\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    // Reported as replacement due to algorithm limitation
    assert_statistics(result, 0, 0, 6, 6, 0, 1);
}

// Test some duplicates, some unique
// Note: Duplicate lines prevent proper matching even with unique lines
TEST_F(IfcompDriver, MixedDuplicatesAndUnique)
{
    std::string a = "A\nA\nUNIQUE1\nB\nB\n";
    std::string b = "A\nA\nUNIQUE2\nB\nB\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    // Reported as 5 replacements due to algorithm limitation
    assert_statistics(result, 0, 0, 5, 5, 0, 1);
}

// Test removing duplicates
// Note: Duplicate lines are reported as replacement rather than deletion
TEST_F(IfcompDriver, RemovingDuplicates)
{
    std::string a = "LINE\nLINE\nLINE\n";
    std::string b = "LINE\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 3, 1, 0, 1);
}

// Test adding duplicates
// Note: Duplicate lines are reported as replacement rather than insertion
TEST_F(IfcompDriver, AddingDuplicates)
{
    std::string a = "LINE\n";
    std::string b = "LINE\nLINE\nLINE\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 1, 3, 0, 1);
}
