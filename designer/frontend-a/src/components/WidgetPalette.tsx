import React from 'react';
import { useLayoutStore } from '../store/layoutStore';

interface WidgetDef {
  type: string;
  label: string;
  icon: string;
}

const WIDGET_CATEGORIES: { label: string; widgets: WidgetDef[] }[] = [
  {
    label: 'DATA',
    widgets: [
      { type: 'data-text', label: 'Temperature', icon: '\u{1F321}' },
      { type: 'data-text', label: 'Humidity', icon: '\u{1F4A7}' },
      { type: 'data-text', label: 'Wind', icon: '\u{1F32C}' },
      { type: 'data-text', label: 'Rain Chance', icon: '\u{2614}' },
      { type: 'data-text', label: 'Station', icon: '\u{1F4CD}' },
    ],
  },
  {
    label: 'TIME',
    widgets: [
      { type: 'time', label: 'Clock', icon: '\u{1F552}' },
      { type: 'date', label: 'Date', icon: '\u{1F4C5}' },
    ],
  },
  {
    label: 'COMPOSITE',
    widgets: [
      { type: 'forecast-card', label: 'Forecast', icon: '\u{1F5D3}' },
      { type: 'weather-icon', label: 'Weather Icon', icon: '\u{26C5}' },
    ],
  },
  {
    label: 'SHAPES',
    widgets: [
      { type: 'rect', label: 'Rectangle', icon: '\u{25A1}' },
      { type: 'rounded-rect', label: 'Rounded Rect', icon: '\u{25A2}' },
      { type: 'hline', label: 'H-Line', icon: '\u{2500}' },
      { type: 'vline', label: 'V-Line', icon: '\u{2502}' },
      { type: 'static-text', label: 'Text Label', icon: 'T' },
    ],
  },
];

// Map palette labels to default data fields
const WIDGET_DATA_DEFAULTS: Record<string, Record<string, unknown>> = {
  'Temperature': { dataField: 'airTempC', font: 4, textSize: 2, suffix: '\u00B0' },
  'Humidity': { dataField: 'relHumidityPct', font: 2, textSize: 1, suffix: '%' },
  'Wind': { dataField: 'windKmh', font: 2, textSize: 1, prefix: '', suffix: ' km/h' },
  'Rain Chance': { dataField: 'rainTodayChance', font: 2, textSize: 1 },
  'Station': { dataField: 'stationName', font: 2, textSize: 1 },
};

export const WidgetPalette: React.FC = () => {
  const addElement = useLayoutStore(s => s.addElement);
  const updateElement = useLayoutStore(s => s.updateElement);
  const layout = useLayoutStore(s => s.layout);

  const handleAdd = (widget: WidgetDef) => {
    addElement(widget.type);
    // Apply widget-specific defaults
    const defaults = WIDGET_DATA_DEFAULTS[widget.label];
    if (defaults && widget.type === 'data-text') {
      const lastEl = useLayoutStore.getState().layout.elements;
      const newEl = lastEl[lastEl.length - 1];
      if (newEl) {
        updateElement(newEl.id, { properties: { ...newEl.properties, ...defaults } });
      }
    }
  };

  return (
    <div className="widget-palette">
      <div className="palette-header">
        <span className="palette-title">WIDGETS</span>
        <span className="palette-count">{layout.elements.length}</span>
      </div>
      {WIDGET_CATEGORIES.map(cat => (
        <div key={cat.label} className="palette-category">
          <div className="category-label">{cat.label}</div>
          <div className="category-items">
            {cat.widgets.map((w, i) => (
              <button
                key={`${w.type}-${i}`}
                className="palette-item"
                onClick={() => handleAdd(w)}
                title={`Add ${w.label}`}
              >
                <span className="palette-item-icon">{w.icon}</span>
                <span className="palette-item-label">{w.label}</span>
              </button>
            ))}
          </div>
        </div>
      ))}
    </div>
  );
};
