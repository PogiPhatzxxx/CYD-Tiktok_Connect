#ifndef KAOMOJI_H
#define KAOMOJI_H

#include <Arduino.h>
#include <TFT_eSPI.h>

// External TFT reference
extern TFT_eSPI tft;

// Arrays of simple ASCII kaomoji parts
const char* leftSides[] = {"(", "(", "(", "(", "("};
const char* rightSides[] = {")", ")", ")", ")", ")"};
const char* eyes[] = {"^", "o", "O", "*", "-", ".", ">", "<"};
const char* mouths[] = {"_", "-", "~", "w", "v", "o", "u", "^"};

// Size of each part array
const int leftSize = sizeof(leftSides) / sizeof(leftSides[0]);
const int rightSize = sizeof(rightSides) / sizeof(rightSides[0]);
const int eyeSize = sizeof(eyes) / sizeof(eyes[0]);
const int mouthSize = sizeof(mouths) / sizeof(mouths[0]);

// Display position for kaomoji
const int KAOMOJI_Y = 15;

// Previous kaomoji to avoid unnecessary redrawing
String prevKaomoji = "";

// Time of last kaomoji change
unsigned long lastKaomojiChange = 0;
const unsigned long kaomojiChangeInterval = 5000; // Change every 5 seconds

String generateFace() {
  String face = "";
  face += leftSides[random(leftSize)];
  String eye = eyes[random(eyeSize)];
  face += eye + mouths[random(mouthSize)] + eye;
  face += rightSides[random(rightSize)];
  return face;
}

void displayKaomoji() {
  unsigned long currentMillis = millis();
  
  // Only update kaomoji every 5 seconds or if it's the first time
  if (currentMillis - lastKaomojiChange >= kaomojiChangeInterval || prevKaomoji == "") {
    String kaomoji = generateFace();
    
    // Calculate positions for better centering
    tft.setTextSize(6);
    int kaomojiWidth = kaomoji.length() * 36; // Approximate width for size 6
    int kaomojiX = (240 - kaomojiWidth) / 2;
    
    // Clear the entire kaomoji area with an even larger margin
    // Extend the clearing area in all directions
    tft.fillRect(0, KAOMOJI_Y - 10, 240, 55, TFT_BLACK);
    
    delay(5);
    
    // Display new kaomoji
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(kaomojiX, KAOMOJI_Y);
    tft.print(kaomoji);
    
    prevKaomoji = kaomoji;
    lastKaomojiChange = currentMillis;
  }
}

#endif