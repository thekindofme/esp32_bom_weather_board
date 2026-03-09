import React, { useState } from 'react';
import { useLayoutStore } from '../store/layoutStore';
import * as api from '../api/client';

export const HeaderBar: React.FC = () => {
  const {
    layout, showGrid, snapToGrid, zoom, isDarkPreview,
    setLayoutName, toggleOrientation, toggleGrid, toggleSnap,
    setZoom, toggleTheme, newLayout, loadLayout, setFlashDialogOpen,
    undo, redo, history, historyIndex,
  } = useLayoutStore();

  const [saving, setSaving] = useState(false);
  const [saved, setSaved] = useState(false);
  const [listOpen, setListOpen] = useState(false);
  const [layouts, setLayouts] = useState<api.Layout[]>([]);

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
    } catch (err) { console.error(err); }
    setSaving(false);
  };

  const handleLoadList = async () => {
    if (listOpen) { setListOpen(false); return; }
    try { setLayouts(await api.listLayouts()); } catch { setLayouts([]); }
    setListOpen(true);
  };

  const handleLoad = async (id: string) => {
    try { loadLayout(await api.getLayout(id)); setListOpen(false); } catch (e) { console.error(e); }
  };

  return (
    <header className="header-bar">
      <div className="header-brand">
        <svg width="22" height="22" viewBox="0 0 22 22" fill="none">
          <rect x="2" y="4" width="18" height="14" rx="2" stroke="currentColor" strokeWidth="1.5"/>
          <rect x="6" y="8" width="4" height="6" fill="currentColor" opacity="0.3"/>
          <rect x="12" y="8" width="4" height="6" fill="currentColor" opacity="0.3"/>
        </svg>
        <span className="brand-text">Layout Atelier</span>
      </div>

      <div className="header-center">
        <input
          className="name-input"
          value={layout.name}
          onChange={e => setLayoutName(e.target.value)}
          spellCheck={false}
        />
        <button className="hdr-btn" onClick={toggleOrientation} title={layout.orientation}>
          <span className="hdr-badge">{layout.width}x{layout.height}</span>
        </button>
        <span className="hdr-sep" />
        <button className={`hdr-btn ${showGrid ? 'on' : ''}`} onClick={toggleGrid} title="Grid">Grid</button>
        <button className={`hdr-btn ${snapToGrid ? 'on' : ''}`} onClick={toggleSnap} title="Snap">Snap</button>
        <span className="hdr-sep" />
        <button className="hdr-btn" onClick={() => setZoom(zoom - 0.25)}>-</button>
        <span className="hdr-zoom">{Math.round(zoom * 100)}%</span>
        <button className="hdr-btn" onClick={() => setZoom(zoom + 0.25)}>+</button>
        <span className="hdr-sep" />
        <button className={`hdr-btn ${!isDarkPreview ? 'on' : ''}`} onClick={toggleTheme}>
          {isDarkPreview ? 'Dark' : 'Light'}
        </button>
        <span className="hdr-sep" />
        <button className="hdr-btn" onClick={undo} disabled={historyIndex < 0}>Undo</button>
        <button className="hdr-btn" onClick={redo} disabled={historyIndex >= history.length - 1}>Redo</button>
      </div>

      <div className="header-actions">
        <button className="hdr-btn" onClick={newLayout}>New</button>
        <div className="dropdown-wrap">
          <button className="hdr-btn" onClick={handleLoadList}>Open</button>
          {listOpen && (
            <div className="hdr-dropdown">
              {layouts.length === 0 ? (
                <div className="hdr-dropdown-empty">No saved layouts</div>
              ) : layouts.map(l => (
                <button key={l.id} className="hdr-dropdown-item" onClick={() => handleLoad(l.id)}>
                  {l.name}
                  <span className="hdr-dropdown-meta">{l.orientation}</span>
                </button>
              ))}
            </div>
          )}
        </div>
        <button className="hdr-btn save" onClick={handleSave} disabled={saving}>
          {saving ? '...' : saved ? 'Saved!' : 'Save'}
        </button>
        <button className="hdr-btn flash" onClick={() => setFlashDialogOpen(true)}>
          Flash
        </button>
      </div>
    </header>
  );
};
