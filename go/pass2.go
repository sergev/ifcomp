package main

// Pass 2: Unique Pair Identification
//
// Purpose: Identify lines that appear exactly once in each file and match
// them as unique pairs, creating bidirectional links between corresponding
// lines. These unique pairs serve as anchors for subsequent passes.
//
// Essence: This pass scans the string table and finds strings that occur
// exactly once in both files. When such a pair is found, both lines are
// marked as UNIQUE_TYPE and linked via ptr0. Lines that appear multiple
// times in either file remain SYT_TYPE and will be processed by later
// passes.
func (i *Ifcomp) pass2() {
	for idx := 1; idx < len(i.LineMatchingState.StringTable); idx++ {
		// Look at each line. If it occurs once in both files, record both as unique.
		firstIdx := toArrayIndex(First)
		secondIdx := toArrayIndex(Second)
		if i.LineMatchingState.StringTable[idx].FileNLines[firstIdx] == 1 &&
			i.LineMatchingState.StringTable[idx].FileNLines[secondIdx] == 1 {
			// Found a unique pair
			fileLinen1 := i.LineMatchingState.LineTable[i.LineMatchingState.StringTable[idx].FileList[firstIdx]].Linen
			fileLinen2 := i.LineMatchingState.LineTable[i.LineMatchingState.StringTable[idx].FileList[secondIdx]].Linen
			// Make each line reference the occurrence in the other file
			i.FileState.FileLine[firstIdx][fileLinen1].PtrType = UNIQUE_TYPE
			i.FileState.FileLine[firstIdx][fileLinen1].Ptr0 = fileLinen2
			i.FileState.FileLine[secondIdx][fileLinen2].PtrType = UNIQUE_TYPE
			i.FileState.FileLine[secondIdx][fileLinen2].Ptr0 = fileLinen1
		}
	}
}
