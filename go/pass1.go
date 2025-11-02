package main

import (
	"bufio"
	"fmt"
	"hash/fnv"
	"io"
	"os"
)

// Hash a line string using FNV hash
func (i *Ifcomp) hashLine(line string) uint64 {
	h := fnv.New64a()
	h.Write([]byte(line))
	return h.Sum64()
}

// Compare two hash codes
func hashcodeCompare(ha, hb uint64) CompareResult {
	if ha < hb {
		return LT
	}
	if ha > hb {
		return GT
	}
	return EQ
}

// Create a new entry in the line table
func (i *Ifcomp) makeLineEntry(linen LineCount, next LineCount) LineCount {
	i.LineMatchingState.LineTable = append(i.LineMatchingState.LineTable, LineTableDecl{
		Linen: linen,
		Next:  next,
	})
	return LineCount(len(i.LineMatchingState.LineTable) - 1)
}

// Create a new string entry in the string table
func (i *Ifcomp) setupDistinctText(text string, linen LineCount, inputFile FileIndex) StringIndex {
	s := StringDecl{}
	other := otherFile(inputFile)
	inputIdx := toArrayIndex(inputFile)
	otherIdx := toArrayIndex(other)
	s.FileNLines[inputIdx] = 1
	s.FileNLines[otherIdx] = 0
	s.FileList[inputIdx] = i.makeLineEntry(linen, NullLineList)
	s.FileList[otherIdx] = NullLineList
	s.NextTextWithSameHash = NullStringList
	s.Text = text

	i.LineMatchingState.StringTable = append(i.LineMatchingState.StringTable, s)
	return StringIndex(len(i.LineMatchingState.StringTable) - 1)
}

// Create a new hash node entry
func (i *Ifcomp) setupHashNode(tip *StringIndex, text string, linen LineCount, inputFile FileIndex, h uint64) HashNodeIndex {
	s := HashNodeDecl{}
	s.NextInBucket = NullHashList
	s.TextList = i.setupDistinctText(text, linen, inputFile)
	*tip = s.TextList
	s.H = h

	i.HashState.HashNode = append(i.HashState.HashNode, s)
	return HashNodeIndex(len(i.HashState.HashNode) - 1)
}

// Add a line number occurrence to an existing text string's file list
func (i *Ifcomp) addLinenToTextList(t StringIndex, linen LineCount, inputFile FileIndex) {
	inputIdx := toArrayIndex(inputFile)
	i.LineMatchingState.StringTable[t].FileNLines[inputIdx]++
	p := i.LineMatchingState.StringTable[t].FileList[inputIdx]
	i.LineMatchingState.StringTable[t].FileList[inputIdx] = i.makeLineEntry(linen, p)
}

