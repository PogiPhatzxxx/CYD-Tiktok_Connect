#include "time.h"
#include "kaomoji.h"
#include "tiktok_live.h"
#include "intro.h" // Include the intro header file

TFT_eSPI tft = TFT_eSPI(); // Create TFT instance

// Function to display the intro animation using all bitmap frames
void displayIntro() {
  Serial.println("Displaying intro animation...");
  tft.fillScreen(TFT_BLACK);
  
  // Loop through all 64 frames in the animation
  for (int i = 0; i < epd_bitmap_allArray_LEN; i++) {
    // Display current frame without clearing the screen between frames
    // This reduces flickering significantly
    tft.drawBitmap(0, 0, epd_bitmap_allArray[i], 240, 320, TFT_WHITE);
    
    // Slow down the animation to reduce eye strain
    delay(100); // 100ms delay = 10 frames per second (more comfortable)
    
    // Optional: Print debug info
    if (i % 10 == 0) { // Print only every 10 frames to reduce serial overhead
      Serial.print("Displaying frame ");
      Serial.println(i);
    }
  }
  
  Serial.println("Animation complete");
  delay(1000); // Pause at the end of animation
  
  Serial.println("Clearing screen...");
  tft.fillScreen(TFT_BLACK);
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0)); // Initialize random seed
  
  tft.init();
  tft.setRotation(0); // Landscape mode for 2.8" CYD
  
  // Show intro screen first
  displayIntro();
  
  // Then initialize other components
  initializeTime(); // Initialize WiFi and NTP
  initializeTikTokLive(); // Initialize TikTok Live connection
}

void loop() {
  if (isTimeInitialized()) {
    displayKaomoji(); // Display random kaomoji above the clock
    displayTime();    // Update the time display
  }
  updateTikTokLive(); // Update TikTok Live display
  delay(10); // Short delay for responsiveness
}