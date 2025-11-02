package main

// Pass 3: Forward Match Extension
//
// Purpose: Extend matches forward from unique pairs by checking if subsequent
// lines match. Expands matched regions by following unique anchors and matching
// consecutive lines that are still SYT_TYPE.
//
// Essence: This pass scans file1 sequentially, and when it encounters a
// UNIQUE_TYPE line, it extends the match forward by checking if the next
// lines in both files are SYT_TYPE and have matching text. Extension continues
// as long as lines match consecutively.
func (i *Ifcomp) pass3() {
	firstIdx := toArrayIndex(First)
	secondIdx := toArrayIndex(Second)
	m := LineCount(1)
	for m <= LineCount(i.FileState.TotalFileNLines[firstIdx]) {
		if i.FileState.FileLine[firstIdx][m].PtrType == UNIQUE_TYPE {
			n := i.FileState.FileLine[firstIdx][m].Ptr0 // Location in file 2
			// Broaden matches. Look for lines that follow unique_type
			// lines and which are not marked unique. If corresponding
			// lines match mark them match_type.
			for m, n = m+1, n+1; m <= LineCount(i.FileState.TotalFileNLines[firstIdx]) &&
				n <= LineCount(i.FileState.TotalFileNLines[secondIdx]) &&
				i.FileState.FileLine[firstIdx][m].PtrType == SYT_TYPE &&
				i.FileState.FileLine[firstIdx][m].FileLineText == i.FileState.FileLine[secondIdx][n].FileLineText; m, n = m+1, n+1 {
				i.FileState.FileLine[firstIdx][m].PtrType = MATCH_TYPE
				i.FileState.FileLine[firstIdx][m].Ptr0 = n
				i.FileState.FileLine[secondIdx][n].PtrType = MATCH_TYPE
				i.FileState.FileLine[secondIdx][n].Ptr0 = m
			}
		} else {
			m++
		}
	}
}
