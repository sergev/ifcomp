#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include "ifcomp.h"

static const char fname_a[] = "a.input";
static const char fname_b[] = "b.input";
static const char fname_out[] = "result.output";

//
// Create file with given name and contents.
//
static void create_file(const char *fname_a, const char *input_a)
{
    int fd = open(fname_a, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror(fname_a);
        exit(1);
    }
    write(fd, input_a, strlen(input_a));
    close(fd);
}

//
// Redirect the standard output to the given file.
//
static void setup_output(const char *fname_out)
{
    int fd = open(fname_out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror(fname_out);
        exit(1);
    }
    // Redirect stdout to this file.
    dup2(fd, 1);
    close(fd);
}

//
// Read file contents.
// The caller is responsible for deallocation of the result.
//
static const char *get_output(const char *fname_out)
{
    // Redirect stdout to stderr.
    close(1);
    dup2(2, 1);

    int fd = open(fname_out, O_RDONLY);
    if (fd < 0) {
        perror(fname_out);
        exit(1);
    }
    off_t nbytes = lseek(fd, 0, SEEK_END);
    char *contents = malloc(1 + nbytes);
    if (!contents) {
        printf("Failed to allocate %ju bytes\n", (uintmax_t)nbytes + 1);
        exit(1);
    }
    lseek(fd, 0, SEEK_SET);
    read(fd, contents, nbytes);
    contents[nbytes] = 0;
    close(fd);
    return contents;
}

//
// Run IFCOMP with given inputs.
// Return result as a string.
// The caller must deallocate the result.
//
static const char *run_ifcomp(const char *input_a, const char *input_b)
{
    create_file(fname_a, input_a);
    create_file(fname_b, input_b);
    setup_output(fname_out);
    ifcomp(fname_a, fname_b);
    fflush(stdout);
    return get_output(fname_out);
}

//
// Fail in case of premature exit.
//
void exit(int status)
{
    const char *result = get_output("result.output");
    printf("%s", result);
    free((void*)result);
    for (;;)
        fail();
}

//
// A test case with identical input files.
//
static void ab_ab(void **unused)
{
    const char *a = "A\n"   "B\n";
    const char *b = "A\n"   "B\n";
    const char *expect = "       0 lines deleted from old.\n"
                         "       0 lines inserted in new.\n"
                         "       0 lines deleted from old and replaced with 0 lines of new.\n"
                         "       0 lines moved in old.\n"
                         "       0 change blocks.\n";
    const char *result = run_ifcomp(a, b);
    assert_string_equal(result, expect);
    free((void*)result);
}

//
// A test case with deletes, moves and replacements.
//
static void axcydweabe_abcde(void **unused)
{
    const char *a = "A\n"   "X\n"   "C\n"   "Y\n"   "D\n"
                    "W\n"   "E\n"   "A\n"   "B\n"   "E\n";
    const char *b = "A\n"   "B\n"   "C\n"   "D\n"   "E\n";
    const char *expect = "*** AFTER TOP =========================================== ***\n"
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
    const char *result = run_ifcomp(a, b);
    assert_string_equal(result, expect);
    free((void*)result);
}

//
// A test case with deletes, moves and replacements.
//
static void abcdeg_defgac(void **unused)
{
    const char *a = "A\n"   "B\n"   "C\n"   "D\n"   "E\n"   "G\n";
    const char *b = "D\n"   "E\n"   "F\n"   "G\n"   "A\n"   "C\n";
    const char *expect = "*** AFTER LINE(s) ======================================= ***\n"
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
    const char *result = run_ifcomp(a, b);
    assert_string_equal(result, expect);
    free((void*)result);
}

//
// A test case from the article.
//
static void much_writing(void **unused)
{
    const char *a = "a\n"     "mass\n"     "of\n"      "latin\n"    "words\n"
                    "falls\n" "upon\n"     "the\n"     "relevant\n" "facts\n"
                    "like\n"  "soft\n"     "snow\n"    ",\n"        "covering\n"
                    "up\n"    "the\n"      "details\n" ".\n";
    const char *b = "much\n"  "writing\n"  "is\n"      "like\n"     "snow\n"
                    ",\n"     "a\n"        "mass\n"    "of\n"       "long\n"
                    "words\n" "and\n"      "phrases\n" "falls\n"    "upon\n"
                    "the\n"   "relevant\n" "facts\n"   "covering\n" "up\n"
                    "the\n"   "details\n"  ".\n";
    const char *expect = "*** AFTER LINE(s) ======================================= ***\n"
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
    const char *result = run_ifcomp(a, b);
    assert_string_equal(result, expect);
    free((void*)result);
}

//
// Run all tests.
//
int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(ab_ab),
        cmocka_unit_test(axcydweabe_abcde),
        cmocka_unit_test(abcdeg_defgac),
        cmocka_unit_test(much_writing),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
