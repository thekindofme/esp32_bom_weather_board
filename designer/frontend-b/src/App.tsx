import React, { useEffect, useCallback } from 'react';
import { HeaderBar } from './components/HeaderBar';
import { DisplayCanvas } from './components/DisplayCanvas';
import { WidgetDrawer } from './components/WidgetDrawer';
import { PropertiesBar } from './components/PropertiesBar';
import { FlashDialog } from './components/FlashDialog';
import { useLayoutStore } from './store/layoutStore';

const App: React.FC = () => {
  const { removeElement, selectedId, undo, redo } = useLayoutStore();

  const handleKeyDown = useCallback((e: KeyboardEvent) => {
    if (e.target instanceof HTMLInputElement || e.target instanceof HTMLTextAreaElement || e.target instanceof HTMLSelectElement) return;
    if ((e.key === 'Delete' || e.key === 'Backspace') && selectedId) {
      e.preventDefault();
      removeElement(selectedId);
    }
    if (e.key === 'z' && (e.metaKey || e.ctrlKey) && !e.shiftKey) { e.preventDefault(); undo(); }
    if (e.key === 'z' && (e.metaKey || e.ctrlKey) && e.shiftKey) { e.preventDefault(); redo(); }
  }, [removeElement, selectedId, undo, redo]);

  useEffect(() => {
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [handleKeyDown]);

  return (
    <div className="atelier">
      <HeaderBar />
      <div className="atelier-body">
        <DisplayCanvas />
        <WidgetDrawer />
      </div>
      <PropertiesBar />
      <FlashDialog />
    </div>
  );
};

export default App;
