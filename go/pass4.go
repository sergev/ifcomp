package main

// Pass 4: Backward Match Extension
//
// Purpose: Extend matches backward from unique pairs by checking if previous
// lines match. Complements Pass 3 by building matched regions in the reverse
// direction from unique anchors.
//
// Essence: This pass scans file1 backward, and when it encounters a
// UNIQUE_TYPE line, it extends the match backward by checking if the previous
// lines in both files are SYT_TYPE and have matching text.
func (i *Ifcomp) pass4() {
	firstIdx := toArrayIndex(First)
	secondIdx := toArrayIndex(Second)
	m := LineCount(i.FileState.TotalFileNLines[firstIdx])
	for m > 0 {
		if i.FileState.FileLine[firstIdx][m].PtrType == UNIQUE_TYPE {
			// Broaden matches in the backwards direction
			n := i.FileState.FileLine[firstIdx][m].Ptr0
			for m, n = m-1, n-1; m > 0 &&
				i.FileState.FileLine[firstIdx][m].PtrType == SYT_TYPE &&
				i.FileState.FileLine[secondIdx][n].PtrType == SYT_TYPE &&
				i.FileState.FileLine[firstIdx][m].FileLineText == i.FileState.FileLine[secondIdx][n].FileLineText; m, n = m-1, n-1 {
				i.FileState.FileLine[firstIdx][m].PtrType = MATCH_TYPE
				i.FileState.FileLine[firstIdx][m].Ptr0 = n
				i.FileState.FileLine[secondIdx][n].PtrType = MATCH_TYPE
				i.FileState.FileLine[secondIdx][n].Ptr0 = m
			}
		} else {
			m--
		}
	}
}
