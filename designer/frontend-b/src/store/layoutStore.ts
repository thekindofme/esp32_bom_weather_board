import { create } from 'zustand';
import { v4 as uuidv4 } from 'uuid';
import type { Layout, LayoutElement } from '../api/client';
import { applyElementUpdate, applyPropertyUpdate, normalizeElement, normalizeLayout } from './layoutNormalization';

export interface LayoutState {
  layout: Layout;
  selectedId: string | null;
  showGrid: boolean;
  snapToGrid: boolean;
  gridSize: number;
  zoom: number;
  isDarkPreview: boolean;
  flashDialogOpen: boolean;
  propertiesTab: 'position' | 'appearance' | 'data';
  draggingId: string | null;
  dragOffset: { x: number; y: number };
  history: Layout[];
  historyIndex: number;

  addElement: (type: string, overrides?: Partial<LayoutElement>) => void;
  updateElement: (id: string, updates: Partial<LayoutElement>, recordHistory?: boolean) => void;
  updateProp: (id: string, key: string, value: unknown, recordHistory?: boolean) => void;
  removeElement: (id: string) => void;
  selectElement: (id: string | null) => void;
  moveElement: (id: string, x: number, y: number) => void;
  setLayoutName: (name: string) => void;
  toggleOrientation: () => void;
  toggleGrid: () => void;
  toggleSnap: () => void;
  setZoom: (z: number) => void;
  toggleTheme: () => void;
  setFlashDialogOpen: (o: boolean) => void;
  setPropertiesTab: (t: 'position' | 'appearance' | 'data') => void;
  loadLayout: (l: Layout) => void;
  loadTemplate: (template: Pick<Layout, 'name' | 'orientation' | 'width' | 'height' | 'rotation' | 'backgroundColor' | 'elements'>) => void;
  newLayout: () => void;
  startDrag: (id: string, offsetX: number, offsetY: number) => void;
  endDrag: () => void;
  pushHistory: () => void;
  undo: () => void;
  redo: () => void;
}

function blank(): Layout {
  return {
    id: '', name: 'Untitled', orientation: 'portrait',
    width: 240, height: 320, rotation: 0,
    backgroundColor: 'themeBg', elements: [],
  };
}

function defaults(type: string, x: number, y: number): LayoutElement {
  const base = { id: uuidv4(), type, x, y, zIndex: 1 };
  switch (type) {
    case 'data-text':
      return { ...base, width: 120, height: 30, properties: { dataField: 'airTempC', font: 4, textSize: 1, color: 'themeText', bgColor: 'themeBg', align: 'left', prefix: '', suffix: '\u00B0' } };
    case 'static-text':
      return { ...base, width: 80, height: 20, properties: { text: 'Label', font: 2, textSize: 1, color: 'themeText', bgColor: 'themeBg', align: 'left' } };
    case 'weather-icon':
      return { ...base, width: 48, height: 48, properties: { iconSize: 48 } };
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
  layout: blank(), selectedId: null, showGrid: true, snapToGrid: true, gridSize: 10,
  zoom: 1.5, isDarkPreview: true, flashDialogOpen: false,
  propertiesTab: 'position', draggingId: null, dragOffset: { x: 0, y: 0 },
  history: [], historyIndex: -1,

  pushHistory: () => {
    const { layout, history, historyIndex } = get();
    const h = history.slice(0, historyIndex + 1);
    h.push(JSON.parse(JSON.stringify(layout)));
    if (h.length > 50) h.shift();
    set({ history: h, historyIndex: h.length - 1 });
  },

  addElement: (type, overrides) => {
    const { layout, snapToGrid, gridSize } = get();
    let el = defaults(type, 20, 20);
    el = overrides ? applyElementUpdate(el, overrides) : normalizeElement(el);
    if (snapToGrid) { el.x = Math.round(el.x / gridSize) * gridSize; el.y = Math.round(el.y / gridSize) * gridSize; }
    get().pushHistory();
    set({ layout: { ...layout, elements: [...layout.elements, el] }, selectedId: el.id });
  },

  updateElement: (id, updates, recordHistory = false) => {
    const { layout } = get();
    if (recordHistory) get().pushHistory();
    set({ layout: { ...layout, elements: layout.elements.map(e => e.id === id ? applyElementUpdate(e, updates) : e) } });
  },

  updateProp: (id, key, value, recordHistory = false) => {
    const { layout } = get();
    if (recordHistory) get().pushHistory();
    set({ layout: { ...layout, elements: layout.elements.map(e => e.id === id ? applyPropertyUpdate(e, key, value) : e) } });
  },

  removeElement: (id) => {
    const { layout, selectedId } = get();
    get().pushHistory();
    set({ layout: { ...layout, elements: layout.elements.filter(e => e.id !== id) }, selectedId: selectedId === id ? null : selectedId });
  },

  selectElement: (id) => set({ selectedId: id }),

  moveElement: (id, x, y) => {
    const { layout, snapToGrid, gridSize } = get();
    const nx = snapToGrid ? Math.round(x / gridSize) * gridSize : Math.round(x);
    const ny = snapToGrid ? Math.round(y / gridSize) * gridSize : Math.round(y);
    set({ layout: { ...layout, elements: layout.elements.map(e => e.id === id ? { ...e, x: nx, y: ny } : e) } });
  },

  setLayoutName: (name) => set({ layout: { ...get().layout, name } }),
  toggleOrientation: () => {
    const { layout } = get();
    const p = layout.orientation === 'portrait';
    set({ layout: { ...layout, orientation: p ? 'landscape' : 'portrait', width: p ? 320 : 240, height: p ? 240 : 320, rotation: p ? 1 : 0 } });
  },
  toggleGrid: () => set({ showGrid: !get().showGrid }),
  toggleSnap: () => set({ snapToGrid: !get().snapToGrid }),
  setZoom: (z) => set({ zoom: Math.max(0.5, Math.min(4, z)) }),
  toggleTheme: () => set({ isDarkPreview: !get().isDarkPreview }),
  setFlashDialogOpen: (o) => set({ flashDialogOpen: o }),
  setPropertiesTab: (t) => set({ propertiesTab: t }),

  loadLayout: (l) => set({ layout: normalizeLayout(l), selectedId: null, history: [], historyIndex: -1 }),
  loadTemplate: (template) => {
    set({
      layout: normalizeLayout({
        ...blank(),
        name: template.name,
        orientation: template.orientation,
        width: template.width,
        height: template.height,
        rotation: template.rotation,
        backgroundColor: template.backgroundColor,
        elements: template.elements.map(e => ({ ...e, id: uuidv4() })),
      }),
      selectedId: null,
      history: [],
      historyIndex: -1,
    });
  },
  newLayout: () => set({ layout: blank(), selectedId: null, history: [], historyIndex: -1 }),

  startDrag: (id, offsetX, offsetY) => set({ draggingId: id, dragOffset: { x: offsetX, y: offsetY } }),
  endDrag: () => set({ draggingId: null }),

  undo: () => {
    const { historyIndex, history } = get();
    if (historyIndex < 0) return;
    set({ layout: JSON.parse(JSON.stringify(history[historyIndex])), historyIndex: historyIndex - 1 });
  },
  redo: () => {
    const { historyIndex, history } = get();
    if (historyIndex >= history.length - 1) return;
    set({ layout: JSON.parse(JSON.stringify(history[historyIndex + 1])), historyIndex: historyIndex + 1 });
  },
}));
