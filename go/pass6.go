package main

import (
	"strings"
)

// Check if a node is a leaf
func (i *Ifcomp) leaf(n TreeIndex) bool {
	return i.TreeState.Node[n].BranchStart == NullNode
}

// Get absolute line number from node
func (i *Ifcomp) trueLineOf(N TreeIndex) LineCount {
	if i.TreeState.Node[N].Linen < 0 {
		return -i.TreeState.Node[N].Linen
	}
	return i.TreeState.Node[N].Linen
}

// Print formatted header
func (i *Ifcomp) printHeader(s string) {
	padding := 52
	out := s + " " + strings.Repeat("=", padding-len(s))
	i.printf("*** %s ***\n", out)
}

// Print formatted header1
func (i *Ifcomp) printHeader1(s string) {
	padding := 52
	out := s + " " + strings.Repeat("-", padding-len(s))
	i.printf("*** %s ***\n", out)
}

// Print formatted trailer
func (i *Ifcomp) printTrailer() {
	out := strings.Repeat("=", 53)
	i.printf("*** %s ***\n\n", out)
}

// Print lines in a node
func (i *Ifcomp) printNode(noden TreeIndex) {
	i.printNode1(noden, false, 0)
}

// Print lines in a node (with options)
func (i *Ifcomp) printNode1(noden TreeIndex, always bool, startingLine int) {
	i.eachLineInNode(noden, always, startingLine, func(whichFile FileIndex, text string, lineno int) {
		prefix := " "
		if whichFile == Second {
			prefix = "+"
		}
		i.printf("%s%6d|%s\n", prefix, lineno, text)
	})
}

// Iterate through all lines in a node
func (i *Ifcomp) eachLineInNode(noden TreeIndex, always bool, startingLine int, fn func(FileIndex, string, int)) {
	var start, finish TreeIndex
	if !i.leaf(noden) {
		start = i.TreeState.Node[noden].BranchStart
		finish = noden
	} else {
		start = noden
		finish = i.TreeState.Node[noden].Next
	}

	for current := start; current != finish; current = i.TreeState.Node[current].Next {
		sline := i.TreeState.Node[current].Linen
		fileno := getWhichFile(sline)
		sline = getAbsLine(sline)
		filenoIdx := toArrayIndex(fileno)

		// cost is the number of nodes. Can be negative.
		cost := int(i.TreeState.Node[current].Cost)
		if always {
			if cost < 0 {
				cost = -cost
			}
		}
		last := int(sline) + cost

		// He may have passed a place to start later than the beginning of a node.
		maxStart := int(sline)
		if startingLine > maxStart {
			maxStart = startingLine
		}
		for s := maxStart; s < last; s++ {
			fileno2 := FileIndex(fileno)
			text := i.LineMatchingState.StringTable[i.FileState.FileLine[filenoIdx][s].FileLineText].Text
			lineno := int(i.FileState.FileLine[filenoIdx][s].Linen)
			fn(fileno2, text, lineno)
		}
	}
}

// Count cosmetic and non-cosmetic lines in a node
func (i *Ifcomp) countNode(noden TreeIndex, p *LineKinds) {
	i.eachLineInNode(noden, false, 0, func(whichFile FileIndex, text string, lineno int) {
		// For now, no cosmetic lines (cosmetic_line always returns false in C++)
		p.NonCosmetic++
	})
}

// Remove a node from its linked list
func (i *Ifcomp) detachNode(noden TreeIndex) {
	prev := i.TreeState.Node[noden].Prev
	next := i.TreeState.Node[noden].Next
	i.TreeState.Node[prev].Next = next
	i.TreeState.Node[next].Prev = prev
}

// Find node in tree containing the specified line number
func (i *Ifcomp) findNode(T TreeBounds, linen TreeIndex) TreeIndex {
	absLinen := linen
	if linen < 0 {
		absLinen = -linen
	}
	N := T.Start
	for N != T.End {
		if i.trueLineOf(N) == LineCount(absLinen) {
			if i.DebugDumpTreesFull {
				i.printf("In tree %d:%d, find line %d at %d\n", T.Start, T.End, linen, N)
			}
			return N
		}
		N = i.TreeState.Node[N].Next
	}
	// Node not found
	if i.DebugDumpTreesFull {
		N = T.Start
		i.printf("[")
		for N != T.End {
			i.printf("%d ", N)
			N = i.TreeState.Node[N].Next
		}
		i.printf("] ln=%d\n", linen)
		i.printf("*** Warning: find_node could not find line %d in tree %d:%d\n", linen, T.Start, T.End)
	}
	return NullNode
}

