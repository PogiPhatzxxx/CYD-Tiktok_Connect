    #ifndef TIKTOK_LIVE_H
    #define TIKTOK_LIVE_H

    #include <Arduino.h>
    #include <WiFi.h>
    #include <WebSocketsClient.h>
    #include <ArduinoJson.h>
    #include <TFT_eSPI.h>

    // External TFT reference
    extern TFT_eSPI tft;

    // WebSocket server details (your Node.js server)
    const char* websocket_server = "REPLACE_IP_ADDRESS"; // Replace with your server IP
    const int websocket_port = 3000;

    // WebSocket client
    WebSocketsClient webSocket;

    // Connection state tracking
    bool isConnecting = false;
    long lastReconnectAttempt = 0;
    int reconnectAttempts = 0;
    const int maxReconnectAttempts = 5;
    const int reconnectBaseDelay = 5000; // 5 seconds base delay

    // Ping/pong tracking
    unsigned long lastPingTime = 0;
    const unsigned long pingInterval = 30000; // 30 seconds

    // Display variables
    int scrollY = 0;
    const int lineHeight = 25; // Reduced from 30 to fit better on screen
    const int maxLines = 4; // Keeping the same number of lines
    String displayLines[maxLines][2]; // Changed to 2D array to store username and content separately
    int currentLine = 0;

    // TikTok display position (on the left side)
    const int TIKTOK_X = 5; // Position on the left side
    const int TIKTOK_Y = 210; // Moved down further from 190 to avoid overlap with time display
    const int TIKTOK_WIDTH = 230; // Width of the TikTok section
    const int TIKTOK_HEIGHT = 100; // Further reduced height to fit better on screen

    // Colors
    #define TL_BLACK 0x0000
    #define TL_WHITE 0xFFFF
    #define TL_RED 0xF800
    #define TL_GREEN 0x07E0
    #define TL_BLUE 0x001F
    #define TL_YELLOW 0xFFE0
    #define TL_MAGENTA 0xF81F
    #define TL_CYAN 0x07FF

    // Function declarations
    void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
    void handleMessage(String message);
    void addLine(String username, String content, uint16_t color); // Updated function declaration
    void initializeTikTokLive();
    void updateTikTokLive();
    bool isTikTokLiveInitialized = false;

    void initializeTikTokLive() {
        // Clear display lines
        for(int i = 0; i < maxLines; i++) {
            displayLines[i][0] = ""; // Initialize first dimension
            displayLines[i][1] = ""; // Initialize second dimension
        }
        
        // Draw border around TikTok section
        tft.drawRect(TIKTOK_X, TIKTOK_Y, TIKTOK_WIDTH, TIKTOK_HEIGHT, TL_WHITE);
        
        // Show startup message - simplified
        addLine("System", "Waiting for Server", TL_YELLOW);
        
        // Setup WebSocket with improved settings
        webSocket.begin(websocket_server, websocket_port, "/");
        webSocket.onEvent(webSocketEvent);
        webSocket.setReconnectInterval(reconnectBaseDelay);
        
        // Enable ping/pong for connection health monitoring
        webSocket.enableHeartbeat(15000, 3000, 2); // Send ping every 15s, pong timeout 3s, 2 retries
        
        isConnecting = true;
        reconnectAttempts = 0;
        lastReconnectAttempt = millis();
        lastPingTime = millis();
        
        isTikTokLiveInitialized = true;
    }

    void updateTikTokLive() {
        if (isTikTokLiveInitialized) {
            // Handle WebSocket loop
            webSocket.loop();
            
            // Check connection status and implement reconnection with exponential backoff
            unsigned long currentTime = millis();
            
            // If disconnected and not already trying to reconnect
            if (!webSocket.isConnected() && !isConnecting) {
                // Calculate exponential backoff delay (with maximum cap)
                int reconnectDelay = min(reconnectBaseDelay * (1 << min(reconnectAttempts, 5)), 300000); // Max 5 minutes
                
                if (currentTime - lastReconnectAttempt > reconnectDelay) {
                    if (reconnectAttempts < maxReconnectAttempts) {
                        Serial.printf("Attempting to reconnect (Attempt %d/%d)...\n", reconnectAttempts + 1, maxReconnectAttempts);
                        addLine("System", "Reconnecting...", TL_YELLOW); // Fixed: added username parameter
                        
                        webSocket.begin(websocket_server, websocket_port, "/");
                        isConnecting = true;
                        lastReconnectAttempt = currentTime;
                        reconnectAttempts++;
                    } else if (reconnectAttempts == maxReconnectAttempts) {
                        // Reset after max attempts to try again after a longer delay
                        Serial.println("Maximum reconnection attempts reached. Will retry in 5 minutes.");
                        addLine("System", "Retry in 5min", TL_RED); // Fixed: added username parameter
                        lastReconnectAttempt = currentTime;
                        reconnectAttempts++; // Increment to prevent repeated messages
                    } else if (currentTime - lastReconnectAttempt > 300000) { // 5 minutes
                        // Reset counter and try again
                        reconnectAttempts = 0;
                    }
                }
            }
            
            // Send periodic ping if connected (as an additional health check)
            if (webSocket.isConnected() && currentTime - lastPingTime > pingInterval) {
                webSocket.sendPing();
                lastPingTime = currentTime;
            }
        }
    }

    void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
        switch(type) {
            case WStype_DISCONNECTED:
                Serial.println("WebSocket Disconnected");
                addLine("System", "Disconnected", TL_RED);
                isConnecting = false;
                break;
                
            case WStype_CONNECTED:
                Serial.printf("WebSocket Connected to: %s\n", payload);
                addLine("System", "Connected", TL_GREEN);
                isConnecting = false;
                reconnectAttempts = 0; // Reset reconnect attempts on successful connection
                break;
                
            case WStype_TEXT:
                // Process the message and update display for chat messages
                Serial.printf("Received: %s\n", payload);
                handleMessage((char*)payload);
                break;
                
            case WStype_PING:
                // Respond to ping (though the library should handle this automatically)
                Serial.println("Received ping");
                break;
                
            case WStype_PONG:
                // Response to our ping
                Serial.println("Received pong");
                break;
                
            case WStype_ERROR:
                Serial.println("WebSocket Error");
                break;
                
            default:
                break;
        }
    }

    // Modified to display chat messages
    void handleMessage(String message) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, message);
        
        String type = doc["type"];
        
        // Display all message types with username above and content below
        if (type == "chat") {
            String username = doc["username"];
            String msg = doc["message"];
            addLine(username, "Comment: " + msg, TL_WHITE);
        }
        else if (type == "gift") {
            String username = doc["username"];
            String giftName = doc["giftName"];
            addLine(username, "Gift: " + giftName, TL_MAGENTA);
            Serial.println("Gift: " + username + " sent " + giftName);
        }
        else if (type == "like") {
            String username = doc["username"];
            addLine(username, "Like", TL_CYAN);
            Serial.println("Like: " + username);
        }
        else if (type == "follow") {
            String username = doc["username"];
            addLine(username, "Follow", TL_YELLOW);
            Serial.println("Follow: " + username);
        }
        else {
            // Log other events to Serial only
            if (type == "connection") {
                Serial.println("ESP32 connected!");
            }
            else if (type == "tiktok_connected") {
                String roomId = doc["roomId"];
                Serial.println("TikTok Live connected to room: " + roomId);
                addLine("TikTok Live", "Connected", TL_GREEN);
            }
            else if (type == "tiktok_disconnected") {
                Serial.println("TikTok Live disconnected");
                addLine("TikTok Live", "Disconnected", TL_RED);
            }
            else if (type == "viewers") {
                int count = doc["count"];
                Serial.println("Viewers: " + String(count));
            }
            else if (type == "error") {
                String errorMsg = doc["message"];
                Serial.println("Error: " + errorMsg);
                addLine("Error", errorMsg, TL_RED);
            }
        }
    }

    void addLine(String username, String content, uint16_t color) {
        // Add new line to buffer
        displayLines[currentLine][0] = username;
        displayLines[currentLine][1] = content;
        currentLine = (currentLine + 1) % maxLines;
        
        // Clear TikTok area (inside the border)
        tft.fillRect(TIKTOK_X + 1, TIKTOK_Y + 1, TIKTOK_WIDTH - 2, TIKTOK_HEIGHT - 2, TL_BLACK);
        
        int y = TIKTOK_Y + 5;
        int maxY = TIKTOK_Y + TIKTOK_HEIGHT - 5; // Maximum Y position to stay inside border
        
        for(int i = 0; i < maxLines; i++) {
            int lineIndex = (currentLine + i) % maxLines;
            if(displayLines[lineIndex][0] != "") {
                // Check if we have enough space for at least the username
                if(y + 10 > maxY) break; // Stop if we're about to exceed the border
                
                // Get content and determine if it's long
                String displayContent = displayLines[lineIndex][1];
                int maxCharsPerLine = 35; // Maximum characters per line
                bool isLongContent = displayContent.length() > maxCharsPerLine;
                
                // For long content, display content first then username
                if(isLongContent) {
                    // Display content first
                    tft.setTextColor(color, TL_BLACK);
                    tft.setCursor(TIKTOK_X + 5, y); // Align text to the left with a small margin
                    tft.setTextSize(1); // Normal font for content
                    
                    // Handle long content by wrapping to new lines
                    int startPos = 0;
                    int linesDisplayed = 0;
                    int maxContentLines = 3; // Maximum number of content lines to display
                    
                    while(startPos < displayContent.length() && linesDisplayed < maxContentLines) {
                        // Check if we have enough space for this line
                        if(y + 10 > maxY) break; // Stop if we're about to exceed the border
                        
                        int charsToShow = min(maxCharsPerLine, (int)displayContent.length() - startPos);
                        String lineContent = displayContent.substring(startPos, startPos + charsToShow);
                        tft.println(lineContent);
                        
                        startPos += charsToShow;
                        y += 10; // Add space for each additional line
                        linesDisplayed++;
                        
                        // If there's more content to show, set cursor for next line
                        if(startPos < displayContent.length() && linesDisplayed < maxContentLines) {
                            // Check if we have enough space for another line
                            if(y + 10 > maxY) break; // Stop if we're about to exceed the border
                            tft.setCursor(TIKTOK_X + 5, y);
                        }
                    }
                    
                    // Add extra spacing between content and username to prevent overlap
                    y += 5; // Add 5 more pixels of space
                    
                    // Now display username below content if there's space
                    if(y + 10 <= maxY) {
                        // Display username with smaller font
                        tft.setTextColor(TL_YELLOW, TL_BLACK); // Username in yellow
                        tft.setCursor(TIKTOK_X + 5, y); // Align text to the left with a small margin
                        tft.setTextSize(1); // Smaller username font
                        
                        // Truncate long usernames
                        String displayUsername = displayLines[lineIndex][0];
                        if(displayUsername.length() > 15) {
                            displayUsername = displayUsername.substring(0, 12) + "...";
                        }
                        
                        tft.println(displayUsername);
                        y += 10; // Space after username
                    }
                    
                    // Add spacing after entry
                    y += lineHeight - 10 - (linesDisplayed * 10) - 10; // Adjusted spacing
                } else {
                    // For short content, keep username above content (original behavior)
                    // Display username with smaller font
                    tft.setTextColor(TL_YELLOW, TL_BLACK); // Username in yellow
                    tft.setCursor(TIKTOK_X + 5, y); // Align text to the left with a small margin
                    tft.setTextSize(1); // Smaller username font
                    
                    // Truncate long usernames
                    String displayUsername = displayLines[lineIndex][0];
                    if(displayUsername.length() > 15) {
                        displayUsername = displayUsername.substring(0, 12) + "...";
                    }
                    
                    tft.println(displayUsername);
                    y += 10; // Reduced space after username
                    
                    // Check if we have enough space for content
                    if(y + 10 > maxY) break; // Stop if we're about to exceed the border
                    
                    // Display content
                    tft.setTextColor(color, TL_BLACK);
                    tft.setCursor(TIKTOK_X + 5, y); // Align text to the left with a small margin
                    tft.setTextSize(1); // Normal font for content
                    tft.println(displayContent);
                    y += lineHeight - 10; // Adjusted for next entry
                }
            }
        }
        
        Serial.println(username + ": " + content);
    }

    #endif