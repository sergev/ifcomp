package main

import (
	"flag"
	"fmt"
	"os"
)

func help(progName string) {
	fmt.Printf("Usage is: %s file1 file2\n", progName)
	os.Exit(1)
}

func main() {
	var firstFname, secondFname string
	var statistics bool

	// Parse flags
	flag.BoolVar(&statistics, "stat", false, "Print detailed memory usage statistics")
	debug := flag.Bool("debug", false, "Enable all debug output modes")
	st := flag.Bool("st", false, "Enable symbol table debugging")
	stfull := flag.Bool("stfull", false, "Enable full symbol table debugging")
	trees := flag.Bool("trees", false, "Enable tree structure debugging")
	treesfull := flag.Bool("treesfull", false, "Enable full tree structure debugging")
	alloc := flag.Bool("alloc", false, "Enable memory allocation debugging")
	nofree := flag.Bool("nofree", false, "Disable memory freeing (for debugging)")

	flag.Parse()
	args := flag.Args()

	if len(args) != 2 {
		help(os.Args[0])
	}

	firstFname = args[0]
	secondFname = args[1]

	// Create Ifcomp instance
	ifc := NewIfcomp()

	// Set debug flags
	if *debug {
		ifc.DebugSytFull = true
		ifc.DebugSyt = true
		ifc.DebugDumpTrees = true
		ifc.DebugDumpTreesFull = true
	}
	if *stfull {
		ifc.DebugSytFull = true
	}
	if *st {
		ifc.DebugSyt = true
	}
	if *trees {
		ifc.DebugDumpTrees = true
	}
	if *treesfull {
		ifc.DebugDumpTreesFull = true
	}
	if *alloc {
		ifc.DebugAlloc = true
	}
	if *nofree {
		ifc.DebugDontFree = true
	}

	fmt.Printf("Comparing: %s %s\n\n", firstFname, secondFname)

	if err := ifc.Compare(firstFname, secondFname); err != nil {
		fmt.Fprintf(os.Stderr, "Error: %v\n", err)
		os.Exit(1)
	}

	if statistics {
		fmt.Println("\nStatistics:")
		ifc.PrintStatistics()
	}
}

// NewIfcomp creates a new Ifcomp instance with initialized data structures
func NewIfcomp() *Ifcomp {
	ifc := &Ifcomp{}
	ifc.output = &outputWriter{out: os.Stdout}
	ifc.InitializeTables()
	return ifc
}

// SetOutput sets a custom output writer for ifcomp
func (i *Ifcomp) SetOutput(out interface {
	Write([]byte) (int, error)
	WriteString(string) (int, error)
}) {
	i.output = &outputWriter{out: out}
}

// Output helpers that redirect to the output writer
func (i *Ifcomp) printf(format string, args ...interface{}) {
	fmt.Fprintf(i.output, format, args...)
}

func (i *Ifcomp) println(args ...interface{}) {
	fmt.Fprintln(i.output, args...)
}

func (i *Ifcomp) print(args ...interface{}) {
	fmt.Fprint(i.output, args...)
}

// InitializeTables initializes data structure tables with dummy entries
func (i *Ifcomp) InitializeTables() {
	// Initialize hash table with empty buckets
	i.HashState.SecHashStartNode = [NBuckets]HashNodeIndex{NullHashList}

	// Initialize file_line arrays with index 0 entry
	i.FileState.FileLine[0] = append(i.FileState.FileLine[0], FileLineDecl{})
	i.FileState.FileLine[1] = append(i.FileState.FileLine[1], FileLineDecl{})

	// Add dummy entries at index 0 to match legacy 1-based indexing
	i.LineMatchingState.LineTable = append(i.LineMatchingState.LineTable, LineTableDecl{})
	i.LineMatchingState.StringTable = append(i.LineMatchingState.StringTable, StringDecl{})
	i.HashState.HashNode = append(i.HashState.HashNode, HashNodeDecl{})
}

// Compare is the main comparison function
func (i *Ifcomp) Compare(firstFname, secondFname string) error {
	// Clear all state
	i.HashState.Clear()
	i.FileState.Clear()
	i.LineMatchingState.Clear()
	i.TreeState.Clear()
	i.Stats.Clear()

	// Reinitialize dummy entries
	i.LineMatchingState.LineTable = append(i.LineMatchingState.LineTable, LineTableDecl{})
	i.LineMatchingState.StringTable = append(i.LineMatchingState.StringTable, StringDecl{})
	i.HashState.HashNode = append(i.HashState.HashNode, HashNodeDecl{})

	// Open input files
	file1, err := openFile(firstFname)
	if err != nil {
		return err
	}
	defer file1.Close()

	file2, err := openFile(secondFname)
	if err != nil {
		return err
	}
	defer file2.Close()

	// Execute passes 1-4
	if err := i.pass1(file1, file2); err != nil {
		return err
	}
	if i.DebugSyt {
		i.testList(1)
	}

	i.pass2()
	if i.DebugSyt {
		i.testList(2)
	}

	i.pass3()
	if i.DebugSyt {
		i.testList(3)
	}

	i.pass4()
	if i.DebugSyt {
		i.testList(4)
	}

	// Execute passes 5-8
	i.pass5()
	i.dumpTrees(5)

	i.pass6()
	i.dumpTrees(6)

	if err := i.pass7(); err != nil {
		return err
	}
	i.dumpTrees(7)

	if err := i.pass8(); err != nil {
		return err
	}
	i.dumpTrees(8)

	i.summary()
	return nil
}

