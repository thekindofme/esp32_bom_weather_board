export interface FontInfo { charWidth: number; charHeight: number; label: string; }

export const TEXT_SIZE_MIN = 1;
export const TEXT_SIZE_MAX = 6;

export const FONT_METRICS: Record<number, FontInfo> = {
  1: { charWidth: 6, charHeight: 8, label: 'Small (8px)' },
  2: { charWidth: 11, charHeight: 16, label: 'Medium (16px)' },
  4: { charWidth: 18, charHeight: 26, label: 'Large (26px)' },
};

export function getFontSize(font: number, textSize: number): number {
  return (FONT_METRICS[font]?.charHeight ?? 16) * textSize;
}

export const MOCK_WEATHER = {
  stationName: 'Melbourne Airport', airTempC: '23.4', apparentTempC: '21.8',
  relHumidityPct: '65', windDir: 'SW', windKmh: '15', rainfallMm: '0.2',
  dayMinTempC: '14', dayMaxTempC: '26', feelsMinTempC: '12', feelsMaxTempC: '24',
  rainTodayChance: '40%', rainTodayRange: '0-4 mm',
  rainMorningRange: '0-1 mm', rainEveningRange: '0-3 mm',
  currentIconCode: '3', observedTimeLocal: '2:30 PM',
  nextDayLabel: ['Mon', 'Tue', 'Wed'] as readonly string[],
  nextDayMinC: ['15', '13', '16'] as readonly string[],
  nextDayMaxC: ['24', '22', '27'] as readonly string[],
  nextDayRainChance: ['20%', '60%', '10%'] as readonly string[],
  nextDayRain: ['0-1 mm', '2-6 mm', '0 mm'] as readonly string[],
  nextDayIconCode: ['1', '12', '3'] as readonly string[],
} as const;

export function getDataFieldValue(field: string): string {
  const map: Record<string, string> = {
    stationName: MOCK_WEATHER.stationName, airTempC: MOCK_WEATHER.airTempC,
    apparentTempC: MOCK_WEATHER.apparentTempC, relHumidityPct: MOCK_WEATHER.relHumidityPct,
    windDir: MOCK_WEATHER.windDir, windKmh: MOCK_WEATHER.windKmh,
    rainfallMm: MOCK_WEATHER.rainfallMm, dayMinTempC: MOCK_WEATHER.dayMinTempC,
    dayMaxTempC: MOCK_WEATHER.dayMaxTempC, feelsMinTempC: MOCK_WEATHER.feelsMinTempC,
    feelsMaxTempC: MOCK_WEATHER.feelsMaxTempC, rainTodayChance: MOCK_WEATHER.rainTodayChance,
    rainTodayRange: MOCK_WEATHER.rainTodayRange, rainMorningRange: MOCK_WEATHER.rainMorningRange,
    rainEveningRange: MOCK_WEATHER.rainEveningRange, currentIconCode: MOCK_WEATHER.currentIconCode,
    observedTimeLocal: MOCK_WEATHER.observedTimeLocal,
  };
  return map[field] ?? field;
}

export const DATA_FIELDS = [
  { value: 'stationName', label: 'Station Name' },
  { value: 'airTempC', label: 'Temperature' },
  { value: 'apparentTempC', label: 'Apparent Temp' },
  { value: 'relHumidityPct', label: 'Humidity' },
  { value: 'windDir', label: 'Wind Dir' },
  { value: 'windKmh', label: 'Wind Speed' },
  { value: 'rainfallMm', label: 'Rainfall' },
  { value: 'dayMinTempC', label: 'Min Temp' },
  { value: 'dayMaxTempC', label: 'Max Temp' },
  { value: 'rainTodayChance', label: 'Rain Chance' },
  { value: 'rainTodayRange', label: 'Rain Range' },
  { value: 'rainMorningRange', label: 'Rain AM' },
  { value: 'rainEveningRange', label: 'Rain PM' },
  { value: 'observedTimeLocal', label: 'Obs Time' },
];
