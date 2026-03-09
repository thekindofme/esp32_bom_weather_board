package flasher

import (
	"os"
	"path/filepath"
	"testing"
)

func TestWriteLayoutFileUsesCanonicalGeneratedPath(t *testing.T) {
	projectRoot := t.TempDir()
	path, err := WriteLayoutFile(projectRoot, "Anything", "// generated")
	if err != nil {
		t.Fatalf("WriteLayoutFile() error = %v", err)
	}

	wantPath := filepath.Join(projectRoot, "src", "layouts", "LayoutCustom_Generated.cpp")
	if path != wantPath {
		t.Fatalf("WriteLayoutFile() path = %q, want %q", path, wantPath)
	}

	content, err := os.ReadFile(path)
	if err != nil {
		t.Fatalf("ReadFile() error = %v", err)
	}
	if string(content) != "// generated" {
		t.Fatalf("generated file content = %q", string(content))
	}
}
