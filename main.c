#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ifcomp.h"

static void help(const char *prog_name)
{
    printf("Usage is: %s file1 file2\n", prog_name);
    exit(1);
}

int main(int argc, char **argv)
{
    char *first_fname = 0;
    char *second_fname = 0;
    bool statistics = false;

    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
#define p_is(s) (strcmp(arg, s) == 0)
        if (arg[0] == '-')
            if (p_is("-stfull"))
                debug_syt_full = true;
            else if (p_is("-st"))
                debug_syt = true;
            else if (p_is("-trees"))
                debug_dump_trees = true;
            else if (p_is("-treesfull"))
                debug_dump_trees_full = true;
            else if (p_is("-alloc"))
                debug_alloc = true;
            else if (p_is("-stat"))
                statistics = true;
            else if (p_is("-nofree"))
                debug_dont_free = true;
            else if (p_is("-debug")) {
                debug_syt_full = true;
                debug_syt = true;
                debug_dump_trees = true;
                debug_dump_trees_full = true;
                // debug_alloc = true;
            } else
                help(argv[0]);
        else if (!first_fname)
            first_fname = arg;
        else if (!second_fname)
            second_fname = arg;
        else
            help(argv[0]);
    }
    if (!first_fname || !second_fname)
        help(argv[0]);

    printf("Comparing: %s %s\n\n", first_fname, second_fname);
    ifcomp(first_fname, second_fname);

    if (statistics) {
        printf("\nStatistics:\n");
        print_statistics();
    }
}
