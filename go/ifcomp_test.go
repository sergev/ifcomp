package main

import (
	"os"
	"testing"
)

func TestIfcompCompare(t *testing.T) {
	// Create temporary test files
	content1 := "A\nB\nC\nD\n"
	content2 := "A\nC\nE\nD\n"

	file1, err := os.CreateTemp("", "test1_*.txt")
	if err != nil {
		t.Fatalf("Failed to create temp file: %v", err)
	}
	defer os.Remove(file1.Name())
	defer file1.Close()

	file2, err := os.CreateTemp("", "test2_*.txt")
	if err != nil {
		t.Fatalf("Failed to create temp file: %v", err)
	}
	defer os.Remove(file2.Name())
	defer file2.Close()

	if _, err := file1.WriteString(content1); err != nil {
		t.Fatalf("Failed to write to file1: %v", err)
	}
	if _, err := file2.WriteString(content2); err != nil {
		t.Fatalf("Failed to write to file2: %v", err)
	}

	file1.Close()
	file2.Close()

	// Test the comparison
	ifc := NewIfcomp()

	// Capture output by running compare (this will print to stdout)
	// For now, just verify it doesn't crash
	ifc.Compare(file1.Name(), file2.Name())

	// Basic checks that the algorithm ran
	// We expect some changes to be detected
	if ifc.Stats.NChangeBlocks == 0 {
		t.Error("Expected at least one change block to be detected")
	}

	// Verify that some matching occurred
	if ifc.FileState.TotalFileNLines[0] == 0 || ifc.FileState.TotalFileNLines[1] == 0 {
		t.Error("Expected files to be read properly")
	}
}
