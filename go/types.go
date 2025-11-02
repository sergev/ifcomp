package main

// File index enumeration - strong type for file identification
type FileIndex int

const (
	First  FileIndex = 0
	Second FileIndex = 1
)

func otherFile(f FileIndex) FileIndex {
	if f == First {
		return Second
	}
	return First
}

func toArrayIndex(f FileIndex) int {
	return int(f)
}

// Line count type
type LineCount int

// String index - one per distinct line
type StringIndex int

const NullStringList StringIndex = 0

// Hash node index
type HashNodeIndex int16

const NullHashList HashNodeIndex = 0

// Line table entry (for linked list of line numbers)
type LineTableDecl struct {
	Linen LineCount
	Next  LineCount
}

const NullLineList LineCount = 0

// String declaration - records a unique line
type StringDecl struct {
	Text                 string
	NextTextWithSameHash StringIndex
	FileNLines           [2]int
	FileList             [2]LineCount
}

// Hash node declaration
type HashNodeDecl struct {
	H            uint64
	TextList     StringIndex
	NextInBucket HashNodeIndex
}

const NBuckets = 256

// Line type enumeration
type LineType int

const (
	SYT_TYPE    LineType = 1
	UNIQUE_TYPE LineType = 2
	MATCH_TYPE  LineType = 3
)

// File line declaration
type FileLineDecl struct {
	Ptr0         LineCount
	FileLineText StringIndex
	Linen        LineCount
	PtrType      LineType
}

// Line kinds for statistics
type LineKinds struct {
	Cosmetic    LineCount
	NonCosmetic LineCount
}

// Tree node index
type TreeIndex int

const NullNode TreeIndex = 0

// Node declaration for trees
type NodeDecl struct {
	Cost        LineCount
	Linen       LineCount
	Prev        TreeIndex
	Next        TreeIndex
	BranchStart TreeIndex
	BranchEnd   TreeIndex
}

// Tree bounds
type TreeBounds struct {
	Start TreeIndex
	End   TreeIndex
}

// Comparison result enum
type CompareResult int

const (
	LT CompareResult = 1
	EQ CompareResult = 2
	GT CompareResult = 3
)

// Helper functions
func getWhichFile(linen LineCount) FileIndex {
	if linen < 0 {
		return Second
	}
	return First
}

func getAbsLine(linen LineCount) LineCount {
	if linen < 0 {
		return -linen
	}
	return linen
}

// Hash table state
type HashTableState struct {
	HashNode         []HashNodeDecl
	SecHashStartNode [NBuckets]HashNodeIndex
}

// File state - per-file line data
type FileState struct {
	FileLine        [2][]FileLineDecl
	TotalFileNLines [2]int
}

// Line matching state - tables for matching lines
type LineMatchingState struct {
	LineTable   []LineTableDecl
	StringTable []StringDecl
}

// Tree state - tree structure for passes 5-8
type TreeState struct {
	Node           []NodeDecl
	Trees          [2]TreeBounds
	FreeNodesStart TreeIndex
}

// Statistics - change tracking
type Statistics struct {
	DeleteStats   LineKinds
	InsertStats   LineKinds
	MoveStats     LineKinds
	Replace1Stats LineKinds
	Replace2Stats LineKinds
	NChangeBlocks int
}

// Ifcomp main structure
type Ifcomp struct {
	HashState         HashTableState
	FileState         FileState
	LineMatchingState LineMatchingState
	TreeState         TreeState
	Stats             Statistics

	// Debug flags
	DebugDontFree        bool
	DebugSytFull         bool
	DebugSyt             bool
	DebugDumpTrees       bool
	DebugDumpTreesFull   bool
	DebugAlloc           bool
	DebugReadCurrentLine bool
}
