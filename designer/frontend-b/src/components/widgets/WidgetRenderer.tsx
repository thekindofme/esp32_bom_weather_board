import React from 'react';
import type { LayoutElement } from '../../api/client';
import { resolveColor } from '../../utils/themeColors';
import { getFontSize, getDataFieldValue, MOCK_WEATHER } from '../../utils/fontMetrics';

interface Props {
  element: LayoutElement;
  isDark: boolean;
}

export const WidgetRenderer: React.FC<Props> = ({ element: el, isDark }) => {
  const p = el.properties as Record<string, unknown>;

  switch (el.type) {
    case 'data-text': {
      const value = getDataFieldValue((p.dataField as string) || 'airTempC');
      const text = `${p.prefix || ''}${value}${p.suffix || ''}`;
      const fontSize = getFontSize((p.font as number) || 2, (p.textSize as number) || 1);
      const align = (p.align as string) || 'left';
      const justify = align === 'center' ? 'center' : align === 'right' ? 'flex-end' : 'flex-start';
      return (
        <div style={{
          width: '100%', height: '100%',
          color: resolveColor((p.color as string) || 'themeText', isDark),
          backgroundColor: resolveColor((p.bgColor as string) || 'themeBg', isDark),
          fontSize, fontFamily: (p.font as number) === 1 ? 'monospace' : 'sans-serif',
          display: 'flex', alignItems: 'center', justifyContent: justify, textAlign: align as 'left' | 'center' | 'right',
          overflow: 'hidden', whiteSpace: 'nowrap',
        }}>
          {text}
        </div>
      );
    }
    case 'static-text': {
      const fontSize = getFontSize((p.font as number) || 2, (p.textSize as number) || 1);
      const align = (p.align as string) || 'left';
      const justify = align === 'center' ? 'center' : align === 'right' ? 'flex-end' : 'flex-start';
      return (
        <div style={{
          width: '100%', height: '100%',
          color: resolveColor((p.color as string) || 'themeText', isDark),
          backgroundColor: resolveColor((p.bgColor as string) || 'themeBg', isDark),
          fontSize, fontFamily: (p.font as number) === 1 ? 'monospace' : 'sans-serif',
          display: 'flex', alignItems: 'center', justifyContent: justify, textAlign: align as 'left' | 'center' | 'right',
          overflow: 'hidden',
        }}>
          {(p.text as string) || 'Label'}
        </div>
      );
    }
    case 'rect':
    case 'rounded-rect': {
      const radius = (p.cornerRadius as number) || (el.type === 'rounded-rect' ? 6 : 0);
      const stroke = (p.strokeColor as string) === 'none' ? 'transparent' : resolveColor((p.strokeColor as string) || 'themeEdge', isDark);
      return (
        <div style={{
          width: '100%', height: '100%',
          backgroundColor: resolveColor((p.fillColor as string) || 'themePanel', isDark),
          border: `1px solid ${stroke}`,
          borderRadius: radius,
        }} />
      );
    }
    case 'hline':
      return <div style={{ width: '100%', height: 1, backgroundColor: resolveColor((p.color as string) || 'themeEdge', isDark) }} />;
    case 'vline':
      return <div style={{ width: 1, height: '100%', backgroundColor: resolveColor((p.color as string) || 'themeEdge', isDark) }} />;
    case 'weather-icon': {
      const size = Math.max(16, (p.iconSize as number) || Math.min(el.width, el.height));
      const code = parseInt(MOCK_WEATHER.currentIconCode, 10);
      const sunColor = '#ffc400';
      const cloudColor = '#aac3d7';
      const rainColor = '#5ab4ff';
      return (
        <div style={{ width: '100%', height: '100%', display: 'flex', alignItems: 'center', justifyContent: 'center', position: 'relative' }}>
          <svg width={size} height={size} viewBox={`0 0 ${size} ${size}`}>
            {(code === 3 || code === 4 || code === 6 || code === 7 || code === 8) ? (
              <>
                <circle cx={12} cy={12} r={6} fill={sunColor} />
                <circle cx={size / 2 - 4} cy={size / 2 - 2} r={7} fill={cloudColor} />
                <circle cx={size / 2 + 4} cy={size / 2 - 6} r={9} fill={cloudColor} />
                <circle cx={size / 2 + 14} cy={size / 2 - 2} r={7} fill={cloudColor} />
              </>
            ) : (code === 12 || code === 13 || code === 14) ? (
              <>
                <circle cx={size / 2 - 8} cy={size / 2 - 6} r={7} fill={cloudColor} />
                <circle cx={size / 2} cy={size / 2 - 10} r={9} fill={cloudColor} />
                <circle cx={size / 2 + 10} cy={size / 2 - 6} r={7} fill={cloudColor} />
                <line x1={size / 2 - 6} y1={size / 2 + 6} x2={size / 2 - 10} y2={size / 2 + 14} stroke={rainColor} strokeWidth={2} />
                <line x1={size / 2 + 2} y1={size / 2 + 6} x2={size / 2 - 2} y2={size / 2 + 14} stroke={rainColor} strokeWidth={2} />
                <line x1={size / 2 + 10} y1={size / 2 + 6} x2={size / 2 + 6} y2={size / 2 + 14} stroke={rainColor} strokeWidth={2} />
              </>
            ) : (
              <>
                <circle cx={size / 2} cy={size / 2} r={8} fill={sunColor} />
                {[0,1,2,3,4,5,6,7].map(i => {
                  const a = (i * Math.PI) / 4;
                  return <line key={i} x1={size / 2 + Math.cos(a) * 10} y1={size / 2 + Math.sin(a) * 10} x2={size / 2 + Math.cos(a) * 15} y2={size / 2 + Math.sin(a) * 15} stroke={sunColor} strokeWidth={2} />;
                })}
              </>
            )}
          </svg>
        </div>
      );
    }
    case 'forecast-card': {
      const cw = (p.cardWidth as number) || 76;
      const ch = (p.cardHeight as number) || 58;
      const sp = (p.cardSpacing as number) || 2;
      const cr = (p.cornerRadius as number) || 6;
      const bg = resolveColor((p.cardBg as string) || 'themePanel', isDark);
      const border = resolveColor((p.cardBorder as string) || 'themeEdge', isDark);
      const lbl = resolveColor((p.labelColor as string) || 'themeAccent', isDark);
      const tmp = resolveColor((p.tempColor as string) || 'themeText', isDark);
      const rn = resolveColor((p.rainColor as string) || 'themeTextMuted', isDark);

      return (
        <div style={{ display: 'flex', gap: sp }}>
          {[0, 1, 2].map(i => (
            <div key={i} style={{
              width: cw, height: ch, background: bg,
              border: `1px solid ${border}`, borderRadius: cr,
              padding: '4px 6px', overflow: 'hidden',
            }}>
              <div style={{ fontSize: 12, color: lbl, fontFamily: 'sans-serif' }}>{MOCK_WEATHER.nextDayLabel[i]}</div>
              <div style={{ fontSize: 12, color: tmp, fontFamily: 'sans-serif' }}>{MOCK_WEATHER.nextDayMinC[i]}-{MOCK_WEATHER.nextDayMaxC[i]}{'\u00B0'}</div>
              <div style={{ fontSize: 8, color: rn, fontFamily: 'sans-serif' }}>{MOCK_WEATHER.nextDayRainChance[i]} {MOCK_WEATHER.nextDayRain[i]}</div>
            </div>
          ))}
        </div>
      );
    }
    case 'time': {
      const fontSize = getFontSize((p.font as number) || 2, 1);
      const is24h = (p.format as string) === '24h';
      const showSec = (p.showSeconds as boolean) !== false;
      let timeText: string;
      if (is24h) {
        timeText = showSec ? '14:30:45' : '14:30';
      } else {
        timeText = showSec ? '2:30:45 PM' : '2:30 PM';
      }
      return (
        <div style={{
          width: '100%', height: '100%',
          color: resolveColor((p.color as string) || 'themeGood', isDark),
          backgroundColor: resolveColor((p.bgColor as string) || 'themeHeader', isDark),
          fontSize, display: 'flex', alignItems: 'center', fontFamily: 'sans-serif',
        }}>{timeText}</div>
      );
    }
    case 'date': {
      const fontSize = getFontSize((p.font as number) || 2, 1);
      const dateFormat = (p.dateFormat as string) || 'short';
      let dateText: string;
      if (dateFormat === 'long') {
        dateText = 'Saturday 8 March';
      } else if (dateFormat === 'numeric') {
        dateText = '08/03/2026';
      } else {
        dateText = 'Sat 08 Mar';
      }
      return (
        <div style={{
          width: '100%', height: '100%',
          color: resolveColor((p.color as string) || 'themeGood', isDark),
          backgroundColor: resolveColor((p.bgColor as string) || 'themeHeader', isDark),
          fontSize, display: 'flex', alignItems: 'center', fontFamily: 'sans-serif',
        }}>{dateText}</div>
      );
    }
    default:
      return <div style={{ width: '100%', height: '100%', background: 'rgba(196,93,62,0.2)', border: '1px dashed #c45d3e' }} />;
  }
};
