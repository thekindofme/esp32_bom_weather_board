package main

import (
	"log"
	"os"
	"path/filepath"

	"github.com/gin-contrib/cors"
	"github.com/gin-gonic/gin"
	"github.com/yfernando/esp32-layout-designer/handlers"
	"github.com/yfernando/esp32-layout-designer/storage"
)

func main() {
	// Determine ESP32 project root
	projectRoot := os.Getenv("ESP32_PROJECT_PATH")
	if projectRoot == "" {
		// Default: two levels up from designer/backend/
		exe, _ := os.Executable()
		projectRoot = filepath.Join(filepath.Dir(exe), "..", "..")
		// If running with `go run`, use relative path
		if _, err := os.Stat(filepath.Join(projectRoot, "platformio.ini")); err != nil {
			projectRoot = filepath.Join("..", "..")
			if _, err := os.Stat(filepath.Join(projectRoot, "platformio.ini")); err != nil {
				log.Println("Warning: Could not find ESP32 project root. Set ESP32_PROJECT_PATH env var.")
				projectRoot = "../.."
			}
		}
	}
	// Resolve to absolute path so child processes get correct paths
	if !filepath.IsAbs(projectRoot) {
		abs, err := filepath.Abs(projectRoot)
		if err != nil {
			log.Fatalf("Failed to resolve absolute path for project root %q: %v", projectRoot, err)
		}
		projectRoot = abs
	}
	log.Printf("ESP32 project root: %s", projectRoot)

	// Database
	dbPath := filepath.Join(".", "layouts.db")
	db, err := storage.NewDB(dbPath)
	if err != nil {
		log.Fatalf("Failed to open database: %v", err)
	}
	defer db.Close()
	log.Printf("Database: %s", dbPath)

	// Gin router
	gin.SetMode(gin.ReleaseMode)
	r := gin.Default()

	// CORS
	r.Use(cors.New(cors.Config{
		AllowOrigins:     []string{"http://localhost:5173", "http://localhost:5174", "http://localhost:3000"},
		AllowMethods:     []string{"GET", "POST", "PUT", "DELETE", "OPTIONS"},
		AllowHeaders:     []string{"Content-Type", "Authorization"},
		AllowCredentials: true,
	}))

	// Health check
	r.GET("/health", func(c *gin.Context) {
		c.JSON(200, gin.H{"status": "ok"})
	})

	// Register API routes
	h := handlers.NewHandler(db, projectRoot)
	h.RegisterRoutes(r)

	port := os.Getenv("PORT")
	if port == "" {
		port = "8080"
	}
	log.Printf("Starting server on :%s", port)
	if err := r.Run(":" + port); err != nil {
		log.Fatalf("Server failed: %v", err)
	}
}
