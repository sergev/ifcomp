package main

import "fmt"

// Build tree structure for a single file
func (i *Ifcomp) pass5Doit(fileno FileIndex, Np *NodeDecl) {
	filenoIdx := toArrayIndex(fileno)
	if i.DebugDumpTrees {
		fmt.Printf("Make tree for file %d\n", filenoIdx+1)
	}

	lineCount := 1
	for lineCount <= i.FileState.TotalFileNLines[filenoIdx] {
		Np.Linen = LineCount(lineCount)
		ptrType := i.FileState.FileLine[filenoIdx][lineCount].PtrType

		if ptrType == SYT_TYPE {
			// Determine a block of syt_type lines
			for lineCount+1 <= i.FileState.TotalFileNLines[filenoIdx] &&
				i.FileState.FileLine[filenoIdx][lineCount+1].PtrType == SYT_TYPE {
				lineCount++
			}
			lineCount++
			Np.Cost = LineCount(lineCount) - Np.Linen
			Np.Cost = -Np.Cost // Signifies delete
		} else {
			// Determine a block of non-syt_type lines
			ptr0 := i.FileState.FileLine[filenoIdx][lineCount].Ptr0
			expPtr0 := ptr0 + 1
			for lineCount+1 <= i.FileState.TotalFileNLines[filenoIdx] &&
				i.FileState.FileLine[filenoIdx][lineCount+1].PtrType != SYT_TYPE &&
				i.FileState.FileLine[filenoIdx][lineCount+1].Ptr0 == expPtr0 {
				lineCount++
				expPtr0++
			}
			lineCount++
			Np.Cost = LineCount(lineCount) - Np.Linen
		}

		if fileno == Second {
			Np.Linen = -Np.Linen
		}

		j := i.makeNode(*Np)
		i.TreeState.Node[Np.Prev].Next = j
		Np.Prev = j
	}
}

// Pass 5: Tree Construction
//
// Purpose: Build initial tree structures representing file segments (matched
// and unmatched). Converts the linear file representation into a tree-based
// structure that groups consecutive lines into segments for efficient change
// detection.
func (i *Ifcomp) pass5() {
	firstIdx := toArrayIndex(First)
	secondIdx := toArrayIndex(Second)

	// Ensure file_line arrays have at least index 0
	if len(i.FileState.FileLine[firstIdx]) == 0 {
		i.FileState.FileLine[firstIdx] = append(i.FileState.FileLine[firstIdx], FileLineDecl{})
	}
	if len(i.FileState.FileLine[secondIdx]) == 0 {
		i.FileState.FileLine[secondIdx] = append(i.FileState.FileLine[secondIdx], FileLineDecl{})
	}

	// Add dummy entry at index 0 for 1-based indexing
	if len(i.TreeState.Node) == 0 {
		i.TreeState.Node = append(i.TreeState.Node, NodeDecl{})
	}

	N := NodeDecl{
		Cost:        0,
		Linen:       0,
		Next:        NullNode,
		Prev:        NullNode,
		BranchStart: NullNode,
		BranchEnd:   NullNode,
	}

	// Make header nodes
	i.TreeState.Trees[firstIdx].Start = i.makeNode(N)
	i.TreeState.Trees[secondIdx].Start = i.makeNode(N)
	N.Prev = i.TreeState.Trees[firstIdx].Start

	i.pass5Doit(First, &N)

	N.Cost = 0
	file1TLinesP := i.FileState.TotalFileNLines[firstIdx] + 1
	N.Linen = LineCount(file1TLinesP)
	i.TreeState.Trees[firstIdx].End = i.makeNode(N)
	i.TreeState.Node[N.Prev].Next = i.TreeState.Trees[firstIdx].End

	N.Prev = i.TreeState.Trees[secondIdx].Start
	i.pass5Doit(Second, &N)

	N.Cost = 0
	file2TLinesP := i.FileState.TotalFileNLines[secondIdx] + 1
	N.Linen = -LineCount(file2TLinesP)
	i.TreeState.Trees[secondIdx].End = i.makeNode(N)
	i.TreeState.Node[N.Prev].Next = i.TreeState.Trees[secondIdx].End

	// Now be sure that the header records can refer to each other
	for len(i.FileState.FileLine[firstIdx]) <= file1TLinesP {
		i.FileState.FileLine[firstIdx] = append(i.FileState.FileLine[firstIdx], FileLineDecl{})
	}
	for len(i.FileState.FileLine[secondIdx]) <= file2TLinesP {
		i.FileState.FileLine[secondIdx] = append(i.FileState.FileLine[secondIdx], FileLineDecl{})
	}

	i.FileState.FileLine[firstIdx][0].Ptr0 = 0
	i.FileState.FileLine[secondIdx][0].Ptr0 = 0

	// Also make the trailers talk to each other
	i.FileState.FileLine[firstIdx][file1TLinesP].Ptr0 = LineCount(file2TLinesP)
	i.FileState.FileLine[secondIdx][file2TLinesP].Ptr0 = LineCount(file1TLinesP)
}
