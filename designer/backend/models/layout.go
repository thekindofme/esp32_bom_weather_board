package models

import "time"

type Element struct {
	ID         string                 `json:"id"`
	Type       string                 `json:"type"`
	X          int                    `json:"x"`
	Y          int                    `json:"y"`
	Width      int                    `json:"width"`
	Height     int                    `json:"height"`
	ZIndex     int                    `json:"zIndex"`
	Properties map[string]interface{} `json:"properties"`
}

type Layout struct {
	ID              string    `json:"id"`
	Name            string    `json:"name"`
	Orientation     string    `json:"orientation"`
	Width           int       `json:"width"`
	Height          int       `json:"height"`
	Rotation        int       `json:"rotation"`
	BackgroundColor string    `json:"backgroundColor"`
	Elements        []Element `json:"elements"`
	CreatedAt       string    `json:"createdAt,omitempty"`
	UpdatedAt       string    `json:"updatedAt,omitempty"`
}

func (l *Layout) SetTimestamps() {
	now := time.Now().UTC().Format(time.RFC3339)
	if l.CreatedAt == "" {
		l.CreatedAt = now
	}
	l.UpdatedAt = now
}
