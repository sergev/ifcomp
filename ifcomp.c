#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define first_file 0
#define second_file 1
#define other_file(f) (1 - (f))
#define two_files 2

// Transliteration of the intermetrics file comparison program.

typedef struct {
    unsigned short h1;
    long h2;
} hash_info;
typedef enum { False, True } bool;
bool debug_syt_full = False, debug_syt = False, debug_dump_trees = False,
     debug_dump_trees_full = False, debug_alloc = False, statistics = False, free_ok = True;
bool debug_read_current_line = False;
int total_file_nlines[two_files];

// 16 for 16 bits, 32 for 32.
typedef int line_count;

short nchange_blocks = 0;
typedef struct {
    line_count cosmetic, non_cosmetic;
} line_kinds;
line_kinds delete, insert, move, replace1, replace2;

//? string cosmetic = "";

#define array_of(name) \
    name##_decl *name; \
    int last_##name, hbound_##name;
#define array_of1(name, w) \
    name##_decl *name w;   \
    int last_##name w, hbound_##name w;

// Cells for linked list of line numbers.

#define null_line_list 0
typedef struct {
    line_count linen, next;
} line_table_decl;
array_of(line_table);

/*
   The string table mechanism uses a double-level hashing scheme. The
   hash nodes each share the property that the hash reduction formula
   applied to the hash_info yield the same value.

   Each hash_node contains a complete hash_info value, and the list of
   hash_nodes are in ascending order according to function
   hashcode_compare.  Each hash_node heads a list of string_decls. The
   list is in no particular order but each has a single text string in
   it and has the exact same hash_info value of each string in the list
   of string_decls.

   Presumably this double-level hashing scheme was devised in the
   original program to keep hash clashes down to a minimum.  But it's
   one more level complicated than normally found in a hashing scheme.


         fixed-length
            table (init w/null_hash_list)
            +---+
            |   |   hash_node   string_decls
            |   |       V         V            V
h1(str)&ff->|   |---->+---+    +------+    +------+
            |   |     |   | -> |      | -> |      | -> null_string_list
            |   |     +---+    +------+    +------+
            |   |       |
            |   |       V
            |   |     +---+    +------+    +------+
            +---+     |   | -> |      | -> |      | -> null_string_list
                      +---+    +------+    +------+
                        |
                        V
                  null_hash_list

 */

#define null_string_list 0
// 16:32
typedef int string_index; // One per distinct line.

typedef struct {
    // This records a unique line, the number of times it occurs
    // in each file, and the lists of the lines in each files.
    // These are linked together if they have the same hash code.
    char *text;
    string_index next_text_with_same_hash; // next line with same hash code.
    char file_nlines[two_files];
    line_count file_list[two_files]; // list of lines of text in the files.
} string_decl;

array_of(string); // Indexed by string_index.

#define null_hash_list 0
typedef short hash_node_index;

typedef struct {
    hash_info h;
    string_index text_list;
    hash_node_index next_in_bucket;
    //  char nunique_lines;
} hash_node_decl;

array_of(hash_node);

#define nbuckets 256

hash_node_index sec_hash_start_node[nbuckets];

typedef enum { syt_type = 1, unique_type, match_type } line_type;

// Arrays for files.

typedef struct {
    line_count ptr0;
    string_index file_line_text;
    line_count linen;
    line_type ptr_type;
} file_line_decl;

array_of1(file_line, [two_files]);

#define CR() printf("\n");

void internal_error(char *proc, char *fmat, ...)
{
    va_list ap;
    va_start(ap, fmat);
    printf("*** Internal error in procedure %s:", proc);
    vfprintf(stdout, fmat, ap);
    CR();
    exit(1);
}

#define char_index(in_here, find_me) strstr(in_here, find_me)

#define file1_line file_line[first_file]
#define file2_line file_line[second_file]

enum { lt = 1, eq = 2, gt = 3 };

int hashcode_compare(hash_info ha, hash_info hb)
{
    return ha.h1 < hb.h1 ? lt : ha.h1 > hb.h1 ? gt : ha.h2 < hb.h2 ? lt : ha.h2 > hb.h2 ? gt : eq;
}

// hash_line.
hash_info hash_line(char *line)
{
    char xor ;
    int i;
    hash_info h = { 0, 0 };
    xor = 0;
    int len = strlen(line);
    for (i = 0; i < len; i += 2) {
        // If the string is odd in length, we'll be including 0 in the
        // hash.
        char bite1 = line[i], bite2 = line[i + 1];
        xor = (xor | bite1) & ~(xor&bite1);
        xor = (xor | bite2) & ~(xor&bite2);
        short j = (bite1 << 8) | bite2;
        h.h2 |= 1 << (j % 31);
    }
    h.h1 = (len << 8) | xor;
    return h;
}

void print_hash_node(hash_node_decl *p)
{
    printf("h2=%lx  h1=%x  text_list=%d  nextb=%d\n", p->h.h2, p->h.h1, p->text_list,
           p->next_in_bucket);
}

void print_string(string_decl *p)
{
    printf("| %s | nexth=%d f1l=%d f2l=%d f1lst=%d f2lst=%d\n", p->text,
           p->next_text_with_same_hash, p->file_nlines[first_file], p->file_nlines[second_file],
           p->file_list[first_file], p->file_list[second_file]);
}

