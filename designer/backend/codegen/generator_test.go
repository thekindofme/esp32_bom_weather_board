package codegen

import (
	"strings"
	"testing"

	"github.com/yfernando/esp32-layout-designer/models"
)

func TestSanitizeNamePrefixesLeadingDigit(t *testing.T) {
	if got, want := sanitizeName("2026 Storm"), "Custom2026Storm"; got != want {
		t.Fatalf("sanitizeName() = %q, want %q", got, want)
	}
}

func TestGenerateLayoutCppUsesStableGeneratedSymbolsAndEscapedName(t *testing.T) {
	layout := &models.Layout{
		Name:            "2026 \"Storm\"\nDemo",
		Orientation:     "portrait",
		Width:           240,
		Height:          320,
		Rotation:        0,
		BackgroundColor: "themeBg",
		Elements: []models.Element{
			{
				ID:     "el-1",
				Type:   "data-text",
				X:      14,
				Y:      58,
				Width:  160,
				Height: 52,
				ZIndex: 1,
				Properties: map[string]interface{}{
					"dataField": "airTempC",
					"font":      4,
					"suffix":    "°",
				},
			},
		},
	}

	code := GenerateLayoutCpp(layout)

	checks := []string{
		`const bool hasGeneratedCustomLayout = true;`,
		`const uint32_t generatedCustomLayoutFingerprint = 0x`,
		`static void custom2026StormDemoDrawHeader`,
		`const LayoutFunctions layoutCustomGenerated = {`,
		`"2026 \"Storm\"\nDemo"`,
	}
	for _, needle := range checks {
		if !strings.Contains(code, needle) {
			t.Fatalf("generated code missing %q\n%s", needle, code)
		}
	}

	if strings.Contains(code, `const LayoutFunctions layoutCustomCustom2026StormDemo`) {
		t.Fatalf("generated code should not use layout-name-derived exported symbol\n%s", code)
	}
}

func TestGenerateLayoutCppAppliesTextSizeToTimeAndDate(t *testing.T) {
	layout := &models.Layout{
		Name:            "Scaled Clock",
		Orientation:     "portrait",
		Width:           240,
		Height:          320,
		Rotation:        0,
		BackgroundColor: "themeBg",
		Elements: []models.Element{
			{
				ID:     "time-1",
				Type:   "time",
				X:      8,
				Y:      8,
				Width:  120,
				Height: 28,
				ZIndex: 1,
				Properties: map[string]interface{}{
					"font":        2,
					"textSize":    3,
					"format":      "24h",
					"showSeconds": false,
				},
			},
			{
				ID:     "date-1",
				Type:   "date",
				X:      8,
				Y:      40,
				Width:  160,
				Height: 28,
				ZIndex: 2,
				Properties: map[string]interface{}{
					"font":       2,
					"textSize":   4,
					"dateFormat": "short",
				},
			},
		},
	}

	code := GenerateLayoutCpp(layout)

	checks := []string{
		`    tft.setTextSize(3);`,
		`    tft.drawString(timeBuf, 8, 8, 2);`,
		`    tft.setTextSize(4);`,
		`    tft.drawString(dateStr, 8, 40, 2);`,
	}
	for _, needle := range checks {
		if !strings.Contains(code, needle) {
			t.Fatalf("generated code missing %q\n%s", needle, code)
		}
	}

	if got := strings.Count(code, `    tft.setTextSize(1);`); got < 2 {
		t.Fatalf("expected generated code to reset text size after scaled time/date draws, got %d\n%s", got, code)
	}
}
