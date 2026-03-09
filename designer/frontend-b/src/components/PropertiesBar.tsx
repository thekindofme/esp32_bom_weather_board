import React from 'react';
import { useLayoutStore } from '../store/layoutStore';
import { THEME_SWATCHES, DARK_THEME, LIGHT_THEME, type ThemeColorName } from '../utils/themeColors';
import { DATA_FIELDS, FONT_METRICS, TEXT_SIZE_MAX, TEXT_SIZE_MIN } from '../utils/fontMetrics';

function ColorField({ label, value, onChange, isDark }: { label: string; value: string; onChange: (v: string) => void; isDark: boolean }) {
  const theme = isDark ? DARK_THEME : LIGHT_THEME;
  return (
    <div className="pf">
      <label>{label}</label>
      <div className="pf-swatches">
        {THEME_SWATCHES.map(s => (
          <button key={s.name} className={`pf-swatch ${value === s.name ? 'active' : ''}`}
            style={{ backgroundColor: theme[s.name] }} onClick={() => onChange(s.name)} title={s.label} />
        ))}
        <input type="color" className="pf-color-input"
          value={value.startsWith('#') ? value : '#888888'}
          onChange={e => onChange(e.target.value)} />
      </div>
    </div>
  );
}

function clampTextSize(value: number): number {
  if (!Number.isFinite(value)) return TEXT_SIZE_MIN;
  return Math.max(TEXT_SIZE_MIN, Math.min(TEXT_SIZE_MAX, Math.round(value)));
}

