import React from 'react';
import { Group, Rect, Text, Line, Circle, RegularPolygon } from 'react-konva';
import Konva from 'konva';
import type { LayoutElement } from '../../api/client';
import { resolveColor } from '../../utils/themeColors';
import { getFontSize, getDataFieldValue, MOCK_WEATHER } from '../../utils/fontMetrics';

interface ElementProps {
  element: LayoutElement;
  isDark: boolean;
  isSelected: boolean;
  onSelect: (id: string, addToSelection: boolean) => void;
  onDragEnd: (id: string, x: number, y: number) => void;
  onDragStart: (id: string) => void;
  onTransformEnd: (id: string, node: Konva.Group) => void;
  groupRef: (node: Konva.Group | null) => void;
}

function fontFamily(font: number): string {
  return font === 1 ? 'monospace' : 'sans-serif';
}

function DataTextElement({ element: el, isDark }: { element: LayoutElement; isDark: boolean }) {
  const { dataField, font, textSize, color, bgColor, align, prefix, suffix } = el.properties as Record<string, unknown>;
  const f = (font as number) || 2;
  const ts = (textSize as number) || 1;
  const resolvedColor = resolveColor((color as string) || 'themeText', isDark);
  const resolvedBg = resolveColor((bgColor as string) || 'themeBg', isDark);
  const value = getDataFieldValue((dataField as string) || 'airTempC');
  const text = `${prefix || ''}${value}${suffix || ''}`;
  const fontSize = getFontSize(f, ts);

  return (
    <>
      <Rect width={el.width} height={el.height} fill={resolvedBg} />
      <Text
        text={text}
        fontSize={fontSize}
        fontFamily={fontFamily(f)}
        fill={resolvedColor}
        width={el.width}
        height={el.height}
        verticalAlign="middle"
        align={(align as string) || 'left'}
      />
    </>
  );
}

function StaticTextElement({ element: el, isDark }: { element: LayoutElement; isDark: boolean }) {
  const { text, font, textSize, color, bgColor, align } = el.properties as Record<string, unknown>;
  const f = (font as number) || 2;
  const ts = (textSize as number) || 1;
  const resolvedColor = resolveColor((color as string) || 'themeText', isDark);
  const resolvedBg = resolveColor((bgColor as string) || 'themeBg', isDark);
  const fontSize = getFontSize(f, ts);

  return (
    <>
      <Rect width={el.width} height={el.height} fill={resolvedBg} />
      <Text
        text={(text as string) || 'Label'}
        fontSize={fontSize}
        fontFamily={fontFamily(f)}
        fill={resolvedColor}
        width={el.width}
        height={el.height}
        verticalAlign="middle"
        align={(align as string) || 'left'}
      />
    </>
  );
}

function RectElement({ element: el, isDark }: { element: LayoutElement; isDark: boolean }) {
  const { fillColor, strokeColor, cornerRadius } = el.properties as Record<string, unknown>;
  const fill = resolveColor((fillColor as string) || 'themePanel', isDark);
  const stroke = (strokeColor as string) === 'none' ? undefined : resolveColor((strokeColor as string) || 'themeEdge', isDark);
  const radius = (cornerRadius as number) || 0;

  return (
    <Rect
      width={el.width}
      height={el.height}
      fill={fill}
      stroke={stroke}
      strokeWidth={stroke ? 1 : 0}
      cornerRadius={radius}
    />
  );
}

function HLineElement({ element: el, isDark }: { element: LayoutElement; isDark: boolean }) {
  const { color } = el.properties as Record<string, unknown>;
  const resolvedColor = resolveColor((color as string) || 'themeEdge', isDark);
  return (
    <>
      <Rect width={el.width} height={3} fill="transparent" />
      <Line points={[0, 0, el.width, 0]} stroke={resolvedColor} strokeWidth={1} />
    </>
  );
}

function VLineElement({ element: el, isDark }: { element: LayoutElement; isDark: boolean }) {
  const { color } = el.properties as Record<string, unknown>;
  const resolvedColor = resolveColor((color as string) || 'themeEdge', isDark);
  return (
    <>
      <Rect width={3} height={el.height} fill="transparent" />
      <Line points={[0, 0, 0, el.height]} stroke={resolvedColor} strokeWidth={1} />
    </>
  );
}