// Check if an unmatched node in file1 can be replaced
func (i *Ifcomp) pass6Replaceable(noden TreeIndex) TreeIndex {
	firstIdx := toArrayIndex(First)
	secondIdx := toArrayIndex(Second)

	prev := i.TreeState.Node[noden].Prev
	prevOtherFile := i.findNode(i.TreeState.Trees[secondIdx], TreeIndex(i.FileState.FileLine[firstIdx][i.trueLineOf(prev)].Ptr0))

	if prevOtherFile == NullNode {
		return NullNode
	}

	nodenOtherFile := i.TreeState.Node[prevOtherFile].Next
	if i.TreeState.Node[nodenOtherFile].Cost >= 0 {
		if i.DebugDumpTreesFull {
			i.printf("replaceable fails: noden_other_file(%d) has neg cost.\n", nodenOtherFile)
		}
		return NullNode
	}
	return nodenOtherFile
}

// Process and output a deletion operation
func (i *Ifcomp) deleteLines(noden TreeIndex) {
	i.Stats.NChangeBlocks++
	i.afterHeader(i.TreeState.Node[noden].Prev)
	i.TreeState.Node[noden].Cost = -i.TreeState.Node[noden].Cost
	i.printHeader1("DELETE LINE(s)")
	i.printNode(noden)
	i.printTrailer()
	i.countNode(noden, &i.Stats.DeleteStats)
	i.detachNode(noden)
}

// Find first unique line in a node
func (i *Ifcomp) uniqueFind(noden TreeIndex) TreeIndex {
	endLine := i.TreeState.Node[noden].Linen
	filen := getWhichFile(endLine)
	endLine = getAbsLine(endLine)
	filenIdx := toArrayIndex(filen)

	cost := int(i.TreeState.Node[noden].Cost)
	for startLine := int(endLine) + cost - 1; startLine >= int(endLine); startLine-- {
		if i.FileState.FileLine[filenIdx][startLine].PtrType == UNIQUE_TYPE {
			return TreeIndex(startLine)
		}
	}
	return NullNode
}

// Print context lines before a change
func (i *Ifcomp) afterLines(noden TreeIndex) {
	i.printHeader("AFTER LINE(s)")
	firstIdx := toArrayIndex(First)
	start := noden
	last := i.TreeState.Node[start].Next
	var linen TreeIndex = 0

	for start != i.TreeState.Trees[firstIdx].Start {
		if i.leaf(start) {
			linen = i.uniqueFind(start)
			if linen != NullNode {
				break
			}
			linen = 0
			last = start
			start = i.TreeState.Node[start].Prev
		} else {
			if last == i.TreeState.Node[start].BranchStart {
				start = i.TreeState.Node[start].Prev
				last = start
			} else {
				last = start
				start = i.TreeState.Node[start].BranchEnd
			}
		}
	}

	i.printNode1(start, false, int(linen))
	last = start
	start = i.TreeState.Node[start].Next

	for start != i.TreeState.Node[noden].Next {
		if i.leaf(start) {
			i.printNode(start)
			last = start
			start = i.TreeState.Node[start].Next
		} else {
			if last == i.TreeState.Node[start].BranchEnd {
				last = start
				start = i.TreeState.Node[start].Next
			} else {
				last = start
				start = i.TreeState.Node[start].BranchStart
			}
		}
	}
}

// Print context header
func (i *Ifcomp) afterHeader(noden TreeIndex) {
	firstIdx := toArrayIndex(First)
	if noden == i.TreeState.Trees[firstIdx].Start {
		i.printHeader("AFTER TOP")
	} else {
		i.afterLines(noden)
	}
}

// Process and output a replacement operation
func (i *Ifcomp) pass6ReplaceLines(node1, node2 TreeIndex) {
	i.Stats.NChangeBlocks++
	// Make the costs positive, indicating that the nodes now
	// correspond to something in the other file.
	i.TreeState.Node[node1].Cost = -i.TreeState.Node[node1].Cost
	i.TreeState.Node[node2].Cost = -i.TreeState.Node[node2].Cost
	i.countNode(node1, &i.Stats.Replace1Stats)
	i.countNode(node2, &i.Stats.Replace2Stats)
	i.afterHeader(i.TreeState.Node[node1].Prev)
	i.printHeader1("REPLACE LINE(s)")
	i.printNode(node1)
	i.printHeader1("WITH LINE(s)")
	i.printNode(node2)
	i.printTrailer()
	i.detachNode(node1)
	i.detachNode(node2)
}

