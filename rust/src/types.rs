// File index enumeration - strong type for file identification
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum FileIndex {
    First = 0,
    Second = 1,
}

impl FileIndex {
    pub fn other_file(self) -> FileIndex {
        match self {
            FileIndex::First => FileIndex::Second,
            FileIndex::Second => FileIndex::First,
        }
    }

    pub fn to_array_index(self) -> usize {
        self as usize
    }
}

// Line count type
pub type LineCount = i32;

// String index - one per distinct line
pub type StringIndex = usize;

pub const NULL_STRING_LIST: StringIndex = 0;

// Hash node index
pub type HashNodeIndex = i16;

pub const NULL_HASH_LIST: HashNodeIndex = 0;

// Line table entry (for linked list of line numbers)
#[derive(Debug, Clone, Copy)]
pub struct LineTableDecl {
    pub linen: LineCount,
    pub next: LineCount,
}

pub const NULL_LINE_LIST: LineCount = 0;

// String declaration - records a unique line
#[derive(Debug, Clone)]
pub struct StringDecl {
    pub text: String,
    pub next_text_with_same_hash: StringIndex,
    pub file_n_lines: [usize; 2],
    pub file_list: [LineCount; 2],
}

// Hash node declaration
#[derive(Debug, Clone, Copy)]
pub struct HashNodeDecl {
    pub h: u64,
    pub text_list: StringIndex,
    pub next_in_bucket: HashNodeIndex,
}

pub const N_BUCKETS: usize = 256;

// Line type enumeration
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineType {
    SytType = 1,
    UniqueType = 2,
    MatchType = 3,
}

// File line declaration
#[derive(Debug, Clone, Copy)]
pub struct FileLineDecl {
    pub ptr0: LineCount,
    pub file_line_text: StringIndex,
    pub linen: LineCount,
    pub ptr_type: LineType,
}

// Line kinds for statistics
#[derive(Debug, Clone, Copy, Default)]
pub struct LineKinds {
    pub cosmetic: LineCount,
    pub non_cosmetic: LineCount,
}

// Tree node index
pub type TreeIndex = usize;

pub const NULL_NODE: TreeIndex = 0;

// Node declaration for trees
#[derive(Debug, Clone, Copy)]
pub struct NodeDecl {
    pub cost: LineCount,
    pub linen: LineCount,
    pub prev: TreeIndex,
    pub next: TreeIndex,
    pub branch_start: TreeIndex,
    pub branch_end: TreeIndex,
}

// Tree bounds
#[derive(Debug, Clone, Copy, Default)]
pub struct TreeBounds {
    pub start: TreeIndex,
    pub end: TreeIndex,
}

// Comparison result enum
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum CompareResult {
    Lt = 1,
    Eq = 2,
    Gt = 3,
}

// Helper functions
pub fn get_which_file(linen: LineCount) -> FileIndex {
    if linen < 0 {
        FileIndex::Second
    } else {
        FileIndex::First
    }
}

pub fn get_abs_line(linen: LineCount) -> LineCount {
    if linen < 0 {
        -linen
    } else {
        linen
    }
}

// Hash table state
#[derive(Debug)]
pub struct HashTableState {
    pub hash_node: Vec<HashNodeDecl>,
    pub sec_hash_start_node: [HashNodeIndex; N_BUCKETS],
}

impl Default for HashTableState {
    fn default() -> Self {
        Self {
            hash_node: Vec::new(),
            sec_hash_start_node: [NULL_HASH_LIST; N_BUCKETS],
        }
    }
}

impl HashTableState {
    pub fn clear(&mut self) {
        self.hash_node.clear();
        self.sec_hash_start_node = [NULL_HASH_LIST; N_BUCKETS];
    }
}

// File state - per-file line data
#[derive(Debug, Default)]
pub struct FileState {
    pub file_line: [Vec<FileLineDecl>; 2],
    pub total_file_n_lines: [usize; 2],
}

impl FileState {
    pub fn clear(&mut self) {
        self.file_line[0].clear();
        self.file_line[1].clear();
        self.file_line[0].push(FileLineDecl {
            ptr0: 0,
            file_line_text: NULL_STRING_LIST,
            linen: 0,
            ptr_type: LineType::SytType,
        });
        self.file_line[1].push(FileLineDecl {
            ptr0: 0,
            file_line_text: NULL_STRING_LIST,
            linen: 0,
            ptr_type: LineType::SytType,
        });
        self.total_file_n_lines = [0, 0];
    }
}

// Line matching state - tables for matching lines
#[derive(Debug, Default)]
pub struct LineMatchingState {
    pub line_table: Vec<LineTableDecl>,
    pub string_table: Vec<StringDecl>,
}

impl LineMatchingState {
    pub fn clear(&mut self) {
        self.line_table.clear();
        self.string_table.clear();
        // Add dummy entries at index 0 for 1-based indexing
        self.line_table.push(LineTableDecl {
            linen: 0,
            next: NULL_LINE_LIST,
        });
        self.string_table.push(StringDecl {
            text: String::new(),
            next_text_with_same_hash: NULL_STRING_LIST,
            file_n_lines: [0, 0],
            file_list: [NULL_LINE_LIST, NULL_LINE_LIST],
        });
    }
}

// Tree state - tree structure for passes 5-8
#[derive(Debug, Default)]
pub struct TreeState {
    pub node: Vec<NodeDecl>,
    pub trees: [TreeBounds; 2],
    pub free_nodes_start: TreeIndex,
}

impl TreeState {
    pub fn clear(&mut self) {
        self.node.clear();
        self.trees = [TreeBounds::default(), TreeBounds::default()];
        self.free_nodes_start = NULL_NODE;
    }
}

// Statistics - change tracking
#[derive(Debug, Clone, Copy, Default)]
pub struct Statistics {
    pub delete_stats: LineKinds,
    pub insert_stats: LineKinds,
    pub move_stats: LineKinds,
    pub replace1_stats: LineKinds,
    pub replace2_stats: LineKinds,
    pub n_change_blocks: usize,
}

impl Statistics {
    pub fn clear(&mut self) {
        *self = Statistics::default();
    }
}
