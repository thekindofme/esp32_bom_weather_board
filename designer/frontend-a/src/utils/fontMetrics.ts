// Approximate TFT_eSPI font dimensions for canvas preview rendering
// Font 1 = GLCD (fixed width), Font 2 & 4 are proportional but we use averages

export interface FontInfo {
  charWidth: number;
  charHeight: number;
  label: string;
}

export const FONT_METRICS: Record<number, FontInfo> = {
  1: { charWidth: 6, charHeight: 8, label: 'Small (Font 1)' },
  2: { charWidth: 11, charHeight: 16, label: 'Medium (Font 2)' },
  4: { charWidth: 18, charHeight: 26, label: 'Large (Font 4)' },
};

export function getTextDimensions(
  text: string,
  font: number,
  textSize: number
): { width: number; height: number } {
  const metrics = FONT_METRICS[font] || FONT_METRICS[2];
  return {
    width: text.length * metrics.charWidth * textSize,
    height: metrics.charHeight * textSize,
  };
}

export function getFontSize(font: number, textSize: number): number {
  const metrics = FONT_METRICS[font] || FONT_METRICS[2];
  return metrics.charHeight * textSize;
}

export const MOCK_WEATHER = {
  stationName: 'Melbourne Airport',
  airTempC: '23.4',
  apparentTempC: '21.8',
  relHumidityPct: '65',
  windDir: 'SW',
  windKmh: '15',
  rainfallMm: '0.2',
  dayMinTempC: '14',
  dayMaxTempC: '26',
  feelsMinTempC: '12',
  feelsMaxTempC: '24',
  rainTodayChance: '40%',
  rainTodayRange: '0-4 mm',
  rainMorningRange: '0-1 mm',
  rainEveningRange: '0-3 mm',
  currentIconCode: '3',
  observedTimeLocal: '2:30 PM',
  nextDayLabel: ['Mon', 'Tue', 'Wed'],
  nextDayMinC: ['15', '13', '16'],
  nextDayMaxC: ['24', '22', '27'],
  nextDayRainChance: ['20%', '60%', '10%'],
  nextDayRain: ['0-1 mm', '2-6 mm', '0 mm'],
  nextDayIconCode: ['1', '12', '3'],
} as const;

export function getDataFieldValue(dataField: string): string {
  const w = MOCK_WEATHER;
  const map: Record<string, string> = {
    stationName: w.stationName,
    airTempC: w.airTempC,
    apparentTempC: w.apparentTempC,
    relHumidityPct: w.relHumidityPct,
    windDir: w.windDir,
    windKmh: w.windKmh,
    rainfallMm: w.rainfallMm,
    dayMinTempC: w.dayMinTempC,
    dayMaxTempC: w.dayMaxTempC,
    feelsMinTempC: w.feelsMinTempC,
    feelsMaxTempC: w.feelsMaxTempC,
    rainTodayChance: w.rainTodayChance,
    rainTodayRange: w.rainTodayRange,
    rainMorningRange: w.rainMorningRange,
    rainEveningRange: w.rainEveningRange,
    rainTodayChance_num: w.rainTodayChance,
    currentIconCode: w.currentIconCode,
    observedTimeLocal: w.observedTimeLocal,
  };
  return map[dataField] ?? dataField;
}

export const DATA_FIELDS = [
  { value: 'stationName', label: 'Station Name' },
  { value: 'airTempC', label: 'Temperature (°C)' },
  { value: 'apparentTempC', label: 'Apparent Temp (°C)' },
  { value: 'relHumidityPct', label: 'Humidity (%)' },
  { value: 'windDir', label: 'Wind Direction' },
  { value: 'windKmh', label: 'Wind Speed (km/h)' },
  { value: 'rainfallMm', label: 'Rainfall (mm)' },
  { value: 'dayMinTempC', label: 'Day Min Temp' },
  { value: 'dayMaxTempC', label: 'Day Max Temp' },
  { value: 'feelsMinTempC', label: 'Feels Min Temp' },
  { value: 'feelsMaxTempC', label: 'Feels Max Temp' },
  { value: 'rainTodayChance', label: 'Rain Chance' },
  { value: 'rainTodayRange', label: 'Rain Range' },
  { value: 'rainMorningRange', label: 'Rain Morning' },
  { value: 'rainEveningRange', label: 'Rain Evening' },
  { value: 'currentIconCode', label: 'Icon Code' },
  { value: 'observedTimeLocal', label: 'Obs Time' },
];
