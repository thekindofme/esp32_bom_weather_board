import type { Layout, LayoutElement } from '../api/client';

function asInt(value: unknown, fallback: number): number {
  return typeof value === 'number' && Number.isFinite(value)
    ? Math.round(value)
    : fallback;
}

export function normalizeElement(element: LayoutElement): LayoutElement {
  const properties = { ...element.properties };

  switch (element.type) {
    case 'weather-icon': {
      const fallbackSize = Math.max(16, Math.min(element.width, element.height) || 48);
      const iconSize = Math.max(16, asInt(properties.iconSize, fallbackSize));
      properties.iconSize = iconSize;
      return { ...element, width: iconSize, height: iconSize, properties };
    }
    case 'forecast-card': {
      const spacing = Math.max(0, asInt(properties.cardSpacing, 2));
      const fallbackWidth = Math.max(40, Math.round((Math.max(element.width, 232) - spacing * 2) / 3));
      const cardWidth = Math.max(40, asInt(properties.cardWidth, fallbackWidth));
      const cardHeight = Math.max(30, asInt(properties.cardHeight, Math.max(element.height, 58)));
      properties.cardSpacing = spacing;
      properties.cardWidth = cardWidth;
      properties.cardHeight = cardHeight;
      return {
        ...element,
        width: cardWidth * 3 + spacing * 2,
        height: cardHeight,
        properties,
      };
    }
    default:
      return element;
  }
}

export function applyElementUpdate(element: LayoutElement, updates: Partial<LayoutElement>): LayoutElement {
  const next: LayoutElement = {
    ...element,
    ...updates,
    properties: {
      ...element.properties,
      ...(updates.properties ?? {}),
    },
  };

  if (next.type === 'weather-icon' && (updates.width !== undefined || updates.height !== undefined)) {
    const width = updates.width ?? element.width;
    const height = updates.height ?? element.height;
    const iconSize = Math.max(16, Math.round(Math.min(width, height)));
    next.properties = { ...next.properties, iconSize };
  }

  if (next.type === 'forecast-card' && (updates.width !== undefined || updates.height !== undefined)) {
    const spacing = Math.max(0, asInt(next.properties.cardSpacing, 2));
    const width = Math.max(124, Math.round(updates.width ?? element.width));
    const height = Math.max(30, Math.round(updates.height ?? element.height));
    const cardWidth = Math.max(40, Math.round((width - spacing * 2) / 3));
    next.properties = { ...next.properties, cardWidth, cardHeight: height };
  }

  return normalizeElement(next);
}

export function applyPropertyUpdate(element: LayoutElement, key: string, value: unknown): LayoutElement {
  const next: LayoutElement = {
    ...element,
    properties: {
      ...element.properties,
      [key]: value,
    },
  };

  if (element.type === 'weather-icon' && key === 'iconSize') {
    const iconSize = Math.max(16, asInt(value, 48));
    next.width = iconSize;
    next.height = iconSize;
  }

  return normalizeElement(next);
}

export function normalizeLayout(layout: Layout): Layout {
  return {
    ...layout,
    elements: layout.elements.map(normalizeElement),
  };
}