function WeatherIconElement({ element: el, isDark }: { element: LayoutElement; isDark: boolean }) {
  const { iconSize } = el.properties as Record<string, unknown>;
  const size = Math.max(16, (iconSize as number) || Math.min(el.width, el.height));
  const code = parseInt(MOCK_WEATHER.currentIconCode, 10);
  const sun = '#ffc400';
  const cloud = '#aac3d7';
  const rain = '#5ab4ff';
  const storm = '#ff7828';

  const cx = size / 2;
  const cy = size / 2;

  if (code === 16 || code === 17) {
    return (
      <Group>
        <Circle x={cx - 8} y={cy - 2} radius={7} fill={cloud} />
        <Circle x={cx} y={cy - 6} radius={9} fill={cloud} />
        <Circle x={cx + 10} y={cy - 2} radius={7} fill={cloud} />
        <RegularPolygon x={cx} y={cy + 14} sides={3} radius={8} fill={storm} />
      </Group>
    );
  }
  if (code === 12 || code === 13 || code === 14) {
    return (
      <Group>
        <Circle x={cx - 8} y={cy - 6} radius={7} fill={cloud} />
        <Circle x={cx} y={cy - 10} radius={9} fill={cloud} />
        <Circle x={cx + 10} y={cy - 6} radius={7} fill={cloud} />
        <Line points={[cx - 8, cy + 6, cx - 12, cy + 12]} stroke={rain} strokeWidth={2} />
        <Line points={[cx, cy + 6, cx - 4, cy + 12]} stroke={rain} strokeWidth={2} />
        <Line points={[cx + 8, cy + 6, cx + 4, cy + 12]} stroke={rain} strokeWidth={2} />
      </Group>
    );
  }
  if (code === 3 || code === 4 || code === 6 || code === 7 || code === 8) {
    return (
      <Group>
        <Circle x={12} y={12} radius={6} fill={sun} />
        <Circle x={cx + 4 - 8} y={cy - 4} radius={7} fill={cloud} />
        <Circle x={cx + 4} y={cy - 8} radius={9} fill={cloud} />
        <Circle x={cx + 14} y={cy - 4} radius={7} fill={cloud} />
      </Group>
    );
  }
  return (
    <Group>
      <Circle x={cx} y={cy} radius={8} fill={sun} />
      {[0, 1, 2, 3, 4, 5, 6, 7].map(i => {
        const a = (i * Math.PI) / 4;
        return (
          <Line
            key={i}
            points={[
              cx + Math.cos(a) * 10, cy + Math.sin(a) * 10,
              cx + Math.cos(a) * 15, cy + Math.sin(a) * 15,
            ]}
            stroke={sun}
            strokeWidth={2}
          />
        );
      })}
    </Group>
  );
}

function TimeElement({ element: el, isDark }: { element: LayoutElement; isDark: boolean }) {
  const { font, textSize, color, bgColor, format, showSeconds } = el.properties as Record<string, unknown>;
  const f = (font as number) || 2;
  const ts = (textSize as number) || 1;
  const resolvedColor = resolveColor((color as string) || 'themeGood', isDark);
  const resolvedBg = resolveColor((bgColor as string) || 'themeHeader', isDark);
  const fontSize = getFontSize(f, ts);
  const is24h = (format as string) === '24h';
  const showSec = (showSeconds as boolean) !== false;
  let timeText: string;
  if (is24h) {
    timeText = showSec ? '14:30:45' : '14:30';
  } else {
    timeText = showSec ? '2:30:45 PM' : '2:30 PM';
  }
  return (
    <>
      <Rect width={el.width} height={el.height} fill={resolvedBg} />
      <Text text={timeText} fontSize={fontSize} fontFamily={fontFamily(f)} fill={resolvedColor} width={el.width} height={el.height} verticalAlign="middle" />
    </>
  );
}