// Process and output an insertion operation
func (i *Ifcomp) pass6InsertLines(noden TreeIndex) {
	firstIdx := toArrayIndex(First)
	secondIdx := toArrayIndex(Second)
	i.Stats.NChangeBlocks++
	i.TreeState.Node[noden].Cost = -i.TreeState.Node[noden].Cost
	i.countNode(noden, &i.Stats.InsertStats)
	prev := i.TreeState.Node[noden].Prev
	if prev == i.TreeState.Trees[secondIdx].Start {
		i.detachNode(noden)
		i.TreeState.Node[i.TreeState.Trees[firstIdx].Start].BranchStart = noden
		i.TreeState.Node[i.TreeState.Trees[firstIdx].Start].BranchEnd = noden
		i.TreeState.Node[noden].Prev = i.TreeState.Trees[firstIdx].Start
		i.TreeState.Node[noden].Next = i.TreeState.Trees[firstIdx].Start
		i.printHeader("AFTER TOP")
		i.printHeader1("INSERT LINE(s)")
		i.printNode(i.TreeState.Trees[firstIdx].Start)
	} else {
		j := i.findNode(i.TreeState.Trees[firstIdx],
			TreeIndex(i.FileState.FileLine[secondIdx][i.trueLineOf(prev)].Ptr0))
		if j == NullNode {
			i.detachNode(noden)
			i.freeNode(noden)
			i.Stats.NChangeBlocks--
			return
		}
		i.afterLines(j)
		i.printHeader1("INSERT LINE(s)")
		i.printNode(noden)
		i.combineNodes(j, noden)
	}
	i.printTrailer()
}

// Dump trees for debugging
func (i *Ifcomp) dumpTrees(pass int) {
	if !i.DebugDumpTrees {
		return
	}
	if pass == 99 {
		i.println("dump trees")
	} else {
		i.printf("dump_trees after pass%d\n", pass)
	}
	i.dumpTree(i.TreeState.Trees[0].Start)
	i.dumpTree(i.TreeState.Trees[1].Start)
}

// Dump tree structure for debugging
func (i *Ifcomp) dumpTree(treeStart TreeIndex) {
	i.printf("Tree %d:\n", treeStart)
	branch := false
	T := treeStart
	for T != NullNode {
		T2 := T
		if i.leaf(T) {
			pad := 0
			if branch {
				pad = 1
			}
			i.formatNode(T, pad)
			T = i.TreeState.Node[T].Next
			if i.DebugDumpTreesFull {
				i.printNode1(T2, true, 0)
			}
		} else {
			if branch {
				branch = false
				T = i.TreeState.Node[T].Next
			} else {
				i.formatNode(T, 0)
				T = i.TreeState.Node[T].BranchStart
				branch = true
			}
		}
	}
}

// Format and print node information
func (i *Ifcomp) formatNode(noden TreeIndex, pad int) {
	i.printf("%s[%d<-N%d->%d, cost=%2d linen=%2d",
		strings.Repeat(" ", pad*7), i.TreeState.Node[noden].Prev, noden,
		i.TreeState.Node[noden].Next, i.TreeState.Node[noden].Cost, i.TreeState.Node[noden].Linen)

	L := i.TreeState.Node[noden].Linen
	fileno := getWhichFile(L)
	L = getAbsLine(L)
	filenoIdx := toArrayIndex(fileno)

	// Check bounds before accessing FileLine - end nodes may have Linen beyond file bounds
	if int(L) < len(i.FileState.FileLine[filenoIdx]) {
		i.printf("(%d)", i.FileState.FileLine[filenoIdx][L].Ptr0)
	} else {
		i.printf("(end)")
	}

	if i.TreeState.Node[noden].BranchStart != NullNode || i.TreeState.Node[noden].BranchEnd != NullNode {
		i.printf(" bs=%2d be=%2d", i.TreeState.Node[noden].BranchStart, i.TreeState.Node[noden].BranchEnd)
	}
	i.printf("]\n")
}

// Make a new tree node
func (i *Ifcomp) makeNode(p NodeDecl) TreeIndex {
	i.TreeState.Node = append(i.TreeState.Node, p)
	idx := TreeIndex(len(i.TreeState.Node) - 1)
	if i.DebugDumpTreesFull {
		i.printf("just made ")
		i.formatNode(idx, 0)
	}
	return idx
}

// Pass 6: Replace/Delete/Insert operations
func (i *Ifcomp) pass6() {
	firstIdx := toArrayIndex(First)
	for j := i.TreeState.Node[i.TreeState.Trees[firstIdx].Start].Next; j != i.TreeState.Trees[firstIdx].End; {
		k := j
		j = i.TreeState.Node[j].Next
		if i.TreeState.Node[k].Cost < 0 {
			// Try to replace first
			l := i.pass6Replaceable(k)
			if l != NullNode {
				i.pass6ReplaceLines(k, l)
			} else {
				i.deleteLines(k)
			}
		}
	}

	secondIdx := toArrayIndex(Second)
	for j := i.TreeState.Node[i.TreeState.Trees[secondIdx].Start].Next; j != i.TreeState.Trees[secondIdx].End; {
		k := j
		j = i.TreeState.Node[j].Next
		if i.TreeState.Node[k].Cost < 0 {
			i.pass6InsertLines(k)
		}
	}
}
