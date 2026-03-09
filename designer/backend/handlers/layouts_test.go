package handlers

import (
	"bytes"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/gin-gonic/gin"
	"github.com/yfernando/esp32-layout-designer/storage"
)

func TestFlashLayoutFromPayloadReturnsBadGatewayOnToolFailure(t *testing.T) {
	gin.SetMode(gin.TestMode)

	projectRoot := t.TempDir()
	pioPath := filepath.Join(projectRoot, ".venv", "bin", "pio")
	if err := os.MkdirAll(filepath.Dir(pioPath), 0755); err != nil {
		t.Fatalf("MkdirAll() error = %v", err)
	}
	script := "#!/bin/sh\nprintf '%s\\n' 'fake pio failure'\nexit 1\n"
	if err := os.WriteFile(pioPath, []byte(script), 0755); err != nil {
		t.Fatalf("WriteFile() error = %v", err)
	}

	dbPath := filepath.Join(t.TempDir(), "layouts.db")
	db, err := storage.NewDB(dbPath)
	if err != nil {
		t.Fatalf("NewDB() error = %v", err)
	}
	defer db.Close()

	router := gin.New()
	NewHandler(db, projectRoot).RegisterRoutes(router)

	body := map[string]any{
		"port": "/dev/ttyFAKE0",
		"layout": map[string]any{
			"name":            "Payload Preview",
			"orientation":     "portrait",
			"width":           240,
			"height":          320,
			"rotation":        0,
			"backgroundColor": "themeBg",
			"elements":        []map[string]any{},
		},
	}
	payload, err := json.Marshal(body)
	if err != nil {
		t.Fatalf("Marshal() error = %v", err)
	}

	req := httptest.NewRequest(http.MethodPost, "/api/flash", bytes.NewReader(payload))
	req.Header.Set("Content-Type", "application/json")
	rec := httptest.NewRecorder()
	router.ServeHTTP(rec, req)

	if rec.Code != http.StatusBadGateway {
		t.Fatalf("status = %d, want %d; body=%s", rec.Code, http.StatusBadGateway, rec.Body.String())
	}

	var response struct {
		Success bool   `json:"success"`
		Error   string `json:"error"`
		Output  string `json:"output"`
	}
	if err := json.NewDecoder(rec.Body).Decode(&response); err != nil {
		t.Fatalf("Decode() error = %v", err)
	}

	if response.Success {
		t.Fatalf("expected flash failure response, got success=true")
	}
	if response.Error != "flash failed" {
		t.Fatalf("error = %q, want %q", response.Error, "flash failed")
	}
	if !strings.Contains(response.Output, "fake pio failure") {
		t.Fatalf("output missing fake pio failure log: %q", response.Output)
	}

	generatedPath := filepath.Join(projectRoot, "src", "layouts", "LayoutCustom_Generated.cpp")
	if _, err := os.Stat(generatedPath); err != nil {
		t.Fatalf("generated layout not written: %v", err)
	}
}
