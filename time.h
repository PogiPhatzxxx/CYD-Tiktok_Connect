#ifndef TIME_H
#define TIME_H

#include <WiFi.h>
#include <time.h>
#include <TFT_eSPI.h>

// WiFi credentials
const char* ssid = "REPLACE_SSID";
const char* password = "REPLACE_WIFI_PASSWORD";

// NTP server and timezone
const char* ntpServer = "time1.google.com";
const long gmtOffset_sec = 8 * 3600; // GMT+8
const int daylightOffset_sec = 0;

// External TFT reference
extern TFT_eSPI tft;

// Function declarations
void initializeTime();
void displayTime();
bool isTimeInitialized();
String getCurrentTimeString();
String getCurrentDateString();
String getCurrentDayString();

// Global variables
bool timeInitialized = false;

// Store previous values to avoid unnecessary redrawing
String prevDayStr = "";
String prevTimeStr = "";
String prevDateStr = "";

// Flag to track if initial drawing is done
bool initialDrawDone = false;

// Display positions and dimensions
const int DISPLAY_X = 10;
const int DISPLAY_Y = 80; // Keep this the same
const int DISPLAY_WIDTH = 220;
const int DISPLAY_HEIGHT = 120;
const int DAY_Y = 90;
const int TIME_Y = 120;
const int DATE_Y = 155;

bool isTimeInitialized() {
  return timeInitialized;
}

String getCurrentTimeString() {
  if (!timeInitialized) return "--:--:--";
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "--:--:--";
  }
  
  char timeStr[16];
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  return String(timeStr);
}

String getCurrentDateString() {
  if (!timeInitialized) return "--/--/----";
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "--/--/----";
  }
  
  char dateStr[32];
  strftime(dateStr, sizeof(dateStr), "%B | %d | %Y", &timeinfo);
  return String(dateStr);
}

String getCurrentDayString() {
  if (!timeInitialized) return "-------";
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "-------";
  }
  
  char dayStr[16];
  strftime(dayStr, sizeof(dayStr), "%A", &timeinfo);
  return String(dayStr);
}

void initializeTime() {
  if (timeInitialized) return;
  
  Serial.println("Initializing time...");
  
  // Connect to WiFi if not connected
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    int wifi_attempts = 0;
    while (WiFi.status() != WL_CONNECTED && wifi_attempts < 60) {
      delay(500);
      Serial.print(".");
      wifi_attempts++;
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\nWiFi connection failed!");
      return;
    }
    Serial.println("\nWiFi connected");
  }

  // Configure time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Wait for time sync
  struct tm timeinfo;
  int attempts = 0;
  while (!getLocalTime(&timeinfo) && attempts < 30) {
    Serial.println("Waiting for NTP time sync...");
    delay(1000);
    attempts++;
  }
  
  if (getLocalTime(&timeinfo)) {
    timeInitialized = true;
    Serial.println("Time synchronized!");
  } else {
    Serial.println("NTP sync failed!");
  }
}

void displayTime() {
  String dayStr = getCurrentDayString();
  String timeStr = getCurrentTimeString();
  String dateStr = getCurrentDateString();
  
  // First time drawing or after a reset
  if (!initialDrawDone) {
    // Clear the entire time/date area
    tft.fillRect(DISPLAY_X, DISPLAY_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT, TFT_BLACK);
    
    // Draw border around time/date section
    tft.drawRect(DISPLAY_X, DISPLAY_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT, TFT_WHITE);
    
    initialDrawDone = true;
  }
  
  // Only update day if it changed
  if (dayStr != prevDayStr) {
    // Clear previous day text area only
    tft.fillRect(DISPLAY_X + 5, DAY_Y, DISPLAY_WIDTH - 10, 20, TFT_BLACK);
    
    // Display day of week
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN);
    int dayWidth = dayStr.length() * 12; // Approximate width for size 2
    int dayX = (240 - dayWidth) / 2;
    tft.setCursor(dayX, DAY_Y);
    tft.print(dayStr);
    
    prevDayStr = dayStr;
  }
  
  // Only update time if it changed
  if (timeStr != prevTimeStr) {
    // Clear previous time text area only
    tft.fillRect(DISPLAY_X + 5, TIME_Y, DISPLAY_WIDTH - 10, 30, TFT_BLACK);
    
    // Display time
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE);
    int timeWidth = timeStr.length() * 18; // Approximate width for size 3
    int timeX = (240 - timeWidth) / 2;
    tft.setCursor(timeX, TIME_Y);
    tft.print(timeStr);
    
    prevTimeStr = timeStr;
  }
  
  // Only update date if it changed
  if (dateStr != prevDateStr) {
    // Clear previous date text area only
    tft.fillRect(DISPLAY_X + 5, DATE_Y, DISPLAY_WIDTH - 10, 20, TFT_BLACK);
    
    // Display date
    tft.setTextSize(2);
    int dateWidth = dateStr.length() * 6;
    int dateX = (145 - dateWidth) / 2;
    tft.setCursor(dateX, DATE_Y);
    tft.print(dateStr);
    
    prevDateStr = dateStr;
  }
}

#endif