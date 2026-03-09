import React, { useRef, useCallback, useEffect } from 'react';
import { Stage, Layer, Rect, Circle, Transformer } from 'react-konva';
import Konva from 'konva';
import { useLayoutStore } from '../store/layoutStore';
import { resolveColor } from '../utils/themeColors';
import { CanvasElement } from './elements/CanvasElements';

export const Canvas: React.FC = () => {
  const stageRef = useRef<any>(null);
  const transformerRef = useRef<Konva.Transformer>(null);
  const elementRefs = useRef<Map<string, Konva.Group>>(new Map());
  const {
    layout, selectedIds, showGrid, gridSize, zoom, isDarkPreview,
    selectElement, moveElement, updateElement, pushHistory,
  } = useLayoutStore();

  const bgColor = resolveColor(layout.backgroundColor, isDarkPreview);

  // Attach transformer to selected element
  useEffect(() => {
    const tr = transformerRef.current;
    if (!tr) return;
    if (selectedIds.length === 1) {
      const node = elementRefs.current.get(selectedIds[0]);
      if (node) {
        tr.nodes([node]);
        tr.getLayer()?.batchDraw();
        return;
      }
    }
    tr.nodes([]);
    tr.getLayer()?.batchDraw();
  }, [selectedIds, layout.elements]);

  const setElementRef = useCallback((id: string, node: Konva.Group | null) => {
    if (node) {
      elementRefs.current.set(id, node);
    } else {
      elementRefs.current.delete(id);
    }
  }, []);

  const handleTransformEnd = useCallback((id: string, node: Konva.Group) => {
    const scaleX = node.scaleX();
    const scaleY = node.scaleY();
    const el = layout.elements.find(e => e.id === id);
    if (!el) return;

    // Reset scale and apply to width/height
    node.scaleX(1);
    node.scaleY(1);

    updateElement(id, {
      x: Math.round(node.x()),
      y: Math.round(node.y()),
      width: Math.max(10, Math.round(el.width * scaleX)),
      height: Math.max(10, Math.round(el.height * scaleY)),
    });
  }, [layout.elements, updateElement]);

  const handleStageClick = useCallback((e: any) => {
    if (e.target === e.target.getStage() || e.target.attrs?.id === 'canvas-bg') {
      selectElement(null);
    }
  }, [selectElement]);

  const handleDragEnd = useCallback((id: string, x: number, y: number) => {
    moveElement(id, x, y);
  }, [moveElement]);

  const handleDragStart = useCallback((_id: string) => {
    pushHistory();
  }, [pushHistory]);

  const gridDots: { x: number; y: number }[] = [];
  if (showGrid) {
    for (let gx = 0; gx <= layout.width; gx += gridSize) {
      for (let gy = 0; gy <= layout.height; gy += gridSize) {
        gridDots.push({ x: gx, y: gy });
      }
    }
  }

  const sortedElements = [...layout.elements].sort((a, b) => a.zIndex - b.zIndex);

  return (
    <div className="canvas-container">
      <div className="canvas-viewport" style={{ transform: `scale(${zoom})`, transformOrigin: 'center center' }}>
        <div className="canvas-frame">
          <div className="canvas-dimensions">
            {layout.width} x {layout.height} &middot; {layout.orientation}
          </div>
          <Stage
            ref={stageRef}
            width={layout.width}
            height={layout.height}
            onClick={handleStageClick}
            onTap={handleStageClick}
            style={{ border: '1px solid rgba(15, 125, 255, 0.3)' }}
          >
            {/* Background layer */}
            <Layer>
              <Rect
                id="canvas-bg"
                x={0} y={0}
                width={layout.width}
                height={layout.height}
                fill={bgColor}
              />
            </Layer>

            {/* Grid layer */}
            {showGrid && (
              <Layer listening={false}>
                {gridDots.map((dot, i) => (
                  <Circle
                    key={i}
                    x={dot.x}
                    y={dot.y}
                    radius={0.5}
                    fill="rgba(15, 125, 255, 0.25)"
                  />
                ))}
              </Layer>
            )}

            {/* Elements layer */}
            <Layer>
              {sortedElements.map(el => (
                <CanvasElement
                  key={el.id}
                  element={el}
                  isDark={isDarkPreview}
                  isSelected={selectedIds.includes(el.id)}
                  onSelect={selectElement}
                  onDragEnd={handleDragEnd}
                  onDragStart={handleDragStart}
                  onTransformEnd={handleTransformEnd}
                  groupRef={(node) => setElementRef(el.id, node)}
                />
              ))}
              <Transformer
                ref={transformerRef}
                boundBoxFunc={(_oldBox, newBox) => {
                  if (newBox.width < 10 || newBox.height < 10) return _oldBox;
                  return newBox;
                }}
                rotateEnabled={false}
                enabledAnchors={['top-left', 'top-center', 'top-right', 'middle-left', 'middle-right', 'bottom-left', 'bottom-center', 'bottom-right']}
                borderStroke="#0f7dff"
                anchorFill="#0f7dff"
                anchorStroke="#fff"
                anchorSize={8}
                anchorCornerRadius={2}
              />
            </Layer>
          </Stage>
        </div>
      </div>
    </div>
  );
};