void dump_hash_node(hash_node_index node)
{
    printf("hash_node  %d: ", node);
    print_hash_node(hash_node + node);
    CR();
    string_index T = hash_node[node].text_list;
    while (T != null_string_list) {
        printf("string %d: ", T);
        print_string(string + T);
        void print_file_list(char *s, int T, int list)
        {
            printf("%s for text %d: ", s, T);
            while (list != null_line_list) {
                printf(" %5d@%d", line_table[list].linen, list);
                list = line_table[list].next;
            }
            CR();
        }
        print_file_list("file_list1", T, string[T].file_list[first_file]);
        print_file_list("file_list2", T, string[T].file_list[second_file]);
        T = string[T].next_text_with_same_hash;
    }
}

void dump_syt(hash_node_index node)
{ // 4366
    printf("** symbol table dump **, start=%d \n", node);
    while (node != null_hash_list)
        dump_hash_node(node), node = hash_node[node].next_in_bucket;
}

void format_file_line(file_line_decl *p)
{
    printf("|%3d|", p->linen);
    switch (p->ptr_type) {
    case syt_type:
        printf("S     ");
        break;
    case unique_type:
        printf("U%5d", p->ptr0);
        break;
    case match_type:
        printf("M%5d", p->ptr0);
        break;
    default:
        printf("??????");
        break;
    }
    printf("|%s|\n", string[p->file_line_text].text);
}

static inline int _max(int a, int b)
{
    return (a > b) ? a : b;
}

void test_list(int pass)
{ // 4634
    line_count i = _max(total_file_nlines[first_file], total_file_nlines[second_file]);
    printf("test list after pass%d\n", pass);
    for (line_count j = 1; j <= i; j++) {
        if (j > total_file_nlines[first_file])
            printf("=============\n");
        else
            format_file_line(file1_line + j);
        if (j <= total_file_nlines[second_file])
            format_file_line(file2_line + j);
    }
    CR();
}

void memory_statistics();

int next_index_func(int *last, int *hbound, char *name, int size, void **p)
{
    // if (debug_alloc) printf("alloc from %s(%p), hb=%d last=%d\n",name,*p,*hbound,*last);
    if (*last + 1 < *hbound)
        return ++(*last);
    else {
        unsigned cur_size = *hbound * size, new_size = 2 * cur_size;
        // Be sure no math overflow.
        if (new_size > cur_size) {
            if (debug_alloc)
                printf("attempt realloc of %u bytes for %s, nhb=%d\n", new_size, name, 2 * *hbound);
            void *newp = realloc(*p, new_size);
            if (newp != 0) {
                *hbound = 2 * *hbound;
                *p = newp;
                return next_index_func(last, hbound, name, size, p);
            } else
                printf("Realloc returned 0.\n");
        } else
            printf("New size shrank!:%u\n", new_size);
    }
    printf("out of space on %s: hbound index %d\n", name, *hbound), memory_statistics(), exit(1);
}

