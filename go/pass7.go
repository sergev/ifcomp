package main

import (
	"fmt"
)

// Check if two adjacent nodes are also adjacent in the other file
func (i *Ifcomp) pass7CombineAdjacentNodes(node1 TreeIndex) bool {
	firstIdx := toArrayIndex(First)
	secondIdx := toArrayIndex(Second)

	node2 := i.TreeState.Node[node1].Next

	if node2 == i.TreeState.Trees[firstIdx].End {
		return false
	}

	if i.DebugDumpTreesFull {
		i.printf("combine node1=%d ln=%d to node2=%d ln=%d\n",
			node1, i.TreeState.Node[node1].Linen, node2, i.TreeState.Node[node2].Linen)
	}

	findIdx := TreeIndex(i.FileState.FileLine[firstIdx][i.trueLineOf(node1)].Ptr0)
	findNodeI := i.findNode(i.TreeState.Trees[secondIdx], findIdx)

	findIdx2 := TreeIndex(i.FileState.FileLine[firstIdx][i.trueLineOf(node2)].Ptr0)
	findNodeJ := i.findNode(i.TreeState.Trees[secondIdx], findIdx2)

	if findNodeI == NullNode || findNodeJ == NullNode {
		return false
	}

	if findNodeJ == i.TreeState.Node[findNodeI].Next {
		i.combineNodes(node1, node2)
		i.combineNodes(findNodeI, findNodeJ)
		return true
	}
	return false
}

// Combine two adjacent nodes into a branch structure
func (i *Ifcomp) combineNodes(node1, node2 TreeIndex) {
	var branchLink1, branchLink2 TreeIndex
	N := NodeDecl{}
	N.Cost = i.TreeState.Node[node1].Cost + i.TreeState.Node[node2].Cost
	N.Linen = i.TreeState.Node[node1].Linen

	// First remove node2 from file2
	i.detachNode(node2)
	N.Prev = i.TreeState.Node[node1].Prev
	N.Next = i.TreeState.Node[node1].Next

	// Now remove node1 from file1
	i.detachNode(node1)

	if !i.leaf(node1) {
		// Just want the branch
		N.BranchStart = i.TreeState.Node[node1].BranchStart
		branchLink1 = i.TreeState.Node[node1].BranchEnd
		// The sequence in node1 is absorbed in N and hence isn't needed
		i.freeNode(node1)
		node1 = N.BranchStart
	} else {
		N.BranchStart = node1
		branchLink1 = node1
	}

	if !i.leaf(node2) {
		branchLink2 = i.TreeState.Node[node2].BranchStart
		N.BranchEnd = i.TreeState.Node[node2].BranchEnd
		// The sequence in node2 is absorbed in N and hence isn't needed
		i.freeNode(node2)
		node2 = branchLink2
	} else {
		branchLink2 = node2
		N.BranchEnd = node2
	}

	newNode := i.makeNode(N)
	// Insert new_node after N.prev and before N.next; i.e., it replaces node1
	i.TreeState.Node[N.Prev].Next = newNode
	i.TreeState.Node[N.Next].Prev = newNode
	i.TreeState.Node[N.BranchStart].Prev = newNode
	i.TreeState.Node[N.BranchEnd].Next = newNode
	i.TreeState.Node[branchLink1].Next = branchLink2
	i.TreeState.Node[branchLink2].Prev = branchLink1
}

// Free a node
func (i *Ifcomp) freeNode(n TreeIndex) {
	if i.DebugDontFree {
		return
	}
	i.TreeState.Node[n].Next = i.TreeState.FreeNodesStart
	i.TreeState.FreeNodesStart = n
}

// Pass 7: Combine Adjacent Nodes
//
// Purpose: Merge adjacent nodes that are also adjacent in the other file,
// reducing tree complexity and creating larger matched segments.
func (i *Ifcomp) pass7() error {
	firstIdx := toArrayIndex(First)
	nodeIdx := i.TreeState.Node[i.TreeState.Trees[firstIdx].Start].Next
	endIdx := i.TreeState.Trees[firstIdx].End

	// Safety check: prevent infinite loops
	iterationCount := 0
	const MAX_ITERATIONS = 10000

	// Loop until we reach the end marker
	// Check both: nodeIdx itself and node[nodeIdx].next
	for nodeIdx != endIdx && i.TreeState.Node[nodeIdx].Next != endIdx {
		iterationCount++
		if iterationCount > MAX_ITERATIONS {
			return fmt.Errorf("internal error in pass7: infinite loop detected at node %d after %d iterations",
				nodeIdx, iterationCount)
		}

		// Safety check: if we've reached an invalid node (header or null), exit
		if nodeIdx == i.TreeState.Trees[firstIdx].Start || nodeIdx == NullNode || nodeIdx == 0 {
			break
		}

		j := i.TreeState.Node[nodeIdx].Prev
		if i.pass7CombineAdjacentNodes(nodeIdx) {
			// After combination, nodeIdx (node1) may have been freed/reused
			// Use j (prev) to get the new combined node
			nodeIdx = i.TreeState.Node[j].Next
		} else {
			// Advance to next node, but ensure we don't go to an invalid node
			nextNode := i.TreeState.Node[nodeIdx].Next
			if nextNode == NullNode || nextNode == 0 || nextNode == i.TreeState.Trees[firstIdx].Start {
				break
			}
			nodeIdx = nextNode
		}
	}

	return nil
}
