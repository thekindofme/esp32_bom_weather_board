import { create } from 'zustand';
import { v4 as uuidv4 } from 'uuid';
import type { Layout, LayoutElement } from '../api/client';
import { applyElementUpdate, applyPropertyUpdate, normalizeElement, normalizeLayout } from './layoutNormalization';

export interface LayoutState {
  layout: Layout;
  selectedIds: string[];
  showGrid: boolean;
  snapToGrid: boolean;
  gridSize: number;
  zoom: number;
  isDarkPreview: boolean;
  flashModalOpen: boolean;
  layoutListOpen: boolean;
  history: Layout[];
  historyIndex: number;

  // Actions
  addElement: (type: string, x?: number, y?: number) => void;
  updateElement: (id: string, updates: Partial<LayoutElement>) => void;
  updateElementProperty: (id: string, key: string, value: unknown) => void;
  removeElement: (id: string) => void;
  removeSelected: () => void;
  selectElement: (id: string | null, addToSelection?: boolean) => void;
  moveElement: (id: string, x: number, y: number) => void;
  resizeElement: (id: string, width: number, height: number) => void;
  setLayoutName: (name: string) => void;
  toggleOrientation: () => void;
  toggleGrid: () => void;
  toggleSnap: () => void;
  setZoom: (zoom: number) => void;
  toggleThemePreview: () => void;
  duplicateSelected: () => void;
  loadLayout: (layout: Layout) => void;
  newLayout: () => void;
  setFlashModalOpen: (open: boolean) => void;
  setLayoutListOpen: (open: boolean) => void;
  undo: () => void;
  redo: () => void;
  pushHistory: () => void;
}

function createDefaultLayout(): Layout {
  return {
    id: '',
    name: 'Untitled Layout',
    orientation: 'portrait',
    width: 240,
    height: 320,
    rotation: 0,
    backgroundColor: 'themeBg',
    elements: [],
  };
}

function defaultElementForType(type: string, x: number, y: number): LayoutElement {
  const base = { id: uuidv4(), type, x, y, zIndex: 1 };
  switch (type) {
    case 'data-text':
      return { ...base, width: 120, height: 30, properties: { dataField: 'airTempC', font: 4, textSize: 1, color: 'themeText', bgColor: 'themeBg', align: 'left', prefix: '', suffix: '\u00B0' } };
    case 'static-text':
      return { ...base, width: 80, height: 20, properties: { text: 'Label', font: 2, textSize: 1, color: 'themeText', bgColor: 'themeBg', align: 'left' } };
    case 'weather-icon':
      return { ...base, width: 48, height: 48, properties: { iconSize: 48, dataField: 'currentIconCode' } };
    case 'rect':
      return { ...base, width: 100, height: 60, zIndex: 0, properties: { fillColor: 'themePanel', strokeColor: 'themeEdge', cornerRadius: 0 } };
    case 'rounded-rect':
      return { ...base, width: 100, height: 60, zIndex: 0, properties: { fillColor: 'themePanel', strokeColor: 'themeEdge', cornerRadius: 6 } };
    case 'hline':
      return { ...base, width: 200, height: 1, properties: { color: 'themeEdge' } };
    case 'vline':
      return { ...base, width: 1, height: 100, properties: { color: 'themeEdge' } };
    case 'forecast-card':
      return { ...base, width: 232, height: 80, properties: { cardWidth: 76, cardHeight: 58, cardSpacing: 2, cornerRadius: 6, cardBg: 'themePanel', cardBorder: 'themeEdge', labelColor: 'themeAccent', tempColor: 'themeText', rainColor: 'themeTextMuted' } };
    case 'time':
      return { ...base, width: 100, height: 20, properties: { font: 2, textSize: 1, color: 'themeGood', bgColor: 'themeHeader', format: '12h', showSeconds: true } };
    case 'date':
      return { ...base, width: 100, height: 20, properties: { font: 2, textSize: 1, color: 'themeGood', bgColor: 'themeHeader', dateFormat: 'short' } };
    default:
      return { ...base, width: 80, height: 30, properties: {} };
  }
}

