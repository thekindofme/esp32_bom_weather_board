package flasher

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

type FlashResult struct {
	Success bool   `json:"success"`
	Output  string `json:"output"`
}

func WriteLayoutFile(projectRoot string, layoutName string, cppCode string) (string, error) {
	filename := "LayoutCustom_Generated.cpp"
	path := filepath.Join(projectRoot, "src", "layouts", filename)

	if err := os.MkdirAll(filepath.Dir(path), 0755); err != nil {
		return "", fmt.Errorf("create layout dir: %w", err)
	}
	if err := os.WriteFile(path, []byte(cppCode), 0644); err != nil {
		return "", fmt.Errorf("write layout file: %w", err)
	}
	return path, nil
}

func BuildAndFlash(projectRoot string, port string) *FlashResult {
	pioPath := FindPioPath(projectRoot)

	// Verify the resolved pio path exists (unless it's a bare "pio" PATH fallback)
	if pioPath != "pio" {
		if _, err := os.Stat(pioPath); err != nil {
			return &FlashResult{
				Success: false,
				Output:  fmt.Sprintf("PlatformIO not found at %s: %v", pioPath, err),
			}
		}
	}

	args := []string{"run", "-e", "esp32dev", "-t", "upload"}
	if port != "" {
		args = append(args, "--upload-port", port)
	}

	cmd := exec.Command(pioPath, args...)
	cmd.Dir = projectRoot
	cmd.Env = append(os.Environ(), "PLATFORMIO_FORCE_COLOR=false")

	output, err := cmd.CombinedOutput()
	outStr := string(output)

	if err != nil {
		return &FlashResult{
			Success: false,
			Output:  outStr + "\n\nError: " + err.Error(),
		}
	}

	success := strings.Contains(outStr, "[SUCCESS]")
	return &FlashResult{
		Success: success,
		Output:  outStr,
	}
}
