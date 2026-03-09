import React from 'react';
import type { LayoutElement } from '../api/client';
import { useLayoutStore } from '../store/layoutStore';

interface Template {
  name: string;
  description: string;
  icon: string;
  builtIn: boolean;
  orientation: 'portrait' | 'landscape';
  width: number;
  height: number;
  elements: LayoutElement[];
}

// All 5 built-in layouts from the ESP32 firmware, faithfully reproduced
const BUILT_IN_TEMPLATES: Template[] = [
  {
    name: 'Hero Temp',
    description: 'Temperature-dominant, readable from across the room',
    icon: '\u{1F321}\u{FE0F}',
    builtIn: true,
    orientation: 'portrait', width: 240, height: 320,
    elements: [
      { id: 'ht-1', type: 'rect', x: 0, y: 0, width: 240, height: 28, zIndex: 0, properties: { fillColor: 'themeHeader', strokeColor: 'none', cornerRadius: 0 } },
      { id: 'ht-2', type: 'hline', x: 0, y: 28, width: 240, height: 1, zIndex: 1, properties: { color: 'themeEdge' } },
      { id: 'ht-3', type: 'date', x: 8, y: 8, width: 100, height: 16, zIndex: 2, properties: { font: 2, color: 'themeGood', bgColor: 'themeHeader' } },
      { id: 'ht-4', type: 'time', x: 130, y: 8, width: 80, height: 16, zIndex: 2, properties: { font: 2, color: 'themeGood', bgColor: 'themeHeader' } },
      { id: 'ht-5', type: 'data-text', x: 14, y: 34, width: 160, height: 16, zIndex: 3, properties: { dataField: 'stationName', font: 2, textSize: 1, color: 'themeText', bgColor: 'themeBg', prefix: '', suffix: '' } },
      { id: 'ht-6', type: 'weather-icon', x: 180, y: 34, width: 48, height: 48, zIndex: 3, properties: { iconSize: 48 } },
      { id: 'ht-7', type: 'data-text', x: 14, y: 58, width: 160, height: 52, zIndex: 4, properties: { dataField: 'airTempC', font: 4, textSize: 2, color: '#D2C478', bgColor: 'themeBg', prefix: '', suffix: '\u00B0' } },
      { id: 'ht-8', type: 'data-text', x: 14, y: 108, width: 160, height: 16, zIndex: 4, properties: { dataField: 'apparentTempC', font: 2, textSize: 1, color: 'themeAccent', bgColor: 'themeBg', prefix: 'Feels ', suffix: '\u00B0' } },
      { id: 'ht-9', type: 'data-text', x: 14, y: 126, width: 100, height: 10, zIndex: 4, properties: { dataField: 'dayMinTempC', font: 1, textSize: 1, color: 'themeTextMuted', bgColor: 'themeBg', prefix: '', suffix: '' } },
      { id: 'ht-10', type: 'hline', x: 8, y: 138, width: 224, height: 1, zIndex: 5, properties: { color: 'themeEdge' } },
      { id: 'ht-11', type: 'static-text', x: 14, y: 142, width: 40, height: 16, zIndex: 5, properties: { text: 'RAIN', font: 2, textSize: 1, color: 'themeText', bgColor: 'themeBg' } },
      { id: 'ht-12', type: 'data-text', x: 62, y: 142, width: 60, height: 16, zIndex: 5, properties: { dataField: 'rainTodayChance', font: 2, textSize: 1, color: 'themeAccent', bgColor: 'themeBg', prefix: '', suffix: '' } },
      { id: 'ht-13', type: 'data-text', x: 130, y: 142, width: 100, height: 16, zIndex: 5, properties: { dataField: 'rainTodayRange', font: 2, textSize: 1, color: 'themeTextMuted', bgColor: 'themeBg', prefix: '', suffix: '' } },
      { id: 'ht-14', type: 'data-text', x: 14, y: 174, width: 150, height: 16, zIndex: 6, properties: { dataField: 'windKmh', font: 2, textSize: 1, color: 'themeText', bgColor: 'themeBg', prefix: 'WIND ', suffix: ' km/h' } },
      { id: 'ht-15', type: 'data-text', x: 170, y: 174, width: 60, height: 16, zIndex: 6, properties: { dataField: 'relHumidityPct', font: 2, textSize: 1, color: 'themeText', bgColor: 'themeBg', prefix: 'HUM ', suffix: '%' } },
      { id: 'ht-16', type: 'hline', x: 8, y: 194, width: 224, height: 1, zIndex: 7, properties: { color: 'themeEdge' } },
      { id: 'ht-17', type: 'static-text', x: 8, y: 198, width: 100, height: 16, zIndex: 7, properties: { text: 'Next 3 Days', font: 2, textSize: 1, color: 'themeAccent', bgColor: 'themeBg' } },
      { id: 'ht-18', type: 'forecast-card', x: 4, y: 218, width: 232, height: 58, zIndex: 8, properties: { cardWidth: 76, cardHeight: 58, cardSpacing: 2, cornerRadius: 6, cardBg: 'themePanel', cardBorder: 'themeEdge', labelColor: 'themeAccent', tempColor: 'themeText', rainColor: 'themeTextMuted' } },
      { id: 'ht-19', type: 'data-text', x: 8, y: 310, width: 200, height: 10, zIndex: 9, properties: { dataField: 'observedTimeLocal', font: 1, textSize: 1, color: 'themeTextMuted', bgColor: 'themeBg', prefix: 'Obs ', suffix: '' } },
    ],
  },
  {
    name: 'Rain First',
    description: 'Rain gets a hero band - rain chance front and center',
    icon: '\u{1F327}\u{FE0F}',
    builtIn: true,
    orientation: 'portrait', width: 240, height: 320,
    elements: [
      { id: 'rf-1', type: 'rect', x: 0, y: 0, width: 240, height: 28, zIndex: 0, properties: { fillColor: 'themeHeader', strokeColor: 'none', cornerRadius: 0 } },
      { id: 'rf-2', type: 'hline', x: 0, y: 28, width: 240, height: 1, zIndex: 1, properties: { color: 'themeEdge' } },
      { id: 'rf-3', type: 'date', x: 8, y: 8, width: 100, height: 16, zIndex: 2, properties: { font: 2, color: 'themeGood', bgColor: 'themeHeader' } },
      { id: 'rf-4', type: 'time', x: 130, y: 8, width: 80, height: 16, zIndex: 2, properties: { font: 2, color: 'themeGood', bgColor: 'themeHeader' } },
      { id: 'rf-5', type: 'data-text', x: 14, y: 32, width: 130, height: 16, zIndex: 3, properties: { dataField: 'stationName', font: 2, textSize: 1, color: 'themeText', bgColor: 'themeBg', prefix: '', suffix: '' } },
      { id: 'rf-6', type: 'weather-icon', x: 150, y: 30, width: 36, height: 36, zIndex: 3, properties: { iconSize: 36 } },
      { id: 'rf-7', type: 'data-text', x: 194, y: 32, width: 44, height: 26, zIndex: 3, properties: { dataField: 'airTempC', font: 4, textSize: 1, color: '#D2C478', bgColor: 'themeBg', prefix: '', suffix: '\u00B0' } },
      { id: 'rf-8', type: 'data-text', x: 14, y: 52, width: 120, height: 16, zIndex: 4, properties: { dataField: 'apparentTempC', font: 2, textSize: 1, color: 'themeAccent', bgColor: 'themeBg', prefix: 'Feels ', suffix: '\u00B0' } },
      { id: 'rf-9', type: 'rect', x: 0, y: 74, width: 240, height: 96, zIndex: 5, properties: { fillColor: 'themePanel', strokeColor: 'none', cornerRadius: 0 } },
      { id: 'rf-10', type: 'hline', x: 0, y: 74, width: 240, height: 1, zIndex: 6, properties: { color: 'themeEdge' } },
      { id: 'rf-11', type: 'hline', x: 0, y: 169, width: 240, height: 1, zIndex: 6, properties: { color: 'themeEdge' } },
      { id: 'rf-12', type: 'static-text', x: 10, y: 80, width: 100, height: 16, zIndex: 7, properties: { text: 'RAIN TODAY', font: 2, textSize: 1, color: 'themeText', bgColor: 'themePanel' } },
      { id: 'rf-13', type: 'data-text', x: 170, y: 76, width: 60, height: 26, zIndex: 7, properties: { dataField: 'rainTodayChance', font: 4, textSize: 1, color: 'themeAccent', bgColor: 'themePanel', prefix: '', suffix: '' } },
      { id: 'rf-14', type: 'rounded-rect', x: 10, y: 102, width: 106, height: 38, zIndex: 7, properties: { fillColor: 'themePanel', strokeColor: 'themeEdge', cornerRadius: 5 } },
      { id: 'rf-15', type: 'static-text', x: 16, y: 106, width: 60, height: 10, zIndex: 8, properties: { text: 'MORNING', font: 1, textSize: 1, color: 'themeAccent', bgColor: 'themePanel' } },
      { id: 'rf-16', type: 'data-text', x: 16, y: 118, width: 90, height: 16, zIndex: 8, properties: { dataField: 'rainMorningRange', font: 2, textSize: 1, color: 'themeText', bgColor: 'themePanel', prefix: '', suffix: '' } },
      { id: 'rf-17', type: 'rounded-rect', x: 124, y: 102, width: 106, height: 38, zIndex: 7, properties: { fillColor: 'themePanel', strokeColor: 'themeEdge', cornerRadius: 5 } },
      { id: 'rf-18', type: 'static-text', x: 130, y: 106, width: 70, height: 10, zIndex: 8, properties: { text: 'AFTERNOON', font: 1, textSize: 1, color: 'themeAccent', bgColor: 'themePanel' } },
      { id: 'rf-19', type: 'data-text', x: 130, y: 118, width: 90, height: 16, zIndex: 8, properties: { dataField: 'rainEveningRange', font: 2, textSize: 1, color: 'themeText', bgColor: 'themePanel', prefix: '', suffix: '' } },
      { id: 'rf-20', type: 'data-text', x: 10, y: 178, width: 160, height: 16, zIndex: 9, properties: { dataField: 'windKmh', font: 2, textSize: 1, color: 'themeText', bgColor: 'themeBg', prefix: 'WIND ', suffix: ' km/h' } },
      { id: 'rf-21', type: 'data-text', x: 176, y: 178, width: 60, height: 16, zIndex: 9, properties: { dataField: 'relHumidityPct', font: 2, textSize: 1, color: 'themeText', bgColor: 'themeBg', prefix: 'HUM ', suffix: '%' } },
      { id: 'rf-22', type: 'data-text', x: 8, y: 310, width: 200, height: 10, zIndex: 10, properties: { dataField: 'observedTimeLocal', font: 1, textSize: 1, color: 'themeTextMuted', bgColor: 'themeBg', prefix: 'Obs ', suffix: '' } },
    ],
  },
  {
    name: 'HUD Grid',
    description: 'Maximum density instrument panel, pure grid lines',
    icon: '\u{1F4CA}',
    builtIn: true,
    orientation: 'portrait', width: 240, height: 320,
    elements: [
      { id: 'hg-1', type: 'date', x: 6, y: 6, width: 100, height: 16, zIndex: 2, properties: { font: 2, color: 'themeGood', bgColor: 'themeBg' } },
      { id: 'hg-2', type: 'time', x: 130, y: 6, width: 80, height: 16, zIndex: 2, properties: { font: 2, color: 'themeAccent', bgColor: 'themeBg' } },
      { id: 'hg-3', type: 'hline', x: 0, y: 28, width: 240, height: 1, zIndex: 1, properties: { color: 'themeEdge' } },
      { id: 'hg-4', type: 'vline', x: 96, y: 29, width: 1, height: 84, zIndex: 1, properties: { color: 'themeEdge' } },
      { id: 'hg-5', type: 'data-text', x: 8, y: 35, width: 86, height: 26, zIndex: 3, properties: { dataField: 'airTempC', font: 4, textSize: 1, color: '#D2C478', bgColor: 'themeBg', prefix: '', suffix: '\u00B0C' } },
      { id: 'hg-6', type: 'weather-icon', x: 20, y: 71, width: 36, height: 36, zIndex: 3, properties: { iconSize: 36 } },
      { id: 'hg-7', type: 'data-text', x: 102, y: 33, width: 130, height: 16, zIndex: 3, properties: { dataField: 'stationName', font: 2, textSize: 1, color: 'themeText', bgColor: 'themeBg', prefix: '', suffix: '' } },
      { id: 'hg-8', type: 'data-text', x: 102, y: 53, width: 130, height: 16, zIndex: 3, properties: { dataField: 'apparentTempC', font: 2, textSize: 1, color: 'themeAccent', bgColor: 'themeBg', prefix: 'Feels ', suffix: '\u00B0C' } },
      { id: 'hg-9', type: 'data-text', x: 102, y: 77, width: 130, height: 16, zIndex: 3, properties: { dataField: 'dayMaxTempC', font: 2, textSize: 1, color: 'themeTextMuted', bgColor: 'themeBg', prefix: 'Hi ', suffix: '\u00B0' } },
      { id: 'hg-10', type: 'hline', x: 0, y: 113, width: 240, height: 1, zIndex: 1, properties: { color: 'themeEdge' } },
      { id: 'hg-11', type: 'vline', x: 80, y: 114, width: 1, height: 52, zIndex: 1, properties: { color: 'themeEdge' } },
      { id: 'hg-12', type: 'vline', x: 160, y: 114, width: 1, height: 52, zIndex: 1, properties: { color: 'themeEdge' } },
      { id: 'hg-13', type: 'static-text', x: 6, y: 118, width: 30, height: 10, zIndex: 4, properties: { text: 'RAIN', font: 1, textSize: 1, color: 'themeTextMuted', bgColor: 'themeBg' } },
      { id: 'hg-14', type: 'data-text', x: 6, y: 130, width: 70, height: 16, zIndex: 4, properties: { dataField: 'rainTodayChance', font: 2, textSize: 1, color: 'themeAccent', bgColor: 'themeBg', prefix: '', suffix: '' } },
      { id: 'hg-15', type: 'static-text', x: 86, y: 118, width: 30, height: 10, zIndex: 4, properties: { text: 'WIND', font: 1, textSize: 1, color: 'themeTextMuted', bgColor: 'themeBg' } },
      { id: 'hg-16', type: 'data-text', x: 86, y: 130, width: 70, height: 16, zIndex: 4, properties: { dataField: 'windDir', font: 2, textSize: 1, color: 'themeText', bgColor: 'themeBg', prefix: '', suffix: '' } },
      { id: 'hg-17', type: 'static-text', x: 166, y: 118, width: 30, height: 10, zIndex: 4, properties: { text: 'HUM', font: 1, textSize: 1, color: 'themeTextMuted', bgColor: 'themeBg' } },
      { id: 'hg-18', type: 'data-text', x: 166, y: 130, width: 70, height: 16, zIndex: 4, properties: { dataField: 'relHumidityPct', font: 2, textSize: 1, color: 'themeText', bgColor: 'themeBg', prefix: '', suffix: '%' } },
      { id: 'hg-19', type: 'hline', x: 0, y: 166, width: 240, height: 1, zIndex: 1, properties: { color: 'themeEdge' } },
      { id: 'hg-20', type: 'vline', x: 120, y: 167, width: 1, height: 36, zIndex: 1, properties: { color: 'themeEdge' } },
      { id: 'hg-21', type: 'static-text', x: 6, y: 171, width: 50, height: 10, zIndex: 5, properties: { text: 'AM RAIN', font: 1, textSize: 1, color: 'themeTextMuted', bgColor: 'themeBg' } },
      { id: 'hg-22', type: 'data-text', x: 6, y: 183, width: 110, height: 16, zIndex: 5, properties: { dataField: 'rainMorningRange', font: 2, textSize: 1, color: 'themeText', bgColor: 'themeBg', prefix: '', suffix: '' } },
      { id: 'hg-23', type: 'static-text', x: 126, y: 171, width: 50, height: 10, zIndex: 5, properties: { text: 'PM RAIN', font: 1, textSize: 1, color: 'themeTextMuted', bgColor: 'themeBg' } },
      { id: 'hg-24', type: 'data-text', x: 126, y: 183, width: 110, height: 16, zIndex: 5, properties: { dataField: 'rainEveningRange', font: 2, textSize: 1, color: 'themeText', bgColor: 'themeBg', prefix: '', suffix: '' } },
      { id: 'hg-25', type: 'hline', x: 0, y: 203, width: 240, height: 1, zIndex: 1, properties: { color: 'themeEdge' } },
      { id: 'hg-26', type: 'forecast-card', x: 0, y: 220, width: 240, height: 58, zIndex: 6, properties: { cardWidth: 76, cardHeight: 58, cardSpacing: 4, cornerRadius: 0, cardBg: 'themeBg', cardBorder: 'themeEdge', labelColor: 'themeAccent', tempColor: 'themeText', rainColor: 'themeTextMuted' } },
      { id: 'hg-27', type: 'data-text', x: 6, y: 308, width: 200, height: 10, zIndex: 7, properties: { dataField: 'observedTimeLocal', font: 1, textSize: 1, color: 'themeTextMuted', bgColor: 'themeBg', prefix: 'Obs ', suffix: '' } },
    ],
  },
  {
    name: 'Nightwatch',
    description: 'Red-on-black, minimal, zero light pollution. 3 bands.',
    icon: '\u{1F319}',
    builtIn: true,
    orientation: 'portrait', width: 240, height: 320,
    elements: [
      { id: 'nw-1', type: 'time', x: 40, y: 8, width: 160, height: 26, zIndex: 1, properties: { font: 4, color: '#B40000', bgColor: '#000000', format: '12h', showSeconds: false } },
      { id: 'nw-2', type: 'date', x: 40, y: 42, width: 160, height: 26, zIndex: 1, properties: { font: 4, color: '#500000', bgColor: '#000000' } },
      { id: 'nw-3', type: 'hline', x: 0, y: 80, width: 240, height: 1, zIndex: 1, properties: { color: '#280000' } },
      { id: 'nw-4', type: 'data-text', x: 20, y: 91, width: 200, height: 52, zIndex: 2, properties: { dataField: 'airTempC', font: 4, textSize: 2, color: '#B40000', bgColor: '#000000', prefix: '', suffix: '\u00B0' } },
      { id: 'nw-5', type: 'data-text', x: 30, y: 163, width: 180, height: 26, zIndex: 2, properties: { dataField: 'apparentTempC', font: 4, textSize: 1, color: '#500000', bgColor: '#000000', prefix: 'feels ', suffix: '\u00B0' } },
      { id: 'nw-6', type: 'hline', x: 0, y: 211, width: 240, height: 1, zIndex: 1, properties: { color: '#280000' } },
      { id: 'nw-7', type: 'data-text', x: 20, y: 222, width: 200, height: 52, zIndex: 3, properties: { dataField: 'rainTodayChance', font: 4, textSize: 2, color: '#C81E00', bgColor: '#000000', prefix: '', suffix: '' } },
      { id: 'nw-8', type: 'data-text', x: 50, y: 284, width: 140, height: 16, zIndex: 3, properties: { dataField: 'rainTodayRange', font: 2, textSize: 1, color: '#500000', bgColor: '#000000', prefix: '', suffix: '' } },
    ],
  },
  {
    name: 'Nightwatch Wide',
    description: 'Landscape red-on-black. Time left, data right.',
    icon: '\u{1F30C}',
    builtIn: true,
    orientation: 'landscape', width: 320, height: 240,
    elements: [
      { id: 'nww-1', type: 'vline', x: 160, y: 0, width: 1, height: 240, zIndex: 1, properties: { color: '#280000' } },
      { id: 'nww-2', type: 'hline', x: 160, y: 130, width: 160, height: 1, zIndex: 1, properties: { color: '#280000' } },
      { id: 'nww-3', type: 'time', x: 20, y: 80, width: 120, height: 26, zIndex: 2, properties: { font: 4, color: '#B40000', bgColor: '#000000', format: '12h', showSeconds: false } },
      { id: 'nww-4', type: 'date', x: 20, y: 124, width: 120, height: 26, zIndex: 2, properties: { font: 4, color: '#500000', bgColor: '#000000' } },
      { id: 'nww-5', type: 'data-text', x: 170, y: 16, width: 140, height: 52, zIndex: 3, properties: { dataField: 'airTempC', font: 4, textSize: 2, color: '#B40000', bgColor: '#000000', prefix: '', suffix: '\u00B0' } },
      { id: 'nww-6', type: 'data-text', x: 175, y: 82, width: 130, height: 26, zIndex: 3, properties: { dataField: 'apparentTempC', font: 4, textSize: 1, color: '#500000', bgColor: '#000000', prefix: 'feels ', suffix: '\u00B0' } },
      { id: 'nww-7', type: 'data-text', x: 170, y: 144, width: 140, height: 52, zIndex: 4, properties: { dataField: 'rainTodayChance', font: 4, textSize: 2, color: '#C81E00', bgColor: '#000000', prefix: '', suffix: '' } },
      { id: 'nww-8', type: 'data-text', x: 185, y: 208, width: 120, height: 16, zIndex: 4, properties: { dataField: 'rainTodayRange', font: 2, textSize: 1, color: '#500000', bgColor: '#000000', prefix: '', suffix: '' } },
    ],
  },
];

