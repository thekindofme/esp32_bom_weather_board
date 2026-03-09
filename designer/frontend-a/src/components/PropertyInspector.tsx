import React from 'react';
import { useLayoutStore } from '../store/layoutStore';
import { THEME_SWATCHES, DARK_THEME, LIGHT_THEME, type ThemeColorName } from '../utils/themeColors';
import { DATA_FIELDS, FONT_METRICS } from '../utils/fontMetrics';

function ColorPicker({
  value,
  onChange,
  isDark,
  label,
}: {
  value: string;
  onChange: (v: string) => void;
  isDark: boolean;
  label: string;
}) {
  const theme = isDark ? DARK_THEME : LIGHT_THEME;
  const isThemeColor = value in theme;

  return (
    <div className="prop-field">
      <label>{label}</label>
      <div className="color-picker">
        <div className="color-swatches">
          {THEME_SWATCHES.map(s => (
            <button
              key={s.name}
              className={`swatch ${value === s.name ? 'active' : ''}`}
              style={{ backgroundColor: theme[s.name] }}
              onClick={() => onChange(s.name)}
              title={s.label}
            />
          ))}
        </div>
        <div className="color-custom">
          <input
            type="color"
            value={isThemeColor ? '#ffffff' : (value.startsWith('#') ? value : '#ffffff')}
            onChange={e => onChange(e.target.value)}
          />
          <input
            type="text"
            className="color-hex-input"
            value={value}
            onChange={e => onChange(e.target.value)}
            placeholder="#hex or theme"
          />
        </div>
      </div>
    </div>
  );
}

function NumberInput({
  label,
  value,
  onChange,
  min,
  max,
  step,
}: {
  label: string;
  value: number;
  onChange: (v: number) => void;
  min?: number;
  max?: number;
  step?: number;
}) {
  return (
    <div className="prop-field prop-field-inline">
      <label>{label}</label>
      <input
        type="number"
        value={value}
        onChange={e => onChange(Number(e.target.value))}
        min={min}
        max={max}
        step={step || 1}
      />
    </div>
  );
}

function SelectInput({
  label,
  value,
  options,
  onChange,
}: {
  label: string;
  value: string | number;
  options: { value: string | number; label: string }[];
  onChange: (v: string) => void;
}) {
  return (
    <div className="prop-field">
      <label>{label}</label>
      <select value={value} onChange={e => onChange(e.target.value)}>
        {options.map(o => (
          <option key={o.value} value={o.value}>{o.label}</option>
        ))}
      </select>
    </div>
  );
}

function TextInput({
  label,
  value,
  onChange,
}: {
  label: string;
  value: string;
  onChange: (v: string) => void;
}) {
  return (
    <div className="prop-field">
      <label>{label}</label>
      <input type="text" value={value} onChange={e => onChange(e.target.value)} />
    </div>
  );
}