#define next_index(name) \
    next_index_func(&last_##name, &hbound_##name, #name, sizeof(*name), (void **)&name)
#define str(x) #x
#define next_index1(name, w)                                                            \
    next_index_func(&last_##name[w], &hbound_##name[w], str(name##w), sizeof(*name[w]), \
                    (void **)&name[w])

line_count make_line_entry(line_count linen, line_count next)
{
    line_count i = next_index(line_table);
    line_table[i].linen = linen;
    line_table[i].next = next;
    return i;
}

string_index make_string(string_decl *p)
{
    string_index i = next_index(string);
    string[i] = *p;
    return i;
}

hash_node_index make_hash_node(hash_node_decl *p)
{
    hash_node_index i = next_index(hash_node);
    hash_node[i] = *p;
    return i;
}

#define PUBLIC

PUBLIC void terminate_printf(int rc, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    exit(rc);
}
static void *Check_mem(void *v, size_t Size, const char *where)
{
    if (!v)
        terminate_printf(1, "Memory exhausted on allocating %d bytes: %s failed.\n", Size, where);
    return v;
}

PUBLIC void *Mmalloc(size_t Amount)
{
    if (Amount == 0)
        Amount = 1; // 0 fails on aix 386.
    return Check_mem((void *)malloc(Amount), Amount, "Mmalloc");
}

long string_bytes = 0;

char *scopy(const char *p)
{
#define POOL_SEG_SIZE 512
    static char *strpool = 0;
    static char *poolptr = 0;
    char *q;
    static int total = 0;
    int len = strlen(p);
    if (strpool == 0 || poolptr + len + 1 >= strpool + POOL_SEG_SIZE) {
        strpool = poolptr = Mmalloc(POOL_SEG_SIZE), total += POOL_SEG_SIZE;
    }
    memcpy(poolptr, p, len + 1);
    q = poolptr;
    poolptr += len + 1;
    string_bytes += len;
    return q;
}

void enter_line(char *text, hash_info h, line_count linen, int input_file,
                hash_node_index *result_hash_node, string_index *result_string_index)
{
    if (debug_syt_full)
        printf("\nEnter line %s, #%d\n", text, linen);
    string_index setup_distinct_text()
    {
        // New piece of text.
        string_decl S;
        int other = other_file(input_file);
        S.file_nlines[input_file] = 1;
        S.file_nlines[other] = 0;
        S.file_list[input_file] = make_line_entry(linen, null_line_list);
        S.file_list[other] = null_line_list;
        S.next_text_with_same_hash = null_string_list;
        S.text = scopy(text);
        return make_string(&S);
    }
    hash_node_index setup_hash_node(string_index * tip)
    {
        hash_node_decl S;
        S.next_in_bucket = null_hash_list;
        S.text_list = *tip = setup_distinct_text();
        //  	S.nunique_lines = 1;
        S.h = h;
        return make_hash_node(&S);
    }
    void add_linen_to_text_list(string_index T)
    {
        string[T].file_nlines[input_file]++;
        line_count *p = &string[T].file_list[input_file];
        *p = make_line_entry(linen, *p);
    }
    hash_node_index *hash_start_node = &sec_hash_start_node[h.h1 % nbuckets];
    string_index SI;
    hash_node_index node;
    if (*hash_start_node == null_hash_list) {
        *hash_start_node = node = setup_hash_node(&SI);
        goto finish;
    }
    node = *hash_start_node;
    hash_node_index last_node;
    string_index last_SI;
    while (node != null_hash_list) {
        int test = hashcode_compare(h, hash_node[node].h);
        if (test == eq) {
            // Search through this syt node to see if the identical line
            // exists already.
            for (SI = hash_node[node].text_list; SI != null_string_list;
                 last_SI = SI, SI = string[SI].next_text_with_same_hash) {
                if (strcmp(string[SI].text, text) == 0) {
                    add_linen_to_text_list(SI);
                    goto finish;
                }
            }
            string[last_SI].next_text_with_same_hash = SI = setup_distinct_text();
            goto finish;
        }
        if (test == lt) {
            int new_node = setup_hash_node(&SI);
            if (node == *hash_start_node)
                hash_node[new_node].next_in_bucket = *hash_start_node, *hash_start_node = new_node;
            else
                hash_node[new_node].next_in_bucket = node,
                hash_node[last_node].next_in_bucket = new_node;
            node = new_node;
            goto finish;
        }
        // test is gt.
        last_node = node;
        int N = hash_node[node].next_in_bucket;
        if (N < 0 || N > hbound_hash_node)
            printf("OOPS! node=%d &=%p\n", N, &hash_node[node].next_in_bucket);
        node = hash_node[node].next_in_bucket;
    }
    // Add to chain.
    if (last_node == null_hash_list)
        printf("?OOPS empty list!\n");
    hash_node[last_node].next_in_bucket = node = setup_hash_node(&SI);
finish:;
    *result_hash_node = node;
    *result_string_index = SI;
}

FILE *input_file[two_files];

void read_lines(int which_file)
{
    int current_line = 0;
    while (1) {
        char buf[4096];
        *buf = 0;
        fgets(buf, sizeof buf, input_file[which_file]);
        // feof only occurs after attempting to read after the end.
        if (*buf == 0 && feof(input_file[which_file]))
            break;
        int len = strlen(buf);
        if (len && buf[len - 1] == '\n')
            buf[len - 1] = 0;
        if (debug_read_current_line)
            printf("read %s\n", buf);
        current_line = next_index1(file_line, which_file);
        hash_node_index H;
        enter_line(buf, hash_line(buf), current_line, which_file,
                   &H, // file_line[which_file][current_line].ptr0,
                   &file_line[which_file][current_line].file_line_text);
        // ptr0 should never be used until it is assigned.
        // As long as the type is syt_type it holds nothing of interest.
        // The original program set ptr0 to H, but this makes no sense.
        file_line[which_file][current_line].ptr0 = -1; // No line.
        file_line[which_file][current_line].linen = current_line;
        file_line[which_file][current_line].ptr_type = syt_type;
        if (debug_syt_full)
            dump_syt(H);
    }
    total_file_nlines[which_file] = current_line;
    if (current_line == 0) {
        printf("File %d has no lines.\n", which_file);
        exit(which_file);
    }
}

// Pass1.
void pass1()
{
    read_lines(first_file);
    read_lines(second_file);
    // We can free the hash stuff; not needed now.
    free(hash_node);
}

void pass2()
{
    for (int i = 1; i <= last_string; i++) {
        // Look at each line.  If it occurs once in both files,
        // record both as unique.
        if (string[i].file_nlines[first_file] == 1 && string[i].file_nlines[second_file] == 1) {
            // Found a unique pair.
            line_count file_linen1 = line_table[string[i].file_list[first_file]].linen;
            line_count file_linen2 = line_table[string[i].file_list[second_file]].linen;
            // Make each line reference the occurrence in the other file.
            file1_line[file_linen1].ptr_type = unique_type;
            file1_line[file_linen1].ptr0 = file_linen2;
            file2_line[file_linen2].ptr_type = unique_type;
            file2_line[file_linen2].ptr0 = file_linen1;
        }
    }
}

void pass3()
{
    line_count m = 1;
    while (m <= total_file_nlines[first_file]) {
        if (file1_line[m].ptr_type == unique_type) {
            line_count n = file1_line[m].ptr0; // Location in file 2.
            // Broaden matches.  Look for lines that follow unique_type
            // lines and which are not marked unique.  If corresponding
            // lines match mark them match_type.
            for (m++, n++;
                 file1_line[m].ptr_type == syt_type && m <= total_file_nlines[first_file] &&
                 file1_line[m].file_line_text == file2_line[n].file_line_text;
                 m++, n++) {
                file1_line[m].ptr_type = match_type;
                file1_line[m].ptr0 = n;
                file2_line[n].ptr_type = match_type;
                file2_line[n].ptr0 = m;
            }
        } else
            m++;
    }
}

void pass4()
{
    line_count m = total_file_nlines[first_file];
    while (m > 0) {
        if (file1_line[m].ptr_type == unique_type) {
            // Broaden matches in the backwards direction.
            line_count n = file1_line[m].ptr0;
            for (m--, n--; file1_line[m].ptr_type == syt_type && m > 0 &&
                           file2_line[n].ptr_type == syt_type &&
                           file1_line[m].file_line_text == file2_line[n].file_line_text;
                 m--, n--) {
                file1_line[m].ptr_type = match_type;
                file1_line[m].ptr0 = n;
                file2_line[n].ptr_type = match_type;
                file2_line[n].ptr0 = m;
            }
        } else
            m--;
    }
}

// Node declarations for the trees.

// 16:32
typedef int tree_index;

typedef struct {
    line_count cost;
    line_count linen;
    tree_index prev, next, branch_start, branch_end;
} node_decl;

array_of(node);

#define null_node 0
tree_index free_nodes_start = null_node;

#define leaf(N) (node[N].branch_start == null_node)

typedef struct {
    tree_index start, end;
} tree_bounds;
tree_bounds trees[two_files];

// Shorthand:
#define tree1 trees[first_file]
#define tree2 trees[second_file]
#define tree1_start tree1.start
#define tree2_start tree2.start
#define tree1_end tree1.end
#define tree2_end tree2.end

tree_index allocate_node()
{
    if (free_nodes_start == null_node)
        return next_index(node);
    else {
        int i = free_nodes_start;
        free_nodes_start = node[free_nodes_start].next;
        return i;
    }
}

void free_node(tree_index n)
{
    if (!free_ok)
        return;
    node[n].next = free_nodes_start;
    free_nodes_start = n;
}

#define cosmetic_line(c) False

#if 0 // I don't understand this stuff.
bool cosmetic_line(char first_byte) {
    int len = strlen(cosmetic);
    for (i = 0; i < len; i++)
	if (first_byte == cosmetic[i]) return True;
    return False;
    }
#endif

// If a node is "not a leaf", it means it has been constructed of
// parts of one file and parts of another.  The sequence of parts
// begins at branch_start and terminates back with the original node.

#define get_start_finish(noden)                           \
    tree_index start, finish;                             \
    if (!leaf(noden))                                     \
        start = node[noden].branch_start, finish = noden; \
    else                                                  \
        start = noden, finish = node[noden].next;

#define get_which_file(fileno, linen)             \
    {                                             \
        if (linen < 0)                            \
            fileno = second_file, linen = -linen; \
        else                                      \
            fileno = first_file;                  \
    }

static inline int _abs(int a)
{
    return (a < 0) ? -a : a;
}

// Call a function for each line.

void each_line_in_node(int noden, bool always, int starting_line,
                       void (*func)(int which_file, char *text, int lineno, void *arg), void *arg)
{
    get_start_finish(noden);
    for (; start != finish; start = node[start].next) {
        line_count filen;
        line_count sline = node[start].linen;
        get_which_file(filen, sline);
        // cost is the number of nodes.  Can be negative.
        int cost = node[start].cost;
        if (always)
            cost = _abs(cost);
        int last = sline + cost;
        file_line_decl *fp = file_line[filen];
        // He may have passed a place to start later than the beginning of
        // a node.
        for (sline = _max(sline, starting_line); sline < last; sline++)
            func(filen, string[fp[sline].file_line_text].text, fp[sline].linen, arg);
    }
}

void count_node_callback(int which_file, char *text, int lineno, void *arg)
{
    line_kinds *p = arg;
    if (cosmetic_line(*text))
        p->cosmetic++;
    else
        p->non_cosmetic++;
}

void count_node(int noden, line_kinds *p)
{
    each_line_in_node(noden, False, 0, count_node_callback, p);
}

void format_node(tree_index noden, int pad)
{
#define X(f) node[noden].f
    for (int i = 0; i < pad; i++)
        printf(" ");
    printf("[%d<-N%d->%d, cost=%2d linen=%2d", X(prev), noden, X(next), X(cost), X(linen));
    line_count L = X(linen);
    int fileno;
    get_which_file(fileno, L);
    printf("(%d)", file_line[fileno][L].ptr0);
    if (X(branch_start) != null_node || X(branch_end) != null_node)
        printf(" bs=%2d be=%2d", X(branch_start), X(branch_end));
    printf("]\n");
}

void print_node1_callback(int which_file, char *text, int lineno, void *arg)
{
    printf("%c%6d|%s\n", which_file == first_file ? ' ' : '+', lineno, text);
}
void print_node1(tree_index noden, bool always, int starting_line)
{
    each_line_in_node(noden, always, starting_line, print_node1_callback, NULL);
}
void print_node(tree_index noden)
{
    print_node1(noden, False, 0);
}

tree_index make_node(node_decl *p)
{
    int i = next_index(node);
    node[i] = *p;
    if (debug_dump_trees_full)
        printf("just made "), format_node(i, 0);
    return i;
}

void dump_tree(tree_index tree_start)
{
    printf("Tree %d:\n", tree_start);
    bool branch = False;
    tree_index T = tree_start;
    while (T != null_node) {
        tree_index T2 = T;
        if leaf (T) {
            format_node(T, branch ? 8 : 1), T = node[T].next;
            if (debug_dump_trees_full)
                print_node1(T2, True, 0);
        } else {
            if (branch)
                branch = False, T = node[T].next;
            else
                format_node(T, 1), T = node[T].branch_start, branch = True;
        }
    }
}

#define no_pass 99
void dump_trees(int pass)
{
    if (!debug_dump_trees)
        return;
    printf(pass == no_pass ? "dump trees\n" : "dump_trees after pass%d\n", pass);
    dump_tree(tree1_start);
    dump_tree(tree2_start);
}

void pass5()
{ // 70e0
    void doit(int fileno, node_decl *Np)
    {
        if (debug_dump_trees)
            printf("Make tree for file %d\n", fileno + 1);
        // This tree is initially just a doubly-linked list of the separate
        // segments of the file that were identified in previous passes.
        // The branch_start and branch_end fields have no contents.
        // There are also a header and trailer node for each file.
        line_count i = 1;
        while (i <= total_file_nlines[fileno]) {
            Np->linen = i;
            file_line_decl *fp = file_line[fileno];
            line_type ptr_type = fp[i].ptr_type;
            if (ptr_type == syt_type) {
                // Determine a block of syt_type lines.
                while (fp[i + 1].ptr_type == syt_type && i + 1 <= total_file_nlines[fileno])
                    i++;
                i++;
                Np->cost = i - Np->linen;
                Np->cost = -Np->cost; // Signifies delete.
            } else {
                // Determine a block of non-syt_type lines.
                line_count ptr0 = fp[i].ptr0, exp_ptr0 = ptr0 + 1;
                while (fp[i + 1].ptr_type != syt_type && fp[i + 1].ptr0 == exp_ptr0 &&
                       i + 1 <= total_file_nlines[fileno])
                    i++, exp_ptr0++;
                i++;
                Np->cost = i - Np->linen;
            }
            if (fileno == second_file)
                Np->linen = -Np->linen;
            tree_index j = make_node(Np);
            node[Np->prev].next = j;
            Np->prev = j;
        }
    }
    node_decl N;
    N.cost = 0;
    N.linen = 0;
    N.next = N.prev = null_node;
    N.branch_start = N.branch_end = null_node;
    // Make header nodes.
    tree1_start = make_node(&N);
    tree2_start = make_node(&N);
    N.prev = tree1_start;
    doit(first_file, &N);

    N.cost = 0;
    int file1_tlinesp = N.linen = total_file_nlines[first_file] + 1;
    tree1_end = make_node(&N);
    node[N.prev].next = tree1_end;

    N.prev = tree2_start;
    doit(second_file, &N);
    N.cost = 0;
    int file2_tlinesp;
    N.linen = -(file2_tlinesp = total_file_nlines[second_file] + 1);
    tree2_end = make_node(&N);
    node[N.prev].next = tree2_end;

    // Now be sure that the header records can refer to each other,
    // since it may occur (e.g. pass8) that we look up line 0 in
    // the other file.
    file1_line[0].ptr0 = file2_line[0].ptr0 = 0;
    // Also make the trailers talk to each other.
    // First be sure the file_line array has space for the extra line
    // number.
    next_index1(file_line, first_file);
    next_index1(file_line, second_file);
    file1_line[file1_tlinesp].ptr0 = file2_tlinesp;
    file2_line[file2_tlinesp].ptr0 = file1_tlinesp;
}

#define true_line_of(N) _abs(node[N].linen)

tree_index find_node(tree_bounds T, tree_index linen)
{ // 5eb0
    linen = _abs(linen);
    tree_index N = T.start;
    while (N != T.end)
        if (true_line_of(N) == linen) {
            if (debug_dump_trees_full)
                printf("In tree %d:%d, find line %d at %d\n", T.start, T.end, linen, N);
            return N;
        } else
            N = node[N].next;
    // OK, tell how the problem happened:
    N = T.start;
    printf("[");
    while (N != T.end)
        printf("%d ", N), N = node[N].next;
    printf("] ln=%d\n", linen);
    internal_error("find_node", " sn=%d en=%d l=%d", T.start, T.end, linen);
}

void detach_node(tree_index noden)
{ // 5f20
    // Remove noden from the linked list.
    tree_index prev = node[noden].prev, next = node[noden].next;
    node[prev].next = next, node[next].prev = prev;
}

void combine_nodes(tree_index node1, tree_index node2)
{ // 5fd8
    tree_index branch_link1, branch_link2;
    node_decl N;
    N.cost = node[node1].cost + node[node2].cost;
    N.linen = node[node1].linen;
    // First remove node2 from file2.
    // Node2 must be detached first to get a true last and next ptr
    // from node1 -- i.e., node2 may be adjacent to node1.
    detach_node(node2);
    N.prev = node[node1].prev;
    N.next = node[node1].next;
    // Now remove node1 from file1.
    detach_node(node1);
    if (!leaf(node1)) {
        // Just want the branch.
        N.branch_start = node[node1].branch_start;
        branch_link1 = node[node1].branch_end;
        // The sequence in node1 is absorbed in N and hence isn't needed.
        free_node(node1);
        node1 = N.branch_start;
    } else
        N.branch_start = branch_link1 = node1;
    if (!leaf(node2)) {
        branch_link2 = node[node2].branch_start;
        N.branch_end = node[node2].branch_end;
        // The sequence in node2 is absorbed in N and hence isn't needed.
        free_node(node2);
        node2 = branch_link2;
    } else
        branch_link2 = N.branch_end = node2;
    tree_index new_node = make_node(&N);
    // Insert new_node after N.prev and before N.next; i.e., it replaces node1.
    node[N.prev].next = node[N.next].prev = new_node;
    node[N.branch_start].prev = node[N.branch_end].next = new_node;
    node[branch_link1].next = branch_link2;
    node[branch_link2].prev = branch_link1;
}

void ph(char *s, char dash, bool space)
{
    int len = strlen(s);
    printf("*** %s", s);
    int pad = 52;
    if (space)
        printf(" ");
    else
        pad++;
    for (int i = 0; i < pad - len; i++)
        printf("%c", dash);
    printf(" ***\n");
}
#define print_header(s) ph(s, '=', True)
#define print_header1(s) ph(s, '-', True)

void print_trailer()
{
    ph("", '=', False);
    printf("\n");
}

void after_lines(tree_index noden)
{ // 6320
    tree_index unique_find(tree_index noden)
    { // 5de0
        tree_index end_line = node[noden].linen;
        short filen;
        get_which_file(filen, end_line);
        // Scan backwards looking for a unique line in the file
        // -- i.e., it must not occur more than once in the file.
        for (line_count start_line = end_line + node[noden].cost - 1; start_line >= end_line;
             start_line--)
            if (file_line[filen][start_line].ptr_type == unique_type)
                return start_line;
        return null_node;
    }
    print_header("AFTER LINE(s)");
    // Print the block starting at the last line that is unique
    // in the file.  I.e., be sure the reader can identify the text.
    tree_index start = noden, last = node[start].next;
    line_count linen = 0;
loop1:;
    while (start != tree1_start) {
        if (leaf(start)) {
            linen = unique_find(start);
            if (linen != null_node)
                break;
            linen = 0;
            last = start;
            start = node[start].prev;
            goto loop1;
        }
        if (last == node[start].branch_start)
            start = node[start].prev, last = start;
        else
            last = start, start = node[start].branch_end;
    }
    print_node1(start, False, linen);
    last = start, start = node[start].next;
loop2:;
    while (start != node[noden].next) {
        if (leaf(start)) {
            print_node(start);
            last = start, start = node[start].next;
            goto loop2;
        }
        if (last == node[start].branch_end)
            last = start, start = node[start].next;
        else
            last = start, start = node[start].branch_start;
    }
}

void after_header(tree_index noden)
{
    if (noden == tree1_start)
#define top_msg() print_header("AFTER TOP")
        top_msg();
    else
        after_lines(noden);
}

void delete_lines(tree_index noden)
{ // 6626
    nchange_blocks++;
    after_header(node[noden].prev);
    node[noden].cost = -node[noden].cost; // Indicate delete (?).
    print_header1("DELETE LINE(s)");
    print_node(noden);
    print_trailer();
    count_node(noden, &delete);
    detach_node(noden);
    dump_trees(no_pass);
}

void pass6()
{ // 7508
    tree_index replaceable(tree_index noden)
    { // 6a18
        // Replaceable if :
        // file1:  blk1  nodenA  blk2	file2:  blk3  nodenB  blk4
        // where nodenA and nodenB don't match something in the other file (cost<0)
        // and blk1=blk3 and blk2=blk4.
        // It appears, however, that Reed took out the blk2=blk4 test.
        // See if noden in first_file can be replaced
        // with something else in second_file.
        // Find the previous node to this sequence.
        tree_index prev = node[noden].prev;
        // Lookup that previous node in the other file.
        tree_index prev_other_file = find_node(tree2, file1_line[true_line_of(prev)].ptr0);
        // OK, now find the successor that node in the other file.
        // This corresponds to our noden.
        tree_index noden_other_file = node[prev_other_file].next;
        // Ask if the successor node is unique (cost < 0).  Otherwise
        // it isn't a replacement.
        if (node[noden_other_file].cost >= 0) {
            if (debug_dump_trees_full)
                printf("replaceable fails: noden_other_file(%d) has neg cost.\n", noden_other_file);
            return null_node;
        }
#if 0
	// It looks like Reed removed this test.
	// OK, it's unique.  A possible replacement.
	// Now check the successor in the other file and find it in our file.
	tree_index succ_other_file = node[noden_other_file].next;
	tree_index succ = find_node(tree1,file2_line[true_line_of(succ_other_file)].ptr0);
	// Now ask if the succ_other_file's correspondent in our file
	// happens to be the successor of the node we're looking at.
	// If so, we have a replacement.
	if (succ != node[noden].next) {
	    if (debug_dump_trees_full)
		printf("replaceable fails: succ(%d) <> next(%d).\n",
			succ,node[noden].next);
	    return null_node;
	    }
#endif
        return noden_other_file;
    }

    void replace_lines(tree_index node1, tree_index node2)
    { // 6b08
        nchange_blocks++;
        // Make the costs positive, indicating that the nodes now
        // correspond to something in the other file.
        node[node1].cost = -node[node1].cost;
        node[node2].cost = -node[node2].cost;
        count_node(node1, &replace1);
        count_node(node2, &replace2);
        tree_index prev = node[node1].prev;
        after_header(prev);
        print_header1("REPLACE LINE(s)");
        print_node(node1);
        print_header1("WITH LINE(s)");
        print_node(node2);
        print_trailer();
        detach_node(node1);
        if (prev == tree1_start) {
            detach_node(node2);
            node[tree1_start].branch_start = node[tree1_start].branch_end = node2;
            node[node2].prev = node[node2].next = tree1_start;
        } else
            combine_nodes(prev, node2);
        dump_trees(no_pass);
    }

    void insert_lines(tree_index noden)
    { // 6dea
        nchange_blocks++;
        node[noden].cost = -node[noden].cost;
        count_node(noden, &insert);
        tree_index i = node[noden].prev;
        if (i == tree2_start) {
            detach_node(noden);
            node[tree1_start].branch_start = node[tree1_start].branch_end = noden;
            node[noden].prev = node[noden].next = tree1_start;
            top_msg();
#define insert_msg() print_header1("INSERT LINE(s)")
            insert_msg();
            print_node(tree1_start);
        } else {
            tree_index j = find_node(tree1, file2_line[true_line_of(i)].ptr0);
            after_lines(j);
            insert_msg();
            print_node(noden);
            combine_nodes(j, noden);
        }
        print_trailer();
        dump_trees(no_pass);
    }

    void do_replace_delete()
    {
        // Scan through first_file and identify any nodes that
        // have no correspondent in the second_file.  See if they can be
        // treated as replaced or deleted in the other file.

        tree_index i = node[tree1_start].next;
        while (i != tree1_end) {
            tree_index j = node[i].next;
            if (node[i].cost < 0) {
                tree_index location_in_other_file = replaceable(i);
                if (location_in_other_file == null_node)
                    delete_lines(i);
                else
                    replace_lines(i, location_in_other_file);
            }
            i = j;
        }
    }

    void do_insert()
    {
        // Scan through second_file and identify any nodes that have no
        // correspondent in first_file.  They are treated as inserted in the
        // first_file.

        tree_index i = node[tree2_start].next;
        while (i != tree2_end) {
            if (node[i].cost < 0) {
                tree_index j = node[i].next;
                insert_lines(i);
                i = j;
            } else
                i = node[i].next;
        }
    }

    // Reed switched the order of insert vs. replace and delete.
    do_replace_delete();
    do_insert();
}

void pass7()
{ // 75ee

    bool combine_adjacent_nodes(tree_index node1)
    { // 654c
        // Look at adjacent nodes node1 and node2.
        // If they are also adjacent in file 2, combine the nodes
        // in both files.
        tree_index node2 = node[node1].next;
        if (debug_dump_trees_full)
            printf("combine node1=%d ln=%d to node2=%d ln=%d\n", node1, node[node1].linen, node2,
                   node[node2].linen);
        tree_index i = find_node(tree2, file1_line[true_line_of(node1)].ptr0);
        tree_index j = find_node(tree2, file1_line[true_line_of(node2)].ptr0);
        if (j == node[i].next) {
            combine_nodes(node1, node2);
            combine_nodes(i, j);
            return True;
        } else
            return False;
    }

    tree_index i = node[tree1_start].next;
    while (node[i].next != tree1_end) {
        tree_index j = node[i].prev;
        i = node[combine_adjacent_nodes(i) ? j : i].next;
    }
}

void insert_node_after(tree_index after_this, tree_index insert_this)
{
    node[insert_this].prev = after_this;
    tree_index after_after = node[after_this].next;
    node[insert_this].next = after_after;
    node[after_after].prev = insert_this;
    node[after_this].next = insert_this;
}

void pass8()
{ // 7678
    void move_lines(tree_index node1, tree_index node2)
    { // 679a
        nchange_blocks++;
        count_node(node2, &move);
        if (node1 == tree1_start) {
            after_header(node1);
#define move_msg() print_header1("MOVE LINE(s)");
            move_msg();
            print_node(node2);
            print_trailer();
            detach_node(node2);
            insert_node_after(tree1_start, node2);
        } else {
            after_lines(node1);
            move_msg();
            print_node(node2);
            print_trailer();
            // Calling combine violates the 1:1 assumption.
            detach_node(node2);
            insert_node_after(node1, node2);
            // combine_nodes(node1,node2);
            // The file retains the 1:1 assumption.  Combine adjacent
            // nodes to appropriately redistribute weight for min_cost.
            pass7(); // See if any nodes can now be made adjacent.
        }
    }

    // Find node with minimum cost between start_node and end_node.
    tree_index min_cost_node(tree_index start_node, tree_index end_node)
    { // 5f6a
        tree_index min_cost = node[start_node].cost, min_node = start_node;
        tree_index N = start_node;
        while (N != end_node) {
            if (min_cost > node[N].cost)
                min_cost = node[min_node = N].cost;
            N = node[N].next;
        }
        if (debug_dump_trees_full)
            printf("min_cost_node(%d,%d)=%d\n", start_node, end_node, min_node);
        return min_node;
    }

    // Now do the moves.
RETRY:;
    tree_index i = tree1_start, j = tree2_start;
    while (i != tree1_end) {
        // First time through, this skips the header.
        i = node[i].next, j = node[j].next;
        // Scan through the two files while file1 references the same
        // line in file2.
        if (debug_dump_trees_full)
            printf("node %d lno %d -> %d, node %d lno %d\n", i, true_line_of(i),
                   file1_line[true_line_of(i)].ptr0, j, true_line_of(j));
        while (file1_line[true_line_of(i)].ptr0 == true_line_of(j) && i != tree1_end)
            i = node[i].next, j = node[j].next;
        if (i == tree1_end)
            return;
        tree_index k = min_cost_node(i, tree1_end);
        tree_index l = find_node(tree2, file1_line[true_line_of(k)].ptr0);
        tree_index m = node[l].prev;
        // m might be the header node with line 0; this requires
        // find_node to be able to find the header node.
        // The original ifcomp program had a bug in this line.
        tree_index n = find_node(tree1, file2_line[true_line_of(m)].ptr0);
        move_lines(n, k);
        // We can't detach node l yet.  We require keeping all moved
        // segments within the other file, or else we will prevent
        // future scanning in parallel.
        // The original program had this statement here.
        // detach_node(l);
        dump_trees(no_pass);
        goto RETRY;
    }
}

int sum_kind(line_kinds l)
{
    return l.non_cosmetic + l.cosmetic;
}

void summary()
{
    //  #define nums(l) l.cosmetic+l.non_cosmetic,l.cosmetic,l.non_cosmetic
    //  #define NFMAT "%d(%d cos, %d non-cos)"
#define NFMAT "%8d "
#define nums(l) l.non_cosmetic
    printf(NFMAT "lines deleted from old.\n", nums(delete));
    printf(NFMAT "lines inserted in new.\n", nums(insert));
    printf(NFMAT
           "lines deleted from old and replaced with %d "
           "lines of new.\n",
           nums(replace1), nums(replace2));
    printf(NFMAT "lines moved in old.\n", nums(move));
    printf("%8d change blocks.\n", nchange_blocks);
}

void *alloc(unsigned size)
{
    void *p = malloc(size);
    if (p == 0)
        printf("out of memory\n"), memory_statistics(), exit(1);
    return p;
}

#define alloc_thing(thing, number) \
    (thing = alloc(number * sizeof(*thing)), last_##thing = 0, hbound_##thing = number)
#define alloc_thing1(thing, number, w) \
    (thing = alloc(number * sizeof(*thing)), last_##thing##w = 0, hbound_##thing##w = number)

void allocate_tables()
{
    // Start with allocating a tiny about to be sure to exercise
    // our memory expander.
    alloc_thing(hash_node, 1);
    alloc_thing(string, 1);
    alloc_thing(line_table, 2);
    alloc_thing(node, 2);
    alloc_thing(file_line[first_file], 1);
    alloc_thing(file_line[second_file], 1);
}

void doit()
{
    for (int i = 0; i < nbuckets; i++)
        sec_hash_start_node[i] = null_hash_list;
    allocate_tables();
    void (*A[])() = { pass1, pass2, pass3, pass4 };
#define asize(A) (sizeof(A) / sizeof((A)[0]))
    for (int i = 0; i < asize(A); i++) {
        A[i]();
        if (debug_syt)
            test_list(i + 1);
    }
    void (*B[])() = { pass5, pass6, pass7, pass8 };
    for (int i = 0; i < asize(B); i++) {
        B[i]();
        dump_trees(i + asize(A) + 1);
    }
    summary();
}

FILE *open_file(char *fn)
{
    FILE *fp = fopen(fn, "r");
    if (fp == 0)
        printf("Can't open file %s:%s\n", fn, strerror(errno)), exit(1);
    return fp;
}

void memory_statistics()
{
    long mem_used = 0;
    unsigned msize;
#define P(X)                                                                      \
    printf("%8d (%d max, %u bytes) %s entries used.\n", last_##X, hbound_##X - 1, \
           msize = (hbound_##X - 1) * sizeof(*X), #X),                            \
        mem_used += msize
    P(hash_node);
    mem_used = 0; // Don't count hash_node.
    P(string);
    P(line_table);
    P(file_line[first_file]);
    P(file_line[second_file]);
    printf("\t\thash_node space was freed before allocating nodes:\n");
    P(node);
#undef P
    printf("%8ld bytes of line texts.\n", string_bytes);
    mem_used += string_bytes;
    printf("%8ld total bytes of memory used.\n", mem_used);
}

void print_statistics()
{
    printf("\nStatistics:\n");
    memory_statistics();
}

void main(int argc, char **argv)
{
    int fileno = first_file;
    void help()
    {
        printf("Usage is: %s file1 file2\n", argv[0]);
        exit(1);
    }
    char *fnames[two_files];
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
#define p_is(s) (strcmp(arg, s) == 0)
        if (arg[0] == '-')
            if (p_is("-stfull"))
                debug_syt_full = True;
            else if (p_is("-st"))
                debug_syt = True;
            else if (p_is("-trees"))
                debug_dump_trees = True;
            else if (p_is("-treesfull"))
                debug_dump_trees_full = True;
            else if (p_is("-alloc"))
                debug_alloc = True;
            else if (p_is("-stat"))
                statistics = True;
            else if (p_is("-nofree"))
                free_ok = False;
            else if (p_is("-debug")) {
                debug_syt_full = True;
                debug_syt = True;
                debug_dump_trees = True;
                debug_dump_trees_full = True;
                // debug_alloc = True;
            } else
                help();
        else if (fileno < two_files)
            input_file[fileno] = open_file(fnames[fileno] = arg), fileno++;
        else
            help();
    }
    if (fileno <= second_file)
        help();
    printf("Comparing:  %s %s\n\n", fnames[first_file], fnames[second_file]);
    doit();
    if (statistics)
        print_statistics();
}