// Summary prints summary statistics
func (i *Ifcomp) summary() {
	i.printf("%8d lines deleted from old.\n", i.Stats.DeleteStats.NonCosmetic)
	i.printf("%8d lines inserted in new.\n", i.Stats.InsertStats.NonCosmetic)
	i.printf("%8d lines deleted from old and replaced with %d lines of new.\n",
		i.Stats.Replace1Stats.NonCosmetic, i.Stats.Replace2Stats.NonCosmetic)
	i.printf("%8d lines moved in old.\n", i.Stats.MoveStats.NonCosmetic)
	i.printf("%8d change blocks.\n", i.Stats.NChangeBlocks)
}

// PrintStatistics prints detailed memory usage statistics
func (i *Ifcomp) PrintStatistics() {
	var memUsed int64

	// string_table
	stringSize := int64(len(i.LineMatchingState.StringTable)) * int64(32) // Approximate
	i.printf("%8d (%d max, %d bytes) string entries used.\n",
		len(i.LineMatchingState.StringTable), len(i.LineMatchingState.StringTable), stringSize)
	memUsed += stringSize

	// line_table
	lineSize := int64(len(i.LineMatchingState.LineTable)) * int64(16) // Approximate
	i.printf("%8d (%d max, %d bytes) line_table entries used.\n",
		len(i.LineMatchingState.LineTable), len(i.LineMatchingState.LineTable), lineSize)
	memUsed += lineSize

	// file_line[FIRST_FILE]
	file0Size := int64(len(i.FileState.FileLine[0])) * int64(24) // Approximate
	i.printf("%8d (%d max, %d bytes) file_line[FIRST_FILE] entries used.\n",
		len(i.FileState.FileLine[0]), len(i.FileState.FileLine[0]), file0Size)
	memUsed += file0Size

	// file_line[SECOND_FILE]
	file1Size := int64(len(i.FileState.FileLine[1])) * int64(24) // Approximate
	i.printf("%8d (%d max, %d bytes) file_line[SECOND_FILE] entries used.\n",
		len(i.FileState.FileLine[1]), len(i.FileState.FileLine[1]), file1Size)
	memUsed += file1Size

	i.println("\t\thash_node space was freed before allocating nodes:")

	// node
	nodeSize := int64(len(i.TreeState.Node)) * int64(32) // Approximate
	i.printf("%8d (%d max, %d bytes) node entries used.\n",
		len(i.TreeState.Node), len(i.TreeState.Node), nodeSize)
	memUsed += nodeSize

	// Calculate string bytes
	var stringBytes int64
	for _, str := range i.LineMatchingState.StringTable {
		stringBytes += int64(len(str.Text))
	}
	i.printf("%8d bytes of line texts.\n", stringBytes)
	memUsed += stringBytes
	i.printf("%8d total bytes of memory used.\n", memUsed)
}

// Clear methods for state structures
func (h *HashTableState) Clear() {
	h.HashNode = nil
	h.SecHashStartNode = [NBuckets]HashNodeIndex{NullHashList}
}

func (f *FileState) Clear() {
	f.FileLine[0] = nil
	f.FileLine[1] = nil
	f.FileLine[0] = append(f.FileLine[0], FileLineDecl{})
	f.FileLine[1] = append(f.FileLine[1], FileLineDecl{})
	f.TotalFileNLines[0] = 0
	f.TotalFileNLines[1] = 0
}

func (l *LineMatchingState) Clear() {
	l.LineTable = nil
	l.StringTable = nil
	// Add dummy entries at index 0 for 1-based indexing
	l.LineTable = append(l.LineTable, LineTableDecl{})
	l.StringTable = append(l.StringTable, StringDecl{})
}

func (t *TreeState) Clear() {
	t.Node = nil
	t.Trees[0] = TreeBounds{}
	t.Trees[1] = TreeBounds{}
	t.FreeNodesStart = NullNode
}

func (s *Statistics) Clear() {
	s.DeleteStats = LineKinds{}
	s.InsertStats = LineKinds{}
	s.MoveStats = LineKinds{}
	s.Replace1Stats = LineKinds{}
	s.Replace2Stats = LineKinds{}
	s.NChangeBlocks = 0
}

// TestList prints test listing of all file lines after a pass (debug function)
func (i *Ifcomp) testList(pass int) {
	maxLines := i.FileState.TotalFileNLines[0]
	if i.FileState.TotalFileNLines[1] > maxLines {
		maxLines = i.FileState.TotalFileNLines[1]
	}

	i.printf("test list after pass%d\n", pass)
	for j := 1; j <= maxLines; j++ {
		if j > i.FileState.TotalFileNLines[0] {
			i.println("=============")
		} else {
			i.formatFileLine(i.FileState.FileLine[0][j])
		}
		if j <= i.FileState.TotalFileNLines[1] {
			i.formatFileLine(i.FileState.FileLine[1][j])
		}
	}
	i.println()
}

// FormatFileLine formats and prints a file line entry
func (i *Ifcomp) formatFileLine(p FileLineDecl) {
	i.printf("|%3d|", p.Linen)
	switch p.PtrType {
	case SYT_TYPE:
		i.print("S      ")
	case UNIQUE_TYPE:
		i.printf("U%5d", p.Ptr0)
	case MATCH_TYPE:
		i.printf("M%5d", p.Ptr0)
	default:
		i.print("??????")
	}
	i.printf("|%s|\n", i.LineMatchingState.StringTable[p.FileLineText].Text)
}
