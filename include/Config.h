#pragma once

// Wi-Fi credentials
static const char* WIFI_SSID = "YOUR_WIFI_SSID";
static const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// BOM feed selection:
// Each state has a file, for example:
// VIC: IDV60920.xml, NSW: IDN60920.xml, QLD: IDQ60920.xml, SA: IDS60920.xml,
// WA: IDW60920.xml, TAS: IDT60920.xml, NT: IDD60920.xml
static const char* BOM_HOST = "ftp.bom.gov.au";
static const char* BOM_FILE_PATH = "/anon/gen/fwo/IDV60920.xml";
// Greenvale does not appear as a standalone station in IDV60920;
// nearest practical station is Melbourne Airport.
static const char* BOM_STATION_ID = "086282";  // Melbourne Airport

// Polling cadence
static const uint32_t WEATHER_REFRESH_MS = 10UL * 60UL * 1000UL; // 10 minutes
static const uint32_t WIFI_CONNECT_TIMEOUT_MS = 20000UL;
static const uint32_t FTP_TIMEOUT_MS = 15000UL;
