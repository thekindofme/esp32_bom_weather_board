package flasher

import (
	"os"
	"path/filepath"
	"runtime"
	"strings"
)

func ListSerialPorts() []string {
	var patterns []string
	switch runtime.GOOS {
	case "darwin":
		patterns = []string{"/dev/cu.usbserial*", "/dev/cu.wchusbserial*", "/dev/cu.SLAB_USBtoUART*", "/dev/tty.usbserial*"}
	case "linux":
		patterns = []string{"/dev/ttyUSB*", "/dev/ttyACM*"}
	default:
		return []string{}
	}

	var ports []string
	seen := map[string]bool{}
	for _, pattern := range patterns {
		matches, _ := filepath.Glob(pattern)
		for _, m := range matches {
			if !seen[m] {
				seen[m] = true
				ports = append(ports, m)
			}
		}
	}
	if ports == nil {
		ports = []string{}
	}
	return ports
}

func FindPioPath(projectRoot string) string {
	candidates := []string{
		filepath.Join(projectRoot, ".venv", "bin", "pio"),
		filepath.Join(projectRoot, ".venv", "Scripts", "pio.exe"),
	}

	// Also check PATH
	pathDirs := strings.Split(os.Getenv("PATH"), string(os.PathListSeparator))
	for _, dir := range pathDirs {
		candidates = append(candidates, filepath.Join(dir, "pio"))
	}

	for _, c := range candidates {
		if info, err := os.Stat(c); err == nil && !info.IsDir() {
			// Resolve to absolute path so exec works regardless of working directory
			if abs, err := filepath.Abs(c); err == nil {
				return abs
			}
			return c
		}
	}
	return "pio" // fallback to PATH lookup
}
