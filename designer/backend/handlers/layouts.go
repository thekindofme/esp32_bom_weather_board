package handlers

import (
	"net/http"
	"os"
	"path/filepath"

	"github.com/gin-gonic/gin"
	"github.com/google/uuid"
	"github.com/yfernando/esp32-layout-designer/codegen"
	"github.com/yfernando/esp32-layout-designer/flasher"
	"github.com/yfernando/esp32-layout-designer/models"
	"github.com/yfernando/esp32-layout-designer/storage"
)

type Handler struct {
	DB          *storage.DB
	ProjectRoot string
}

func NewHandler(db *storage.DB, projectRoot string) *Handler {
	return &Handler{DB: db, ProjectRoot: projectRoot}
}

func applyLayoutDefaults(layout *models.Layout) {
	if layout.Name == "" {
		layout.Name = "Untitled"
	}
	if layout.Width == 0 {
		layout.Width = 240
	}
	if layout.Height == 0 {
		layout.Height = 320
	}
	if layout.Orientation == "" {
		layout.Orientation = "portrait"
	}
	if layout.BackgroundColor == "" {
		layout.BackgroundColor = "themeBg"
	}
	if layout.Elements == nil {
		layout.Elements = []models.Element{}
	}
}

func (h *Handler) respondFlashResult(c *gin.Context, result *flasher.FlashResult) {
	if !result.Success {
		c.JSON(http.StatusBadGateway, gin.H{
			"success": false,
			"error":   "flash failed",
			"output":  result.Output,
		})
		return
	}
	c.JSON(http.StatusOK, result)
}

func (h *Handler) flashResolvedLayout(c *gin.Context, layout *models.Layout, port string) {
	code := codegen.GenerateLayoutCpp(layout)
	_, writeErr := flasher.WriteLayoutFile(h.ProjectRoot, layout.Name, code)
	if writeErr != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": writeErr.Error()})
		return
	}

	result := flasher.BuildAndFlash(h.ProjectRoot, port)
	h.respondFlashResult(c, result)
}

func (h *Handler) ListLayouts(c *gin.Context) {
	layouts, err := h.DB.ListLayouts()
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	c.JSON(http.StatusOK, layouts)
}

func (h *Handler) GetLayout(c *gin.Context) {
	id := c.Param("id")
	layout, err := h.DB.GetLayout(id)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	if layout == nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "layout not found"})
		return
	}
	c.JSON(http.StatusOK, layout)
}

func (h *Handler) CreateLayout(c *gin.Context) {
	var layout models.Layout
	if err := c.ShouldBindJSON(&layout); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	layout.ID = uuid.New().String()
	applyLayoutDefaults(&layout)

	if err := h.DB.CreateLayout(&layout); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	c.JSON(http.StatusCreated, layout)
}

func (h *Handler) UpdateLayout(c *gin.Context) {
	id := c.Param("id")
	var layout models.Layout
	if err := c.ShouldBindJSON(&layout); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	if err := h.DB.UpdateLayout(id, &layout); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	layout.ID = id
	c.JSON(http.StatusOK, layout)
}

func (h *Handler) DeleteLayout(c *gin.Context) {
	id := c.Param("id")
	if err := h.DB.DeleteLayout(id); err != nil {
		c.JSON(http.StatusNotFound, gin.H{"error": err.Error()})
		return
	}
	c.Status(http.StatusNoContent)
}

func (h *Handler) GenerateCode(c *gin.Context) {
	id := c.Param("id")
	layout, err := h.DB.GetLayout(id)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	if layout == nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "layout not found"})
		return
	}

	code := codegen.GenerateLayoutCpp(layout)
	c.JSON(http.StatusOK, gin.H{"code": code})
}

func (h *Handler) GenerateCodeFromPayload(c *gin.Context) {
	var layout models.Layout
	if err := c.ShouldBindJSON(&layout); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	applyLayoutDefaults(&layout)

	code := codegen.GenerateLayoutCpp(&layout)
	c.JSON(http.StatusOK, gin.H{"code": code})
}

func (h *Handler) FlashLayout(c *gin.Context) {
	id := c.Param("id")
	var req struct {
		Port string `json:"port"`
	}
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	layout, err := h.DB.GetLayout(id)
	if err != nil || layout == nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "layout not found"})
		return
	}

	h.flashResolvedLayout(c, layout, req.Port)
}

func (h *Handler) FlashLayoutFromPayload(c *gin.Context) {
	var req struct {
		Port   string        `json:"port"`
		Layout models.Layout `json:"layout"`
	}
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	applyLayoutDefaults(&req.Layout)
	h.flashResolvedLayout(c, &req.Layout, req.Port)
}

func (h *Handler) GetDevicePorts(c *gin.Context) {
	ports := flasher.ListSerialPorts()
	c.JSON(http.StatusOK, ports)
}

func (h *Handler) RegisterRoutes(r *gin.Engine) {
	api := r.Group("/api")
	{
		api.GET("/layouts", h.ListLayouts)
		api.GET("/layouts/:id", h.GetLayout)
		api.POST("/layouts", h.CreateLayout)
		api.PUT("/layouts/:id", h.UpdateLayout)
		api.DELETE("/layouts/:id", h.DeleteLayout)
		api.POST("/codegen", h.GenerateCodeFromPayload)
		api.POST("/codegen/:id", h.GenerateCode)
		api.POST("/flash", h.FlashLayoutFromPayload)
		api.POST("/layouts/:id/flash", h.FlashLayout)
		api.GET("/device/ports", h.GetDevicePorts)
	}

	// Serve frontend static files if dist directories exist
	for _, fe := range []struct{ path, dir string }{
		{"/a", filepath.Join(h.ProjectRoot, "designer", "frontend-a", "dist")},
		{"/b", filepath.Join(h.ProjectRoot, "designer", "frontend-b", "dist")},
	} {
		if info, err := os.Stat(fe.dir); err == nil && info.IsDir() {
			r.Static(fe.path, fe.dir)
		}
	}
}