export const PropertiesBar: React.FC = () => {
  const { selectedId, layout, propertiesTab, setPropertiesTab, updateElement, updateProp, removeElement, isDarkPreview, snapToGrid, gridSize } = useLayoutStore();

  const el = layout.elements.find(e => e.id === selectedId);

  if (!el) {
    return (
      <div className="properties-bar">
        <div className="props-empty">Select an element to edit its properties</div>
      </div>
    );
  }

  const p = el.properties as Record<string, unknown>;
  const fontOptions = Object.entries(FONT_METRICS).map(([k, v]) => ({ value: k, label: v.label }));
  const defaultTextColor = el.type === 'time' || el.type === 'date' ? 'themeGood' : 'themeText';
  const defaultBgColor = el.type === 'time' || el.type === 'date' ? 'themeHeader' : 'themeBg';
  const snapCoordinate = (value: number) => {
    const next = Math.max(0, value);
    return snapToGrid ? Math.round(next / gridSize) * gridSize : Math.round(next);
  };
  const centerElement = (axis: 'horizontal' | 'vertical') => {
    if (axis === 'horizontal') {
      updateElement(el.id, { x: snapCoordinate((layout.width - el.width) / 2) }, true);
      return;
    }
    updateElement(el.id, { y: snapCoordinate((layout.height - el.height) / 2) }, true);
  };

  return (
    <div className="properties-bar">
      <div className="props-tabs">
        {(['position', 'appearance', 'data'] as const).map(t => (
          <button key={t} className={`props-tab ${propertiesTab === t ? 'active' : ''}`}
            onClick={() => setPropertiesTab(t)}>{t.charAt(0).toUpperCase() + t.slice(1)}</button>
        ))}
        <div className="props-tab-spacer" />
        <span className="props-type-badge">{el.type}</span>
        <button className="props-delete" onClick={() => removeElement(el.id)}>Delete</button>
      </div>

      <div className="props-content">
        {propertiesTab === 'position' && (
            <div className="props-grid">
              <div className="pf">
                <label>X</label>
                <input type="number" value={el.x} onChange={e => updateElement(el.id, { x: Number(e.target.value) }, true)} />
              </div>
              <div className="pf">
                <label>Y</label>
                <input type="number" value={el.y} onChange={e => updateElement(el.id, { y: Number(e.target.value) }, true)} />
              </div>
              <div className="pf">
                <label>Width</label>
                <input type="number" value={el.width} onChange={e => updateElement(el.id, { width: Number(e.target.value) }, true)} min={1} />
              </div>
              <div className="pf">
                <label>Height</label>
                <input type="number" value={el.height} onChange={e => updateElement(el.id, { height: Number(e.target.value) }, true)} min={1} />
              </div>
              <div className="pf">
                <label>Z-Index</label>
                <input type="number" value={el.zIndex} onChange={e => updateElement(el.id, { zIndex: Number(e.target.value) }, true)} />
              </div>
              <div className="pf-axis-actions">
                <button className="btn-outline btn-sm" onClick={() => centerElement('horizontal')}>Center H</button>
                <button className="btn-outline btn-sm" onClick={() => centerElement('vertical')}>Center V</button>
              </div>
            </div>
        )}

        {propertiesTab === 'appearance' && (
          <div className="props-grid">
            {(el.type === 'data-text' || el.type === 'static-text' || el.type === 'time' || el.type === 'date') && (
              <>
                <div className="pf">
                  <label>Font</label>
                  <select value={(p.font as number) || 2} onChange={e => updateProp(el.id, 'font', Number(e.target.value), true)}>
                    {fontOptions.map(o => <option key={o.value} value={o.value}>{o.label}</option>)}
                  </select>
                </div>
                <div className="pf">
                  <label>Size</label>
                  <input
                    type="number"
                    value={(p.textSize as number) || 1}
                    onChange={e => updateProp(el.id, 'textSize', clampTextSize(Number(e.target.value)), true)}
                    min={TEXT_SIZE_MIN}
                    max={TEXT_SIZE_MAX}
                  />
                </div>
                <ColorField label="Color" value={(p.color as string) || defaultTextColor} onChange={v => updateProp(el.id, 'color', v, true)} isDark={isDarkPreview} />
                <ColorField label="Bg" value={(p.bgColor as string) || defaultBgColor} onChange={v => updateProp(el.id, 'bgColor', v, true)} isDark={isDarkPreview} />
              </>
            )}
            {(el.type === 'data-text' || el.type === 'static-text') && (
              <div className="pf">
                <label>Align</label>
                <select value={(p.align as string) || 'left'} onChange={e => updateProp(el.id, 'align', e.target.value, true)}>
                  <option value="left">Left</option>
                  <option value="center">Center</option>
                  <option value="right">Right</option>
                </select>
              </div>
            )}
            {el.type === 'time' && (
              <>
                <div className="pf">
                  <label>Format</label>
                  <select value={(p.format as string) || '12h'} onChange={e => updateProp(el.id, 'format', e.target.value, true)}>
                    <option value="12h">12-hour</option>
                    <option value="24h">24-hour</option>
                  </select>
                </div>
                <div className="pf">
                  <label>Seconds</label>
                  <select value={(p.showSeconds as boolean) === false ? 'false' : 'true'} onChange={e => updateProp(el.id, 'showSeconds', e.target.value === 'true', true)}>
                    <option value="true">Show</option>
                    <option value="false">Hide</option>
                  </select>
                </div>
              </>
            )}
            {el.type === 'date' && (
              <div className="pf">
                <label>Date Format</label>
                <select value={(p.dateFormat as string) || 'short'} onChange={e => updateProp(el.id, 'dateFormat', e.target.value, true)}>
                  <option value="short">Short (Sat 08 Mar)</option>
                  <option value="long">Long (Saturday 8 March)</option>
                  <option value="numeric">Numeric (08/03/2026)</option>
                </select>
              </div>
            )}
            {(el.type === 'rect' || el.type === 'rounded-rect') && (
              <>
                <ColorField label="Fill" value={(p.fillColor as string) || 'themePanel'} onChange={v => updateProp(el.id, 'fillColor', v, true)} isDark={isDarkPreview} />
                <ColorField label="Stroke" value={(p.strokeColor as string) || 'themeEdge'} onChange={v => updateProp(el.id, 'strokeColor', v, true)} isDark={isDarkPreview} />
                <div className="pf">
                  <label>Radius</label>
                  <input type="number" value={(p.cornerRadius as number) || 0} onChange={e => updateProp(el.id, 'cornerRadius', Number(e.target.value), true)} min={0} />
                </div>
              </>
            )}
            {(el.type === 'hline' || el.type === 'vline') && (
              <ColorField label="Color" value={(p.color as string) || 'themeEdge'} onChange={v => updateProp(el.id, 'color', v, true)} isDark={isDarkPreview} />
            )}
            {el.type === 'forecast-card' && (
              <>
                <div className="pf"><label>Card W</label><input type="number" value={(p.cardWidth as number) || 76} onChange={e => updateProp(el.id, 'cardWidth', Number(e.target.value), true)} /></div>
                <div className="pf"><label>Card H</label><input type="number" value={(p.cardHeight as number) || 58} onChange={e => updateProp(el.id, 'cardHeight', Number(e.target.value), true)} /></div>
                <div className="pf"><label>Spacing</label><input type="number" value={(p.cardSpacing as number) || 2} onChange={e => updateProp(el.id, 'cardSpacing', Number(e.target.value), true)} /></div>
                <div className="pf"><label>Radius</label><input type="number" value={(p.cornerRadius as number) || 6} onChange={e => updateProp(el.id, 'cornerRadius', Number(e.target.value), true)} /></div>
                <ColorField label="Card Bg" value={(p.cardBg as string) || 'themePanel'} onChange={v => updateProp(el.id, 'cardBg', v, true)} isDark={isDarkPreview} />
                <ColorField label="Border" value={(p.cardBorder as string) || 'themeEdge'} onChange={v => updateProp(el.id, 'cardBorder', v, true)} isDark={isDarkPreview} />
                <ColorField label="Labels" value={(p.labelColor as string) || 'themeAccent'} onChange={v => updateProp(el.id, 'labelColor', v, true)} isDark={isDarkPreview} />
                <ColorField label="Temps" value={(p.tempColor as string) || 'themeText'} onChange={v => updateProp(el.id, 'tempColor', v, true)} isDark={isDarkPreview} />
                <ColorField label="Rain" value={(p.rainColor as string) || 'themeTextMuted'} onChange={v => updateProp(el.id, 'rainColor', v, true)} isDark={isDarkPreview} />
              </>
            )}
            {el.type === 'weather-icon' && (
              <div className="pf"><label>Size</label><input type="number" value={(p.iconSize as number) || 48} onChange={e => updateProp(el.id, 'iconSize', Number(e.target.value), true)} /></div>
            )}
          </div>
        )}

        {propertiesTab === 'data' && (
          <div className="props-grid">
            {el.type === 'data-text' && (
              <>
                <div className="pf">
                  <label>Data Field</label>
                  <select value={(p.dataField as string) || 'airTempC'} onChange={e => updateProp(el.id, 'dataField', e.target.value, true)}>
                    {DATA_FIELDS.map(f => <option key={f.value} value={f.value}>{f.label}</option>)}
                  </select>
                </div>
                <div className="pf">
                  <label>Prefix</label>
                  <input type="text" value={(p.prefix as string) || ''} onChange={e => updateProp(el.id, 'prefix', e.target.value, true)} />
                </div>
                <div className="pf">
                  <label>Suffix</label>
                  <input type="text" value={(p.suffix as string) || ''} onChange={e => updateProp(el.id, 'suffix', e.target.value, true)} />
                </div>
              </>
            )}
            {el.type === 'static-text' && (
              <div className="pf">
                <label>Text</label>
                <input type="text" value={(p.text as string) || ''} onChange={e => updateProp(el.id, 'text', e.target.value, true)} />
              </div>
            )}
            {!['data-text', 'static-text'].includes(el.type) && (
              <div className="props-empty-small">No data bindings for this element type</div>
            )}
          </div>
        )}
      </div>
    </div>
  );
};
