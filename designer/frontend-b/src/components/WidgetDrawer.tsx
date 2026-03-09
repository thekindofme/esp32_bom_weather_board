import React from 'react';
import { useLayoutStore } from '../store/layoutStore';
import { TemplateGallery } from './TemplateGallery';

interface WidgetDef { type: string; label: string; icon: string; overrides?: Record<string, unknown>; }

const WIDGET_GROUPS: { label: string; items: WidgetDef[] }[] = [
  {
    label: 'Data',
    items: [
      { type: 'data-text', label: 'Temperature', icon: '\u{1F321}', overrides: { dataField: 'airTempC', font: 4, textSize: 2, suffix: '\u00B0' } },
      { type: 'data-text', label: 'Feels Like', icon: '\u{1F912}', overrides: { dataField: 'apparentTempC', font: 2, suffix: '\u00B0' } },
      { type: 'data-text', label: 'Humidity', icon: '\u{1F4A7}', overrides: { dataField: 'relHumidityPct', font: 2, suffix: '%' } },
      { type: 'data-text', label: 'Wind', icon: '\u{1F32C}', overrides: { dataField: 'windKmh', font: 2, suffix: ' km/h' } },
      { type: 'data-text', label: 'Rain Chance', icon: '\u{2614}', overrides: { dataField: 'rainTodayChance', font: 2 } },
      { type: 'data-text', label: 'Rain Range', icon: '\u{1F327}', overrides: { dataField: 'rainTodayRange', font: 2 } },
      { type: 'data-text', label: 'Station', icon: '\u{1F4CD}', overrides: { dataField: 'stationName', font: 2 } },
    ],
  },
  {
    label: 'Time',
    items: [
      { type: 'time', label: 'Clock', icon: '\u{1F552}' },
      { type: 'date', label: 'Date', icon: '\u{1F4C5}' },
    ],
  },
  {
    label: 'Composite',
    items: [
      { type: 'forecast-card', label: '3-Day Forecast', icon: '\u{1F5D3}' },
      { type: 'weather-icon', label: 'Weather Icon', icon: '\u{26C5}' },
    ],
  },
  {
    label: 'Decorative',
    items: [
      { type: 'rect', label: 'Rectangle', icon: '\u{25A1}' },
      { type: 'rounded-rect', label: 'Rounded Rect', icon: '\u{25A2}' },
      { type: 'hline', label: 'Horizontal Line', icon: '\u{2500}' },
      { type: 'vline', label: 'Vertical Line', icon: '\u{2502}' },
      { type: 'static-text', label: 'Text Label', icon: 'Aa' },
    ],
  },
];

export const WidgetDrawer: React.FC = () => {
  const addElement = useLayoutStore(s => s.addElement);
  const updateProp = useLayoutStore(s => s.updateProp);

  const handleAdd = (w: WidgetDef) => {
    addElement(w.type);
    if (w.overrides) {
      const state = useLayoutStore.getState();
      const lastEl = state.layout.elements[state.layout.elements.length - 1];
      if (lastEl) {
        Object.entries(w.overrides).forEach(([k, v]) => updateProp(lastEl.id, k, v));
      }
    }
  };

  return (
    <div className="widget-drawer">
      <TemplateGallery />

      <div className="drawer-divider" />

      {WIDGET_GROUPS.map(g => (
        <div key={g.label} className="widget-group">
          <div className="drawer-section-label">{g.label}</div>
          <div className="widget-grid">
            {g.items.map((w, i) => (
              <button key={`${w.type}-${i}`} className="widget-btn" onClick={() => handleAdd(w)}>
                <span className="widget-btn-icon">{w.icon}</span>
                <span className="widget-btn-label">{w.label}</span>
              </button>
            ))}
          </div>
        </div>
      ))}
    </div>
  );
};
