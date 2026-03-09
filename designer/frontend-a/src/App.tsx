import React, { useEffect, useCallback } from 'react';
import { Toolbar } from './components/Toolbar';
import { WidgetPalette } from './components/WidgetPalette';
import { Canvas } from './components/Canvas';
import { PropertyInspector } from './components/PropertyInspector';
import { FlashModal } from './components/FlashModal';
import { useLayoutStore } from './store/layoutStore';

const App: React.FC = () => {
  const { removeSelected, duplicateSelected, undo, redo, selectedIds, layout } = useLayoutStore();

  const handleKeyDown = useCallback((e: KeyboardEvent) => {
    if (e.target instanceof HTMLInputElement || e.target instanceof HTMLTextAreaElement || e.target instanceof HTMLSelectElement) {
      return;
    }

    if ((e.key === 'Delete' || e.key === 'Backspace') && selectedIds.length > 0) {
      e.preventDefault();
      removeSelected();
    }
    if (e.key === 'd' && (e.metaKey || e.ctrlKey)) {
      e.preventDefault();
      duplicateSelected();
    }
    if (e.key === 'z' && (e.metaKey || e.ctrlKey) && !e.shiftKey) {
      e.preventDefault();
      undo();
    }
    if (e.key === 'z' && (e.metaKey || e.ctrlKey) && e.shiftKey) {
      e.preventDefault();
      redo();
    }
  }, [removeSelected, duplicateSelected, undo, redo, selectedIds]);

  useEffect(() => {
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [handleKeyDown]);

  return (
    <div className="app">
      <Toolbar />
      <div className="main-area">
        <WidgetPalette />
        <Canvas />
        <PropertyInspector />
      </div>
      <div className="status-bar">
        <span className="status-left">
          {layout.elements.length} element{layout.elements.length !== 1 ? 's' : ''}
          {selectedIds.length > 0 && ` \u00B7 ${selectedIds.length} selected`}
        </span>
        <span className="status-right">
          ESP32 CYD \u00B7 {layout.width}x{layout.height} \u00B7 {layout.orientation}
        </span>
      </div>
      <FlashModal />
    </div>
  );
};

export default App;