// Insert a line into the hash table
func (i *Ifcomp) enterLine(text string, h uint64, linen LineCount, inputFile FileIndex, resultHashNode *HashNodeIndex, resultStringIndex *StringIndex) {
	if i.DebugSytFull {
		fmt.Printf("\nEnter line %s, #%d\n", text, linen)
	}

	bucketIdx := int(h % NBuckets)
	hashStartNode := &i.HashState.SecHashStartNode[bucketIdx]
	var SI StringIndex
	var currentNode HashNodeIndex

	if *hashStartNode == NullHashList {
		*hashStartNode = i.setupHashNode(&SI, text, linen, inputFile, h)
		currentNode = *hashStartNode
		*resultHashNode = currentNode
		*resultStringIndex = SI
		return
	}

	currentNode = *hashStartNode
	var lastNode HashNodeIndex = NullHashList
	var lastSI StringIndex

	for currentNode != NullHashList {
		test := hashcodeCompare(h, i.HashState.HashNode[currentNode].H)
		if test == EQ {
			// Search through this syt node to see if the identical line exists already
			SI = i.HashState.HashNode[currentNode].TextList
			lastSI = SI
			for SI != NullStringList {
				if i.LineMatchingState.StringTable[SI].Text == text {
					i.addLinenToTextList(SI, linen, inputFile)
					*resultHashNode = currentNode
					*resultStringIndex = SI
					return
				}
				lastSI = SI
				SI = i.LineMatchingState.StringTable[SI].NextTextWithSameHash
			}
			// If text_list was empty, handle it
			if i.HashState.HashNode[currentNode].TextList == NullStringList {
				i.HashState.HashNode[currentNode].TextList = i.setupDistinctText(text, linen, inputFile)
				SI = i.HashState.HashNode[currentNode].TextList
			} else {
				i.LineMatchingState.StringTable[lastSI].NextTextWithSameHash = i.setupDistinctText(text, linen, inputFile)
				SI = i.LineMatchingState.StringTable[lastSI].NextTextWithSameHash
			}
			*resultHashNode = currentNode
			*resultStringIndex = SI
			return
		}
		if test == LT {
			newNode := i.setupHashNode(&SI, text, linen, inputFile, h)
			if currentNode == *hashStartNode {
				i.HashState.HashNode[newNode].NextInBucket = *hashStartNode
				*hashStartNode = newNode
			} else {
				i.HashState.HashNode[newNode].NextInBucket = currentNode
				i.HashState.HashNode[lastNode].NextInBucket = newNode
			}
			*resultHashNode = newNode
			*resultStringIndex = SI
			return
		}
		// test is GT
		lastNode = currentNode
		currentNode = i.HashState.HashNode[currentNode].NextInBucket
	}

	// Add to chain
	if lastNode == NullHashList {
		fmt.Println("?OOPS empty list!")
	}
	i.HashState.HashNode[lastNode].NextInBucket = i.setupHashNode(&SI, text, linen, inputFile, h)
	currentNode = i.HashState.HashNode[lastNode].NextInBucket
	*resultHashNode = currentNode
	*resultStringIndex = SI
}

// Open a file for reading
func openFile(fn string) (*os.File, error) {
	file, err := os.Open(fn)
	if err != nil {
		fmt.Printf("Can't open file %s\n", fn)
		os.Exit(1)
	}
	return file, nil
}

// Read all lines from an input file
func (i *Ifcomp) readLines(whichFile FileIndex, reader io.Reader) {
	currentLine := 0
	scanner := bufio.NewScanner(reader)
	whichIdx := toArrayIndex(whichFile)

	for scanner.Scan() {
		line := scanner.Text()
		if i.DebugReadCurrentLine {
			fmt.Printf("read %s\n", line)
		}

		currentLine++

		// Resize file_line if needed
		for len(i.FileState.FileLine[whichIdx]) <= currentLine {
			i.FileState.FileLine[whichIdx] = append(i.FileState.FileLine[whichIdx], FileLineDecl{})
		}

		var H HashNodeIndex
		h := i.hashLine(line)
		i.enterLine(line, h, LineCount(currentLine), whichFile, &H, &i.FileState.FileLine[whichIdx][currentLine].FileLineText)

		i.FileState.FileLine[whichIdx][currentLine].Ptr0 = -1
		i.FileState.FileLine[whichIdx][currentLine].Linen = LineCount(currentLine)
		i.FileState.FileLine[whichIdx][currentLine].PtrType = SYT_TYPE
	}

	if err := scanner.Err(); err != nil {
		fmt.Printf("Error reading file: %v\n", err)
		os.Exit(1)
	}

	i.FileState.TotalFileNLines[whichIdx] = currentLine
	if currentLine == 0 {
		fmt.Printf("File %d has no lines.\n", whichIdx)
		os.Exit(whichIdx)
	}
}

// Pass 1: Hash Table Construction
func (i *Ifcomp) pass1(file1, file2 io.Reader) {
	i.readLines(First, file1)
	i.readLines(Second, file2)
	// We can free the hash stuff; not needed now
	i.HashState.HashNode = nil
}
