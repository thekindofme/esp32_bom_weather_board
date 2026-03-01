#pragma once

// Optional local override file (ignored by git).
// Create include/ConfigLocal.h from include/ConfigLocal.example.h and define
// any values you want to override.
#if defined(__has_include)
#if __has_include("ConfigLocal.h")
#include "ConfigLocal.h"
#endif
#endif

// Wi-Fi credentials
#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

// BOM feed selection:
// Each state has a file, for example:
// VIC: IDV60920.xml, NSW: IDN60920.xml, QLD: IDQ60920.xml, SA: IDS60920.xml,
// WA: IDW60920.xml, TAS: IDT60920.xml, NT: IDD60920.xml
#ifndef BOM_HOST
#define BOM_HOST "ftp.bom.gov.au"
#endif
#ifndef BOM_FILE_PATH
#define BOM_FILE_PATH "/anon/gen/fwo/IDV60920.xml"
#endif
// Greenvale does not appear as a standalone station in IDV60920;
// nearest practical station is Melbourne Airport.
#ifndef BOM_STATION_ID
#define BOM_STATION_ID "086282"  // Melbourne Airport
#endif

// Location forecast feed (used for rain range outlook)
#ifndef BOM_FORECAST_FILE_PATH
#define BOM_FORECAST_FILE_PATH "/anon/gen/fwo/IDV10753.xml"
#endif
#ifndef BOM_FORECAST_LOCATION
#define BOM_FORECAST_LOCATION "Tullamarine"
#endif

// Polling cadence
#ifndef WEATHER_REFRESH_MS
#define WEATHER_REFRESH_MS (10UL * 60UL * 1000UL) // 10 minutes
#endif
#ifndef WIFI_CONNECT_TIMEOUT_MS
#define WIFI_CONNECT_TIMEOUT_MS 20000UL
#endif
#ifndef FTP_TIMEOUT_MS
#define FTP_TIMEOUT_MS 15000UL
#endif

// Time sync
#ifndef TZ_INFO
#define TZ_INFO "AEST-10AEDT,M10.1.0/2,M4.1.0/3"
#endif
#ifndef NTP_SERVER_1
#define NTP_SERVER_1 "pool.ntp.org"
#endif
#ifndef NTP_SERVER_2
#define NTP_SERVER_2 "time.google.com"
#endif

// Touch toggle (XPT2046 IRQ line on many ESP32-2432S028 boards)
#ifndef TOUCH_IRQ_PIN
#define TOUCH_IRQ_PIN 36
#endif
#ifndef TOUCH_TOGGLE_DEBOUNCE_MS
#define TOUCH_TOGGLE_DEBOUNCE_MS 400UL
#endif
