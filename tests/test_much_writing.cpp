#include <gtest/gtest.h>

#include "ifcomp_driver.h"

// Test case from the article
TEST(IfcompTest, MuchWritingExample)
{
    IfcompDriver driver;
    driver.SetUp();

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

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    EXPECT_EQ(result, std::string(expect));

    driver.TearDown();
}
