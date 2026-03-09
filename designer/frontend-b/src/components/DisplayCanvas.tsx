import React, { useRef, useCallback, useState } from 'react';
import { useLayoutStore } from '../store/layoutStore';
import { resolveColor } from '../utils/themeColors';
import { WidgetRenderer } from './widgets/WidgetRenderer';

type ResizeHandle = 'nw' | 'n' | 'ne' | 'e' | 'se' | 's' | 'sw' | 'w';
const RESIZE_HANDLES: ResizeHandle[] = ['nw', 'n', 'ne', 'e', 'se', 's', 'sw', 'w'];
const MIN_SIZE = 10;

interface ResizeState {
  id: string;
  handle: ResizeHandle;
  startX: number;
  startY: number;
  origX: number;
  origY: number;
  origW: number;
  origH: number;
}

export const DisplayCanvas: React.FC = () => {
  const {
    layout, selectedId, showGrid, gridSize, zoom, isDarkPreview, snapToGrid,
    selectElement, moveElement, updateElement, startDrag, endDrag, draggingId, dragOffset, pushHistory,
  } = useLayoutStore();
  const canvasRef = useRef<HTMLDivElement>(null);
  const [resizing, setResizing] = useState<ResizeState | null>(null);

  const bgColor = resolveColor(layout.backgroundColor, isDarkPreview);
  const sorted = [...layout.elements].sort((a, b) => a.zIndex - b.zIndex);

  const handleCanvasClick = (e: React.MouseEvent) => {
    if (e.target === e.currentTarget || (e.target as HTMLElement).classList.contains('display-inner')) {
      selectElement(null);
    }
  };

  const handleMouseDown = useCallback((id: string, e: React.MouseEvent) => {
    e.stopPropagation();
    selectElement(id);
    pushHistory();
    const el = layout.elements.find(x => x.id === id);
    if (!el || !canvasRef.current) return;
    const rect = canvasRef.current.getBoundingClientRect();
    const scale = zoom;
    const ox = (e.clientX - rect.left) / scale - el.x;
    const oy = (e.clientY - rect.top) / scale - el.y;
    startDrag(id, ox, oy);
  }, [layout.elements, zoom, selectElement, startDrag, pushHistory]);

  const handleResizeStart = useCallback((id: string, handle: ResizeHandle, e: React.MouseEvent) => {
    e.stopPropagation();
    e.preventDefault();
    const el = layout.elements.find(x => x.id === id);
    if (!el) return;
    pushHistory();
    setResizing({
      id, handle,
      startX: e.clientX, startY: e.clientY,
      origX: el.x, origY: el.y, origW: el.width, origH: el.height,
    });
  }, [layout.elements, pushHistory]);

  const handleMouseMove = useCallback((e: React.MouseEvent) => {
    if (resizing) {
      const dx = (e.clientX - resizing.startX) / zoom;
      const dy = (e.clientY - resizing.startY) / zoom;
      const h = resizing.handle;
      const snap = (v: number) => snapToGrid ? Math.round(v / gridSize) * gridSize : Math.round(v);

      let newX = resizing.origX;
      let newY = resizing.origY;
      let newW = resizing.origW;
      let newH = resizing.origH;

      if (h.includes('e')) newW = resizing.origW + dx;
      if (h.includes('w')) { newW = resizing.origW - dx; newX = resizing.origX + dx; }
      if (h.includes('s')) newH = resizing.origH + dy;
      if (h.includes('n')) { newH = resizing.origH - dy; newY = resizing.origY + dy; }

      // Enforce minimum size
      if (newW < MIN_SIZE) { if (h.includes('w')) newX = resizing.origX + resizing.origW - MIN_SIZE; newW = MIN_SIZE; }
      if (newH < MIN_SIZE) { if (h.includes('n')) newY = resizing.origY + resizing.origH - MIN_SIZE; newH = MIN_SIZE; }

      updateElement(resizing.id, {
        x: snap(newX), y: snap(newY),
        width: snap(newW) || MIN_SIZE, height: snap(newH) || MIN_SIZE,
      });
      return;
    }

    if (!draggingId || !canvasRef.current) return;
    const rect = canvasRef.current.getBoundingClientRect();
    const scale = zoom;
    const x = (e.clientX - rect.left) / scale - dragOffset.x;
    const y = (e.clientY - rect.top) / scale - dragOffset.y;
    moveElement(draggingId, x, y);
  }, [resizing, draggingId, dragOffset, zoom, moveElement, updateElement, snapToGrid, gridSize]);

  const handleMouseUp = useCallback(() => {
    if (resizing) { setResizing(null); return; }
    if (draggingId) endDrag();
  }, [resizing, draggingId, endDrag]);

  // Ruler ticks
  const hTicks = [];
  for (let i = 0; i <= layout.width; i += 10) {
    const major = i % 50 === 0;
    hTicks.push(
      <div key={`h${i}`} className="ruler-tick-h" style={{ left: i * zoom, height: major ? 8 : 4 }}>
        {major && i > 0 && <span className="ruler-num">{i}</span>}
      </div>
    );
  }
  const vTicks = [];
  for (let i = 0; i <= layout.height; i += 10) {
    const major = i % 50 === 0;
    vTicks.push(
      <div key={`v${i}`} className="ruler-tick-v" style={{ top: i * zoom, width: major ? 8 : 4 }}>
        {major && i > 0 && <span className="ruler-num-v">{i}</span>}
      </div>
    );
  }

  return (
    <div className="canvas-area" onMouseMove={handleMouseMove} onMouseUp={handleMouseUp} onMouseLeave={handleMouseUp}>
      <div className="blueprint-frame">
        {/* Horizontal ruler */}
        <div className="ruler-h" style={{ width: layout.width * zoom + 28 }}>
          <div className="ruler-corner" />
          {hTicks}
        </div>
        <div className="blueprint-body">
          {/* Vertical ruler */}
          <div className="ruler-v" style={{ height: layout.height * zoom }}>
            {vTicks}
          </div>
          {/* Display */}
          <div
            ref={canvasRef}
            className="display-surface"
            style={{
              width: layout.width * zoom,
              height: layout.height * zoom,
            }}
            onClick={handleCanvasClick}
          >
            <div
              className="display-inner"
              style={{
                width: layout.width,
                height: layout.height,
                backgroundColor: bgColor,
                transform: `scale(${zoom})`,
                transformOrigin: 'top left',
              }}
            >
              {/* Grid */}
              {showGrid && (
                <div className="grid-overlay" style={{ width: layout.width, height: layout.height }}>
                  <svg width={layout.width} height={layout.height}>
                    {Array.from({ length: Math.floor(layout.width / gridSize) + 1 }, (_, i) => (
                      <line key={`gv${i}`} x1={i * gridSize} y1={0} x2={i * gridSize} y2={layout.height}
                        stroke="rgba(26,107,90,0.12)" strokeWidth={i % 5 === 0 ? 0.5 : 0.25} />
                    ))}
                    {Array.from({ length: Math.floor(layout.height / gridSize) + 1 }, (_, i) => (
                      <line key={`gh${i}`} x1={0} y1={i * gridSize} x2={layout.width} y2={i * gridSize}
                        stroke="rgba(26,107,90,0.12)" strokeWidth={i % 5 === 0 ? 0.5 : 0.25} />
                    ))}
                  </svg>
                </div>
              )}

              {/* Elements */}
              {sorted.map(el => (
                <div
                  key={el.id}
                  className={`canvas-element ${selectedId === el.id ? 'selected' : ''}`}
                  style={{
                    position: 'absolute',
                    left: el.x,
                    top: el.y,
                    width: el.width,
                    height: el.height,
                    zIndex: el.zIndex + 10,
                    cursor: draggingId === el.id ? 'grabbing' : 'grab',
                  }}
                  onMouseDown={(e) => handleMouseDown(el.id, e)}
                >
                  <WidgetRenderer element={el} isDark={isDarkPreview} />
                  {selectedId === el.id && RESIZE_HANDLES.map(h => (
                    <div
                      key={h}
                      className={`resize-handle ${h}`}
                      onMouseDown={(e) => handleResizeStart(el.id, h, e)}
                    />
                  ))}
                </div>
              ))}
            </div>
          </div>
        </div>
        <div className="canvas-label">
          {layout.width} x {layout.height} &middot; {layout.orientation}
        </div>
      </div>
    </div>
  );
};
