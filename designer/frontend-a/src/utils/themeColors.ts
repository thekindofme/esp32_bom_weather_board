export interface ThemeColorSet {
  themeBg: string;
  themeHeader: string;
  themePanel: string;
  themeEdge: string;
  themeText: string;
  themeTextMuted: string;
  themeAccent: string;
  themeGood: string;
}

export const DARK_THEME: ThemeColorSet = {
  themeBg: 'rgb(1, 6, 12)',
  themeHeader: 'rgb(6, 14, 22)',
  themePanel: 'rgb(9, 20, 30)',
  themeEdge: 'rgb(25, 70, 95)',
  themeText: 'rgb(180, 205, 220)',
  themeTextMuted: 'rgb(95, 130, 150)',
  themeAccent: 'rgb(120, 180, 205)',
  themeGood: 'rgb(105, 185, 135)',
};

export const LIGHT_THEME: ThemeColorSet = {
  themeBg: 'rgb(225, 234, 239)',
  themeHeader: 'rgb(205, 222, 232)',
  themePanel: 'rgb(243, 248, 251)',
  themeEdge: 'rgb(98, 142, 168)',
  themeText: 'rgb(24, 48, 64)',
  themeTextMuted: 'rgb(88, 110, 124)',
  themeAccent: 'rgb(31, 104, 146)',
  themeGood: 'rgb(20, 128, 78)',
};

export const THEME_COLOR_NAMES = [
  'themeBg', 'themeHeader', 'themePanel', 'themeEdge',
  'themeText', 'themeTextMuted', 'themeAccent', 'themeGood',
] as const;

export type ThemeColorName = typeof THEME_COLOR_NAMES[number];

export function resolveColor(
  colorValue: string,
  isDark: boolean
): string {
  const theme = isDark ? DARK_THEME : LIGHT_THEME;
  if (colorValue in theme) {
    return theme[colorValue as ThemeColorName];
  }
  if (colorValue === 'custom' || colorValue === 'none') {
    return 'transparent';
  }
  return colorValue;
}

export const THEME_SWATCHES: { name: ThemeColorName; label: string }[] = [
  { name: 'themeBg', label: 'Background' },
  { name: 'themeHeader', label: 'Header' },
  { name: 'themePanel', label: 'Panel' },
  { name: 'themeEdge', label: 'Edge' },
  { name: 'themeText', label: 'Text' },
  { name: 'themeTextMuted', label: 'Muted' },
  { name: 'themeAccent', label: 'Accent' },
  { name: 'themeGood', label: 'Good' },
];
