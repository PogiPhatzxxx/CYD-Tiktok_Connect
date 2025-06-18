# TikTok Live Display Project

## Table of Contents

- [Changing WiFi Credentials](#changing-wifi-credentials)
- [Changing TikTok Username](#changing-tiktok-username)
- [Changing WebSocket Server Settings](#changing-websocket-server-settings)
- [Installing Node.js](#installing-nodejs)
- [Replace and Backing Up Arduino Libraries](#replace-and-backing-up-arduino-libraries)
- [Running the Server](#running-the-server)
- [Credits](#credits)

## Changing WiFi Credentials

To connect your device to your WiFi network, you need to modify the credentials in the `time.h` file:

1. Open the project in Arduino IDE
2. Navigate to the `time.h` file
3. Locate the following lines (around line 8-9):

```cpp
// WiFi credentials
const char* ssid = "REPLACE_SSID";
const char* password = "REPLACE_SSID_PASSWORD";
```

## Changing TikTok Username
To display live events from a different TikTok account, modify the username in the server.js file:

1. Navigate to the Server folder
2. Open server.js
3. Find this line (around line 13):

```js
const tiktokUsername = 'PUT_USERNAME_HERE'; // Replace with actual username
```

## Changing WebSocket Server Settings
To ensure your ESP32 device can connect to the WebSocket server running on your PC:

1. Find your PC's IP address on your local network (use ipconfig in Command Prompt on Windows)
2. Open the tiktok_live.h file
3. Locate the following lines (around line 12-13):

```
const char* websocket_server = "REPLACE_IP_ADDRESS";
const int websocket_port = 3000;
```
   
## Installing Node.js
The server component requires Node.js to run:

1. Download Node.js from https://nodejs.org/ (LTS version recommended)
2. Run the installer and follow the installation wizard
3. Verify installation by opening Command Prompt and typing:

```
node --version
npm --version
```
## Replace and Backing Up Arduino Libraries
To back up the required libraries for this project:

1. Locate your Arduino libraries folder (typically in Documents\Arduino\libraries )
2. Copy the following folders to your backup location:
   - ArduinoJson
   - TFT_eSPI
   - WebSockets
3. Replace with the Library Provided Above
  
## Running the Server
1. Open Command Prompt in Server Directory
2. Run this:
  
```
node server.js
```

## Credits
This project uses the [TikTok-Live-Connector](https://github.com/zerodytrash/TikTok-Live-Connector) library for connecting to TikTok live streams.