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