export const useLayoutStore = create<LayoutState>((set, get) => ({
  layout: createDefaultLayout(),
  selectedIds: [],
  showGrid: true,
  snapToGrid: true,
  gridSize: 10,
  zoom: 1.5,
  isDarkPreview: true,
  flashModalOpen: false,
  layoutListOpen: false,
  history: [],
  historyIndex: -1,

  pushHistory: () => {
    const { layout, history, historyIndex } = get();
    const newHistory = history.slice(0, historyIndex + 1);
    newHistory.push(JSON.parse(JSON.stringify(layout)));
    if (newHistory.length > 50) newHistory.shift();
    set({ history: newHistory, historyIndex: newHistory.length - 1 });
  },

  addElement: (type, x, y) => {
    const { layout, snapToGrid, gridSize } = get();
    const defaultX = x ?? Math.floor(layout.width / 4);
    const defaultY = y ?? Math.floor(layout.height / 4);
    const snappedX = snapToGrid ? Math.round(defaultX / gridSize) * gridSize : defaultX;
    const snappedY = snapToGrid ? Math.round(defaultY / gridSize) * gridSize : defaultY;
    const el = normalizeElement(defaultElementForType(type, snappedX, snappedY));
    get().pushHistory();
    set({
      layout: { ...layout, elements: [...layout.elements, el] },
      selectedIds: [el.id],
    });
  },

  updateElement: (id, updates) => {
    const { layout } = get();
    get().pushHistory();
    set({
      layout: {
        ...layout,
        elements: layout.elements.map(el =>
          el.id === id ? applyElementUpdate(el, updates) : el
        ),
      },
    });
  },

  updateElementProperty: (id, key, value) => {
    const { layout } = get();
    get().pushHistory();
    set({
      layout: {
        ...layout,
        elements: layout.elements.map(el =>
          el.id === id ? applyPropertyUpdate(el, key, value) : el
        ),
      },
    });
  },

  removeElement: (id) => {
    const { layout, selectedIds } = get();
    get().pushHistory();
    set({
      layout: { ...layout, elements: layout.elements.filter(el => el.id !== id) },
      selectedIds: selectedIds.filter(sid => sid !== id),
    });
  },

  removeSelected: () => {
    const { layout, selectedIds } = get();
    if (selectedIds.length === 0) return;
    get().pushHistory();
    set({
      layout: { ...layout, elements: layout.elements.filter(el => !selectedIds.includes(el.id)) },
      selectedIds: [],
    });
  },

  selectElement: (id, addToSelection = false) => {
    if (id === null) {
      set({ selectedIds: [] });
      return;
    }
    if (addToSelection) {
      const { selectedIds } = get();
      if (selectedIds.includes(id)) {
        set({ selectedIds: selectedIds.filter(sid => sid !== id) });
      } else {
        set({ selectedIds: [...selectedIds, id] });
      }
    } else {
      set({ selectedIds: [id] });
    }
  },

  moveElement: (id, x, y) => {
    const { layout, snapToGrid, gridSize } = get();
    const nx = snapToGrid ? Math.round(x / gridSize) * gridSize : Math.round(x);
    const ny = snapToGrid ? Math.round(y / gridSize) * gridSize : Math.round(y);
    set({
      layout: {
        ...layout,
        elements: layout.elements.map(el =>
          el.id === id ? { ...el, x: nx, y: ny } : el
        ),
      },
    });
  },

  resizeElement: (id, width, height) => {
    const { layout } = get();
    set({
      layout: {
        ...layout,
        elements: layout.elements.map(el =>
          el.id === id
            ? applyElementUpdate(el, {
                width: Math.max(1, Math.round(width)),
                height: Math.max(1, Math.round(height)),
              })
            : el
        ),
      },
    });
  },

  setLayoutName: (name) => {
    set({ layout: { ...get().layout, name } });
  },

  toggleOrientation: () => {
    const { layout } = get();
    const isPortrait = layout.orientation === 'portrait';
    set({
      layout: {
        ...layout,
        orientation: isPortrait ? 'landscape' : 'portrait',
        width: isPortrait ? 320 : 240,
        height: isPortrait ? 240 : 320,
        rotation: isPortrait ? 1 : 0,
      },
    });
  },

  toggleGrid: () => set({ showGrid: !get().showGrid }),
  toggleSnap: () => set({ snapToGrid: !get().snapToGrid }),
  setZoom: (zoom) => set({ zoom: Math.max(0.5, Math.min(4, zoom)) }),
  toggleThemePreview: () => set({ isDarkPreview: !get().isDarkPreview }),

  duplicateSelected: () => {
    const { layout, selectedIds } = get();
    if (selectedIds.length === 0) return;
    get().pushHistory();
    const newElements: LayoutElement[] = [];
    for (const sid of selectedIds) {
      const src = layout.elements.find(el => el.id === sid);
      if (src) {
        newElements.push({ ...src, id: uuidv4(), x: src.x + 10, y: src.y + 10 });
      }
    }
    set({
      layout: { ...layout, elements: [...layout.elements, ...newElements] },
      selectedIds: newElements.map(el => el.id),
    });
  },

  loadLayout: (layout) => {
    set({ layout: normalizeLayout(layout), selectedIds: [], history: [], historyIndex: -1 });
  },

  newLayout: () => {
    set({ layout: createDefaultLayout(), selectedIds: [], history: [], historyIndex: -1 });
  },

  setFlashModalOpen: (open) => set({ flashModalOpen: open }),
  setLayoutListOpen: (open) => set({ layoutListOpen: open }),

  undo: () => {
    const { historyIndex, history, layout } = get();
    if (historyIndex < 0) return;
    // Save current state at the end so redo can restore it
    const newHistory = [...history];
    if (historyIndex === history.length - 1) {
      newHistory.push(JSON.parse(JSON.stringify(layout)));
    }
    const prev = newHistory[historyIndex];
    set({ layout: JSON.parse(JSON.stringify(prev)), historyIndex: historyIndex - 1, history: newHistory });
  },

  redo: () => {
    const { historyIndex, history } = get();
    if (historyIndex + 1 >= history.length) return;
    const next = history[historyIndex + 1];
    set({ layout: JSON.parse(JSON.stringify(next)), historyIndex: historyIndex + 1 });
  },
}));