export const PropertyInspector: React.FC = () => {
  const selectedIds = useLayoutStore(s => s.selectedIds);
  const layout = useLayoutStore(s => s.layout);
  const updateElement = useLayoutStore(s => s.updateElement);
  const updateElementProperty = useLayoutStore(s => s.updateElementProperty);
  const removeElement = useLayoutStore(s => s.removeElement);
  const duplicateSelected = useLayoutStore(s => s.duplicateSelected);
  const isDark = useLayoutStore(s => s.isDarkPreview);

  if (selectedIds.length === 0) {
    return (
      <div className="property-inspector">
        <div className="inspector-header">PROPERTIES</div>
        <div className="inspector-empty">
          <div className="empty-icon">+</div>
          <div className="empty-text">Select an element<br />to edit properties</div>
        </div>
      </div>
    );
  }

  if (selectedIds.length > 1) {
    return (
      <div className="property-inspector">
        <div className="inspector-header">PROPERTIES</div>
        <div className="inspector-empty">
          <div className="empty-text">{selectedIds.length} elements selected</div>
        </div>
        <div className="inspector-actions">
          <button className="btn-danger" onClick={() => selectedIds.forEach(id => removeElement(id))}>Delete All</button>
        </div>
      </div>
    );
  }

  const el = layout.elements.find(e => e.id === selectedIds[0]);
  if (!el) return null;

  const props = el.properties as Record<string, unknown>;
  const setProp = (key: string, value: unknown) => updateElementProperty(el.id, key, value);

  const fontOptions = Object.entries(FONT_METRICS).map(([k, v]) => ({
    value: k,
    label: v.label,
  }));

  return (
    <div className="property-inspector">
      <div className="inspector-header">
        <span>PROPERTIES</span>
        <span className="element-type-badge">{el.type}</span>
      </div>

      {/* Position section */}
      <div className="inspector-section">
        <div className="section-label">POSITION</div>
        <div className="prop-grid-4">
          <NumberInput label="X" value={el.x} onChange={v => updateElement(el.id, { x: v })} min={0} />
          <NumberInput label="Y" value={el.y} onChange={v => updateElement(el.id, { y: v })} min={0} />
          <NumberInput label="W" value={el.width} onChange={v => updateElement(el.id, { width: v })} min={1} />
          <NumberInput label="H" value={el.height} onChange={v => updateElement(el.id, { height: v })} min={1} />
        </div>
        <NumberInput label="Z-Index" value={el.zIndex} onChange={v => updateElement(el.id, { zIndex: v })} />
      </div>

      {/* Type-specific properties */}
      <div className="inspector-section">
        <div className="section-label">APPEARANCE</div>

        {(el.type === 'data-text') && (
          <>
            <SelectInput
              label="Data Field"
              value={(props.dataField as string) || 'airTempC'}
              options={DATA_FIELDS}
              onChange={v => setProp('dataField', v)}
            />
            <SelectInput
              label="Font"
              value={String((props.font as number) || 2)}
              options={fontOptions}
              onChange={v => setProp('font', Number(v))}
            />
            <SelectInput
              label="Text Size"
              value={String((props.textSize as number) || 1)}
              options={[{ value: '1', label: '1x' }, { value: '2', label: '2x' }]}
              onChange={v => setProp('textSize', Number(v))}
            />
            <TextInput label="Prefix" value={(props.prefix as string) || ''} onChange={v => setProp('prefix', v)} />
            <TextInput label="Suffix" value={(props.suffix as string) || ''} onChange={v => setProp('suffix', v)} />
            <ColorPicker label="Color" value={(props.color as string) || 'themeText'} onChange={v => setProp('color', v)} isDark={isDark} />
            <ColorPicker label="Bg Color" value={(props.bgColor as string) || 'themeBg'} onChange={v => setProp('bgColor', v)} isDark={isDark} />
            <SelectInput
              label="Align"
              value={(props.align as string) || 'left'}
              options={[{ value: 'left', label: 'Left' }, { value: 'center', label: 'Center' }, { value: 'right', label: 'Right' }]}
              onChange={v => setProp('align', v)}
            />
          </>
        )}

        {el.type === 'static-text' && (
          <>
            <TextInput label="Text" value={(props.text as string) || 'Label'} onChange={v => setProp('text', v)} />
            <SelectInput
              label="Font"
              value={String((props.font as number) || 2)}
              options={fontOptions}
              onChange={v => setProp('font', Number(v))}
            />
            <SelectInput
              label="Text Size"
              value={String((props.textSize as number) || 1)}
              options={[{ value: '1', label: '1x' }, { value: '2', label: '2x' }]}
              onChange={v => setProp('textSize', Number(v))}
            />
            <ColorPicker label="Color" value={(props.color as string) || 'themeText'} onChange={v => setProp('color', v)} isDark={isDark} />
            <ColorPicker label="Bg Color" value={(props.bgColor as string) || 'themeBg'} onChange={v => setProp('bgColor', v)} isDark={isDark} />
            <SelectInput
              label="Align"
              value={(props.align as string) || 'left'}
              options={[{ value: 'left', label: 'Left' }, { value: 'center', label: 'Center' }, { value: 'right', label: 'Right' }]}
              onChange={v => setProp('align', v)}
            />
          </>
        )}

        {(el.type === 'rect' || el.type === 'rounded-rect') && (
          <>
            <ColorPicker label="Fill" value={(props.fillColor as string) || 'themePanel'} onChange={v => setProp('fillColor', v)} isDark={isDark} />
            <ColorPicker label="Stroke" value={(props.strokeColor as string) || 'themeEdge'} onChange={v => setProp('strokeColor', v)} isDark={isDark} />
            <NumberInput label="Radius" value={(props.cornerRadius as number) || 0} onChange={v => setProp('cornerRadius', v)} min={0} max={30} />
          </>
        )}

        {(el.type === 'hline' || el.type === 'vline') && (
          <ColorPicker label="Color" value={(props.color as string) || 'themeEdge'} onChange={v => setProp('color', v)} isDark={isDark} />
        )}

        {el.type === 'weather-icon' && (
          <NumberInput label="Icon Size" value={(props.iconSize as number) || 48} onChange={v => setProp('iconSize', v)} min={16} max={96} />
        )}

        {el.type === 'forecast-card' && (
          <>
            <NumberInput label="Card W" value={(props.cardWidth as number) || 76} onChange={v => setProp('cardWidth', v)} min={40} />
            <NumberInput label="Card H" value={(props.cardHeight as number) || 58} onChange={v => setProp('cardHeight', v)} min={30} />
            <NumberInput label="Spacing" value={(props.cardSpacing as number) || 2} onChange={v => setProp('cardSpacing', v)} min={0} />
            <NumberInput label="Radius" value={(props.cornerRadius as number) || 6} onChange={v => setProp('cornerRadius', v)} min={0} />
            <ColorPicker label="Card Bg" value={(props.cardBg as string) || 'themePanel'} onChange={v => setProp('cardBg', v)} isDark={isDark} />
            <ColorPicker label="Border" value={(props.cardBorder as string) || 'themeEdge'} onChange={v => setProp('cardBorder', v)} isDark={isDark} />
            <ColorPicker label="Labels" value={(props.labelColor as string) || 'themeAccent'} onChange={v => setProp('labelColor', v)} isDark={isDark} />
            <ColorPicker label="Temps" value={(props.tempColor as string) || 'themeText'} onChange={v => setProp('tempColor', v)} isDark={isDark} />
            <ColorPicker label="Rain" value={(props.rainColor as string) || 'themeTextMuted'} onChange={v => setProp('rainColor', v)} isDark={isDark} />
          </>
        )}

        {(el.type === 'time' || el.type === 'date') && (
          <>
            <SelectInput
              label="Font"
              value={String((props.font as number) || 2)}
              options={fontOptions}
              onChange={v => setProp('font', Number(v))}
            />
            <ColorPicker label="Color" value={(props.color as string) || 'themeGood'} onChange={v => setProp('color', v)} isDark={isDark} />
            <ColorPicker label="Bg Color" value={(props.bgColor as string) || 'themeHeader'} onChange={v => setProp('bgColor', v)} isDark={isDark} />
          </>
        )}
        {el.type === 'time' && (
          <>
            <SelectInput
              label="Format"
              value={(props.format as string) || '12h'}
              options={[{ value: '12h', label: '12-hour' }, { value: '24h', label: '24-hour' }]}
              onChange={v => setProp('format', v)}
            />
            <SelectInput
              label="Seconds"
              value={(props.showSeconds as boolean) === false ? 'false' : 'true'}
              options={[{ value: 'true', label: 'Show' }, { value: 'false', label: 'Hide' }]}
              onChange={v => setProp('showSeconds', v === 'true')}
            />
          </>
        )}
        {el.type === 'date' && (
          <SelectInput
            label="Date Format"
            value={(props.dateFormat as string) || 'short'}
            options={[
              { value: 'short', label: 'Short (Sat 08 Mar)' },
              { value: 'long', label: 'Long (Saturday 8 March)' },
              { value: 'numeric', label: 'Numeric (08/03/2026)' },
            ]}
            onChange={v => setProp('dateFormat', v)}
          />
        )}
      </div>

      {/* Actions */}
      <div className="inspector-section inspector-actions">
        <button className="btn-secondary" onClick={duplicateSelected}>Duplicate</button>
        <button className="btn-danger" onClick={() => removeElement(el.id)}>Delete</button>
      </div>
    </div>
  );
};
