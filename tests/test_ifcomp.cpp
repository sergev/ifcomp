#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
extern "C" {
#include "../ifcomp.h"
}

class IfcompTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Create unique temporary file names
        char template_a[] = "ifcomp_test_a_XXXXXX";
        char template_b[] = "ifcomp_test_b_XXXXXX";
        char template_out[] = "ifcomp_test_out_XXXXXX";

        fd_a = mkstemp(template_a);
        fd_b = mkstemp(template_b);
        fd_out = mkstemp(template_out);

        ASSERT_GE(fd_a, 0);
        ASSERT_GE(fd_b, 0);
        ASSERT_GE(fd_out, 0);

        strcpy(fname_a, template_a);
        strcpy(fname_b, template_b);
        strcpy(fname_out, template_out);

        // Redirect stdout to output file
        original_stdout = dup(STDOUT_FILENO);
        dup2(fd_out, STDOUT_FILENO);
    }

    void TearDown() override
    {
        // Restore stdout
        if (original_stdout >= 0) {
            dup2(original_stdout, STDOUT_FILENO);
            close(original_stdout);
        }

        // Close file descriptors
        if (fd_a >= 0)
            close(fd_a);
        if (fd_b >= 0)
            close(fd_b);
        if (fd_out >= 0)
            close(fd_out);

        // Remove temporary files
        unlink(fname_a);
        unlink(fname_b);
        unlink(fname_out);
    }

    void create_file(const char *fname, const char *content)
    {
        FILE *f = fopen(fname, "w");
        ASSERT_NE(f, nullptr);
        fputs(content, f);
        fclose(f);
    }

    std::string get_output()
    {
        fflush(stdout);

        // Close the redirected stdout and restore original
        close(fd_out);
        dup2(original_stdout, STDOUT_FILENO);

        // Read the output file
        std::ifstream file(fname_out);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    char fname_a[256];
    char fname_b[256];
    char fname_out[256];
    int fd_a = -1;
    int fd_b = -1;
    int fd_out = -1;
    int original_stdout = -1;
};

// Test case with identical input files
TEST_F(IfcompTest, IdenticalFiles)
{
    const char *a = "A\nB\n";
    const char *b = "A\nB\n";
    const char *expect =
        "       0 lines deleted from old.\n"
        "       0 lines inserted in new.\n"
        "       0 lines deleted from old and replaced with 0 lines of new.\n"
        "       0 lines moved in old.\n"
        "       0 change blocks.\n";

    create_file(fname_a, a);
    create_file(fname_b, b);
    ifcomp(fname_a, fname_b);

    std::string result = get_output();
    EXPECT_EQ(result, std::string(expect));
}

// Test case with deletes, moves and replacements
TEST_F(IfcompTest, ComplexChanges)
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

    create_file(fname_a, a);
    create_file(fname_b, b);
    ifcomp(fname_a, fname_b);

    std::string result = get_output();
    EXPECT_EQ(result, std::string(expect));
}

// Test case with deletes, moves and replacements
TEST_F(IfcompTest, PermutationChanges)
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

    create_file(fname_a, a);
    create_file(fname_b, b);
    ifcomp(fname_a, fname_b);

    std::string result = get_output();
    EXPECT_EQ(result, std::string(expect));
}

// Test case from the article
TEST_F(IfcompTest, MuchWritingExample)
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

    create_file(fname_a, a);
    create_file(fname_b, b);
    ifcomp(fname_a, fname_b);

    std::string result = get_output();
    EXPECT_EQ(result, std::string(expect));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