const CUSTOM_TEMPLATES: Template[] = [
  {
    name: 'Blank Canvas',
    description: 'Start from scratch with an empty layout',
    icon: '\u{2795}',
    builtIn: false,
    orientation: 'portrait', width: 240, height: 320,
    elements: [],
  },
];

export const TemplateGallery: React.FC = () => {
  const loadTemplate = useLayoutStore(s => s.loadTemplate);

  const handleLoadTemplate = (t: Template) => {
    loadTemplate({
      name: t.name + ' (copy)',
      orientation: t.orientation,
      width: t.width,
      height: t.height,
      rotation: t.orientation === 'landscape' ? 1 : 0,
      backgroundColor: 'themeBg',
      elements: t.elements,
    });
  };

  return (
    <div className="template-gallery">
      <div className="drawer-section-label">Built-in Layouts</div>
      <div className="template-cards">
        {BUILT_IN_TEMPLATES.map(t => (
          <button key={t.name} className="template-card" onClick={() => handleLoadTemplate(t)}>
            <span className="template-icon">{t.icon}</span>
            <div className="template-info">
              <div className="template-name">
                {t.name}
                <span className="template-badge">firmware</span>
              </div>
              <div className="template-desc">{t.description}</div>
              <div className="template-meta">{t.width}x{t.height} {t.orientation}</div>
            </div>
          </button>
        ))}
      </div>

      <div className="drawer-divider" />

      <div className="drawer-section-label">Start Fresh</div>
      <div className="template-cards">
        {CUSTOM_TEMPLATES.map(t => (
          <button key={t.name} className="template-card" onClick={() => handleLoadTemplate(t)}>
            <span className="template-icon">{t.icon}</span>
            <div className="template-info">
              <div className="template-name">{t.name}</div>
              <div className="template-desc">{t.description}</div>
            </div>
          </button>
        ))}
      </div>
    </div>
  );
};