function DateElement({ element: el, isDark }: { element: LayoutElement; isDark: boolean }) {
  const { font, textSize, color, bgColor, dateFormat } = el.properties as Record<string, unknown>;
  const f = (font as number) || 2;
  const ts = (textSize as number) || 1;
  const resolvedColor = resolveColor((color as string) || 'themeGood', isDark);
  const resolvedBg = resolveColor((bgColor as string) || 'themeHeader', isDark);
  const fontSize = getFontSize(f, ts);
  const fmt = (dateFormat as string) || 'short';
  let dateText: string;
  if (fmt === 'long') {
    dateText = 'Saturday 8 March';
  } else if (fmt === 'numeric') {
    dateText = '08/03/2026';
  } else {
    dateText = 'Sat 08 Mar';
  }
  return (
    <>
      <Rect width={el.width} height={el.height} fill={resolvedBg} />
      <Text text={dateText} fontSize={fontSize} fontFamily={fontFamily(f)} fill={resolvedColor} width={el.width} height={el.height} verticalAlign="middle" />
    </>
  );
}

function ForecastCardElement({ element: el, isDark }: { element: LayoutElement; isDark: boolean }) {
  const { cardWidth, cardHeight, cardSpacing, cornerRadius, cardBg, cardBorder, labelColor, tempColor, rainColor } = el.properties as Record<string, unknown>;
  const cw = (cardWidth as number) || 76;
  const ch = (cardHeight as number) || 58;
  const sp = (cardSpacing as number) || 2;
  const cr = (cornerRadius as number) || 6;
  const bg = resolveColor((cardBg as string) || 'themePanel', isDark);
  const border = resolveColor((cardBorder as string) || 'themeEdge', isDark);
  const lbl = resolveColor((labelColor as string) || 'themeAccent', isDark);
  const tmp = resolveColor((tempColor as string) || 'themeText', isDark);
  const rn = resolveColor((rainColor as string) || 'themeTextMuted', isDark);
  const w = MOCK_WEATHER;

  return (
    <Group>
      {[0, 1, 2].map(i => {
        const cx = i * (cw + sp);
        return (
          <Group key={i} x={cx} y={0}>
            <Rect width={cw} height={ch} fill={bg} stroke={border} strokeWidth={1} cornerRadius={cr} />
            <Text text={w.nextDayLabel[i]} x={6} y={4} fontSize={14} fill={lbl} fontFamily="sans-serif" />
            <Text text={`${w.nextDayMinC[i]}-${w.nextDayMaxC[i]}\u00B0`} x={6} y={22} fontSize={14} fill={tmp} fontFamily="sans-serif" />
            <Text text={`${w.nextDayRainChance[i]} ${w.nextDayRain[i]}`} x={6} y={40} fontSize={8} fill={rn} fontFamily="sans-serif" />
          </Group>
        );
      })}
    </Group>
  );
}

export const CanvasElement: React.FC<ElementProps> = ({
  element,
  isDark,
  isSelected,
  onSelect,
  onDragEnd,
  onDragStart,
  onTransformEnd,
  groupRef,
}) => {
  const renderContent = () => {
    switch (element.type) {
      case 'data-text': return <DataTextElement element={element} isDark={isDark} />;
      case 'static-text': return <StaticTextElement element={element} isDark={isDark} />;
      case 'rect':
      case 'rounded-rect': return <RectElement element={element} isDark={isDark} />;
      case 'hline': return <HLineElement element={element} isDark={isDark} />;
      case 'vline': return <VLineElement element={element} isDark={isDark} />;
      case 'weather-icon': return <WeatherIconElement element={element} isDark={isDark} />;
      case 'forecast-card': return <ForecastCardElement element={element} isDark={isDark} />;
      case 'time': return <TimeElement element={element} isDark={isDark} />;
      case 'date': return <DateElement element={element} isDark={isDark} />;
      default:
        return <Rect width={element.width} height={element.height} fill="rgba(255,0,0,0.3)" />;
    }
  };

  return (
    <Group
      ref={groupRef}
      x={element.x}
      y={element.y}
      draggable
      onClick={(e) => {
        e.cancelBubble = true;
        onSelect(element.id, e.evt.shiftKey);
      }}
      onTap={(e) => {
        e.cancelBubble = true;
        onSelect(element.id, false);
      }}
      onDragStart={() => onDragStart(element.id)}
      onDragEnd={(e) => {
        onDragEnd(element.id, e.target.x(), e.target.y());
      }}
      onTransformEnd={(e) => {
        onTransformEnd(element.id, e.target as Konva.Group);
      }}
    >
      {renderContent()}
    </Group>
  );
};
