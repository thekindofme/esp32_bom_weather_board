import React, { useState, useEffect, useRef } from 'react';
import { useLayoutStore } from '../store/layoutStore';
import * as api from '../api/client';

export const Toolbar: React.FC = () => {
  const {
    layout, showGrid, snapToGrid, zoom, isDarkPreview,
    setLayoutName, toggleOrientation, toggleGrid, toggleSnap,
    setZoom, toggleThemePreview, newLayout, loadLayout,
    setFlashModalOpen, undo, redo, history, historyIndex,
  } = useLayoutStore();

  const [saving, setSaving] = useState(false);
  const [saved, setSaved] = useState(false);
  const [listOpen, setListOpen] = useState(false);
  const [layouts, setLayouts] = useState<api.Layout[]>([]);
  const dropdownRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const handleClickOutside = (e: MouseEvent) => {
      if (dropdownRef.current && !dropdownRef.current.contains(e.target as Node)) {
        setListOpen(false);
      }
    };
    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  const handleSave = async () => {
    setSaving(true);
    try {
      if (layout.id) {
        await api.updateLayout(layout.id, layout);
      } else {
        const created = await api.createLayout(layout);
        loadLayout(created);
      }
      setSaved(true);
      setTimeout(() => setSaved(false), 2000);
    } catch (err) {
      console.error('Save failed:', err);
    }
    setSaving(false);
  };

  const handleLoadList = async () => {
    if (listOpen) {
      setListOpen(false);
      return;
    }
    try {
      const list = await api.listLayouts();
      setLayouts(list);
    } catch {
      setLayouts([]);
    }
    setListOpen(true);
  };

  const handleLoad = async (id: string) => {
    try {
      const loaded = await api.getLayout(id);
      loadLayout(loaded);
      setListOpen(false);
    } catch (err) {
      console.error('Load failed:', err);
    }
  };

  return (
    <div className="toolbar">
      <div className="toolbar-left">
        <button className="toolbar-btn brand-btn" onClick={newLayout} title="New Layout">
          <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M7 1v12M1 7h12" stroke="currentColor" strokeWidth="2" strokeLinecap="round"/></svg>
        </button>
        <input
          className="layout-name-input"
          value={layout.name}
          onChange={e => setLayoutName(e.target.value)}
          spellCheck={false}
        />
        <div className="toolbar-divider" />
        <button
          className={`toolbar-btn ${layout.orientation === 'landscape' ? 'active' : ''}`}
          onClick={toggleOrientation}
          title="Toggle orientation"
        >
          {layout.orientation === 'portrait' ? (
            <svg width="12" height="16" viewBox="0 0 12 16" fill="none"><rect x="0.5" y="0.5" width="11" height="15" rx="1" stroke="currentColor"/></svg>
          ) : (
            <svg width="16" height="12" viewBox="0 0 16 12" fill="none"><rect x="0.5" y="0.5" width="15" height="11" rx="1" stroke="currentColor"/></svg>
          )}
          <span className="btn-label">{layout.width}x{layout.height}</span>
        </button>
      </div>

      <div className="toolbar-center">
        <button className={`toolbar-btn ${showGrid ? 'active' : ''}`} onClick={toggleGrid} title="Toggle grid">
          <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M0 4.67h14M0 9.33h14M4.67 0v14M9.33 0v14" stroke="currentColor" strokeWidth="0.8"/></svg>
        </button>
        <button className={`toolbar-btn ${snapToGrid ? 'active' : ''}`} onClick={toggleSnap} title="Snap to grid">
          <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><circle cx="3" cy="3" r="1.5" fill="currentColor"/><circle cx="11" cy="3" r="1.5" fill="currentColor"/><circle cx="3" cy="11" r="1.5" fill="currentColor"/><circle cx="11" cy="11" r="1.5" fill="currentColor"/><rect x="5" y="5" width="4" height="4" fill="currentColor" opacity="0.5"/></svg>
        </button>
        <div className="toolbar-divider" />
        <button className="toolbar-btn" onClick={() => setZoom(zoom - 0.25)} title="Zoom out">-</button>
        <span className="zoom-label">{Math.round(zoom * 100)}%</span>
        <button className="toolbar-btn" onClick={() => setZoom(zoom + 0.25)} title="Zoom in">+</button>
        <div className="toolbar-divider" />
        <button
          className={`toolbar-btn ${isDarkPreview ? '' : 'active'}`}
          onClick={toggleThemePreview}
          title="Toggle preview theme"
        >
          {isDarkPreview ? (
            <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><circle cx="7" cy="7" r="5" stroke="currentColor" strokeWidth="1.2"/><path d="M7 2a5 5 0 000 10z" fill="currentColor"/></svg>
          ) : (
            <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><circle cx="7" cy="7" r="3" fill="currentColor"/><path d="M7 0v2M7 12v2M0 7h2M12 7h2M2 2l1.4 1.4M10.6 10.6l1.4 1.4M2 12l1.4-1.4M10.6 3.4L12 2" stroke="currentColor" strokeWidth="1.2" strokeLinecap="round"/></svg>
          )}
        </button>
        <div className="toolbar-divider" />
        <button className="toolbar-btn" onClick={undo} disabled={historyIndex < 0} title="Undo (Ctrl+Z)">
          <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M4 5H10a3 3 0 010 6H8" stroke="currentColor" strokeWidth="1.2" strokeLinecap="round"/><path d="M6 3L4 5l2 2" stroke="currentColor" strokeWidth="1.2" strokeLinecap="round" strokeLinejoin="round"/></svg>
        </button>
        <button className="toolbar-btn" onClick={redo} disabled={historyIndex >= history.length - 1} title="Redo (Ctrl+Shift+Z)">
          <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M10 5H4a3 3 0 000 6h2" stroke="currentColor" strokeWidth="1.2" strokeLinecap="round"/><path d="M8 3l2 2-2 2" stroke="currentColor" strokeWidth="1.2" strokeLinecap="round" strokeLinejoin="round"/></svg>
        </button>
      </div>

      <div className="toolbar-right">
        <div className="dropdown-container" ref={dropdownRef}>
          <button className="toolbar-btn" onClick={handleLoadList} title="Load layout">
            <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M2 4h10M2 7h10M2 10h10" stroke="currentColor" strokeWidth="1.2" strokeLinecap="round"/></svg>
            <span className="btn-label">Layouts</span>
          </button>
          {listOpen && (
            <div className="dropdown-menu">
              {layouts.length === 0 ? (
                <div className="dropdown-empty">No saved layouts</div>
              ) : (
                layouts.map(l => (
                  <button key={l.id} className="dropdown-item" onClick={() => handleLoad(l.id)}>
                    <span>{l.name}</span>
                    <span className="dropdown-item-meta">{l.orientation}</span>
                  </button>
                ))
              )}
            </div>
          )}
        </div>
        <button className="toolbar-btn save-btn" onClick={handleSave} disabled={saving}>
          {saving ? (
            <span className="spinner-tiny" />
          ) : saved ? (
            <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M3 7l3 3 5-6" stroke="#69b987" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"/></svg>
          ) : (
            <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M11 12H3a1 1 0 01-1-1V3a1 1 0 011-1h6l3 3v7a1 1 0 01-1 1z" stroke="currentColor" strokeWidth="1.2"/><path d="M5 12V8h4v4" stroke="currentColor" strokeWidth="1.2"/></svg>
          )}
          <span className="btn-label">Save</span>
        </button>
        <button className="toolbar-btn flash-btn" onClick={() => setFlashModalOpen(true)}>
          <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M8 1L3 8h4l-1 5 5-7H7l1-5z" fill="currentColor"/></svg>
          <span className="btn-label">Flash</span>
        </button>
      </div>
    </div>
  );
};
