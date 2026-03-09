package storage

import (
	"database/sql"
	"encoding/json"
	"fmt"
	"time"

	"github.com/yfernando/esp32-layout-designer/models"
	_ "modernc.org/sqlite"
)

type DB struct {
	db *sql.DB
}

func NewDB(path string) (*DB, error) {
	db, err := sql.Open("sqlite", path+"?_journal_mode=WAL&_busy_timeout=5000")
	if err != nil {
		return nil, fmt.Errorf("open db: %w", err)
	}

	_, err = db.Exec(`CREATE TABLE IF NOT EXISTS layouts (
		id TEXT PRIMARY KEY,
		name TEXT NOT NULL,
		data TEXT NOT NULL,
		created_at TEXT NOT NULL,
		updated_at TEXT NOT NULL
	)`)
	if err != nil {
		return nil, fmt.Errorf("create table: %w", err)
	}

	return &DB{db: db}, nil
}

func (d *DB) Close() error {
	return d.db.Close()
}

func (d *DB) CreateLayout(layout *models.Layout) error {
	now := time.Now().UTC().Format(time.RFC3339)
	layout.CreatedAt = now
	layout.UpdatedAt = now

	data, err := json.Marshal(layout)
	if err != nil {
		return fmt.Errorf("marshal layout: %w", err)
	}

	_, err = d.db.Exec(
		"INSERT INTO layouts (id, name, data, created_at, updated_at) VALUES (?, ?, ?, ?, ?)",
		layout.ID, layout.Name, string(data), layout.CreatedAt, layout.UpdatedAt,
	)
	if err != nil {
		return fmt.Errorf("insert layout: %w", err)
	}
	return nil
}

func (d *DB) GetLayout(id string) (*models.Layout, error) {
	var data string
	err := d.db.QueryRow("SELECT data FROM layouts WHERE id = ?", id).Scan(&data)
	if err == sql.ErrNoRows {
		return nil, nil
	}
	if err != nil {
		return nil, fmt.Errorf("query layout: %w", err)
	}

	var layout models.Layout
	if err := json.Unmarshal([]byte(data), &layout); err != nil {
		return nil, fmt.Errorf("unmarshal layout: %w", err)
	}
	return &layout, nil
}

func (d *DB) ListLayouts() ([]models.Layout, error) {
	rows, err := d.db.Query("SELECT data FROM layouts ORDER BY updated_at DESC")
	if err != nil {
		return nil, fmt.Errorf("query layouts: %w", err)
	}
	defer rows.Close()

	var layouts []models.Layout
	for rows.Next() {
		var data string
		if err := rows.Scan(&data); err != nil {
			return nil, fmt.Errorf("scan layout: %w", err)
		}
		var layout models.Layout
		if err := json.Unmarshal([]byte(data), &layout); err != nil {
			continue
		}
		layouts = append(layouts, layout)
	}
	if err := rows.Err(); err != nil {
		return nil, fmt.Errorf("iterate layouts: %w", err)
	}
	if layouts == nil {
		layouts = []models.Layout{}
	}
	return layouts, nil
}

func (d *DB) UpdateLayout(id string, layout *models.Layout) error {
	layout.UpdatedAt = time.Now().UTC().Format(time.RFC3339)
	layout.ID = id

	data, err := json.Marshal(layout)
	if err != nil {
		return fmt.Errorf("marshal layout: %w", err)
	}

	res, err := d.db.Exec(
		"UPDATE layouts SET name = ?, data = ?, updated_at = ? WHERE id = ?",
		layout.Name, string(data), layout.UpdatedAt, id,
	)
	if err != nil {
		return fmt.Errorf("update layout: %w", err)
	}
	n, _ := res.RowsAffected()
	if n == 0 {
		return fmt.Errorf("layout not found: %s", id)
	}
	return nil
}

func (d *DB) DeleteLayout(id string) error {
	res, err := d.db.Exec("DELETE FROM layouts WHERE id = ?", id)
	if err != nil {
		return fmt.Errorf("delete layout: %w", err)
	}
	n, _ := res.RowsAffected()
	if n == 0 {
		return fmt.Errorf("layout not found: %s", id)
	}
	return nil
}
