package main

import (
	"fmt"
)

// Insert a node after another node
func (i *Ifcomp) insertNodeAfter(afterThis, insertThis TreeIndex) {
	i.TreeState.Node[insertThis].Prev = afterThis
	afterAfter := i.TreeState.Node[afterThis].Next
	i.TreeState.Node[insertThis].Next = afterAfter
	i.TreeState.Node[afterAfter].Prev = insertThis
	i.TreeState.Node[afterThis].Next = insertThis
}

// Find the node with minimum cost in a range
func (i *Ifcomp) pass8MinCostNode(startNode, endNode TreeIndex) TreeIndex {
	minCost := i.TreeState.Node[startNode].Cost
	minNode := startNode
	N := startNode
	for N != endNode {
		if minCost > i.TreeState.Node[N].Cost {
			minCost = i.TreeState.Node[N].Cost
			minNode = N
		}
		N = i.TreeState.Node[N].Next
	}
	if i.DebugDumpTreesFull {
		i.printf("min_cost_node(%d,%d)=%d\n", startNode, endNode, minNode)
	}
	return minNode
}

// Process and output a move operation
func (i *Ifcomp) pass8MoveLines(node1, node2 TreeIndex) error {
	firstIdx := toArrayIndex(First)
	i.Stats.NChangeBlocks++
	i.countNode(node2, &i.Stats.MoveStats)
	if node1 == i.TreeState.Trees[firstIdx].Start {
		i.afterHeader(node1)
		i.printHeader1("MOVE LINE(s)")
		i.printNode(node2)
		i.printTrailer()
		i.detachNode(node2)
		i.insertNodeAfter(i.TreeState.Trees[firstIdx].Start, node2)
	} else {
		i.afterLines(node1)
		i.printHeader1("MOVE LINE(s)")
		i.printNode(node2)
		i.printTrailer()
		i.detachNode(node2)
		i.insertNodeAfter(node1, node2)
		// Combine adjacent nodes to redistribute weight for min_cost
		if err := i.pass7(); err != nil {
			return err
		}
	}
	return nil
}

// Pass 8: Move Detection and Processing
//
// Purpose: Detect and process moved code blocks by identifying misalignments
// between files and relocating segments to their correct positions.
func (i *Ifcomp) pass8() error {
	firstIdx := toArrayIndex(First)
	secondIdx := toArrayIndex(Second)

	// Safety check: prevent infinite loops
	iterationCount := 0
	const MAX_ITERATIONS = 10000

	for {
		iterationCount++
		if iterationCount > MAX_ITERATIONS {
			return fmt.Errorf("internal error in pass8: infinite loop detected after %d iterations", iterationCount)
		}

		nodeIdx := i.TreeState.Trees[firstIdx].Start
		nodeJ := i.TreeState.Trees[secondIdx].Start

		for nodeIdx != i.TreeState.Trees[firstIdx].End {
			// First time through, this skips the header
			nodeIdx = i.TreeState.Node[nodeIdx].Next
			nodeJ = i.TreeState.Node[nodeJ].Next

			// Scan through the two files while file1 references the same line in file2
			if i.DebugDumpTreesFull {
				i.printf("node %d lno %d -> %d, node %d lno %d\n",
					nodeIdx, i.trueLineOf(nodeIdx),
					i.FileState.FileLine[firstIdx][i.trueLineOf(nodeIdx)].Ptr0,
					nodeJ, i.trueLineOf(nodeJ))
			}

			for i.FileState.FileLine[firstIdx][i.trueLineOf(nodeIdx)].Ptr0 == i.trueLineOf(nodeJ) &&
				nodeIdx != i.TreeState.Trees[firstIdx].End {
				nodeIdx = i.TreeState.Node[nodeIdx].Next
				nodeJ = i.TreeState.Node[nodeJ].Next
			}

			if nodeIdx == i.TreeState.Trees[firstIdx].End {
				return nil
			}

			nodeK := i.pass8MinCostNode(nodeIdx, i.TreeState.Trees[firstIdx].End)
			findIdx := TreeIndex(i.FileState.FileLine[firstIdx][i.trueLineOf(nodeK)].Ptr0)
			nodeL := i.findNode(i.TreeState.Trees[secondIdx], findIdx)

			if nodeL == NullNode {
				return nil
			}

			nodeM := i.TreeState.Node[nodeL].Prev
			// m might be the header node with line 0
			findIdx2 := TreeIndex(i.FileState.FileLine[secondIdx][i.trueLineOf(nodeM)].Ptr0)
			nodeN := i.findNode(i.TreeState.Trees[firstIdx], findIdx2)

			if nodeN == NullNode {
				return nil
			}

			if err := i.pass8MoveLines(nodeN, nodeK); err != nil {
				return err
			}
			i.dumpTrees(99)
			// Restart from beginning
			break
		}
	}
}
