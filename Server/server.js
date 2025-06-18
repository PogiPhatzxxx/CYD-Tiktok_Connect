const { TikTokLiveConnection, WebcastEvent } = require('tiktok-live-connector');
const WebSocket = require('ws');
const express = require('express');
const http = require('http');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ 
    server,
    clientTracking: true,
    maxPayload: 65536 
});

const esp32Clients = new Set();
const tiktokUsername = 'uradatiktok'; // Replace with actual username

const tiktokConnection = new TikTokLiveConnection(tiktokUsername, {
    processInitialData: true,
    enableExtendedGiftInfo: true,
    requestPollingIntervalMs: 1000,  // Reduced polling interval for faster updates
    enableWebsocketUpgrade: true,
    requestHeaders: {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36'
    },
    reconnectIntervalMillis: 2000,   // Faster reconnection
    maxReconnectAttempts: 10         // More reconnection attempts
});

const pingInterval = setInterval(() => {
    wss.clients.forEach((ws) => {
        if (ws.isAlive === false) return ws.terminate();
        ws.isAlive = false;
        ws.ping();
    });
}, 20000); // More frequent health checks

server.on('close', () => clearInterval(pingInterval));

wss.on('connection', (ws) => {
    ws.isAlive = true;
    esp32Clients.add(ws);
    
    ws.send(JSON.stringify({
        type: 'connection',
        status: 'connected',
        timestamp: Date.now()
    }));
    
    ws.on('pong', () => ws.isAlive = true);
    
    ws.on('close', () => esp32Clients.delete(ws));
    
    ws.on('error', (error) => esp32Clients.delete(ws));
});

function broadcastToESP32(data) {
    const message = JSON.stringify(data);
    esp32Clients.forEach(client => {
        try {
            if (client.readyState === WebSocket.OPEN) {
                client.send(message);
            }
        } catch (error) {
            esp32Clients.delete(client);
        }
    });
}

let reconnectAttempts = 0;
const maxReconnectAttempts = 15; // Increased max attempts
let reconnectTimeout;

// Register all available TikTok events to capture everything
const allEvents = Object.values(WebcastEvent);
allEvents.forEach(eventName => {
    tiktokConnection.on(eventName, (data) => {
        // Handle specific events with custom formatting
        if (eventName === WebcastEvent.CONNECTED) {
            reconnectAttempts = 0;
            broadcastToESP32({
                type: 'tiktok_connected',
                roomId: data.roomId,
                timestamp: Date.now()
            });
        } 
        else if (eventName === WebcastEvent.DISCONNECTED) {
            broadcastToESP32({
                type: 'tiktok_disconnected',
                timestamp: Date.now()
            });
            
            if (reconnectAttempts < maxReconnectAttempts) {
                const delay = Math.min(1000 * Math.pow(1.5, reconnectAttempts), 15000); // Faster exponential backoff
                clearTimeout(reconnectTimeout);
                reconnectTimeout = setTimeout(() => {
                    reconnectAttempts++;
                    tiktokConnection.connect().catch(() => {});
                }, delay);
            }
        }
        else if (eventName === WebcastEvent.CHAT) {
            broadcastToESP32({
                type: 'chat',
                username: data.user.uniqueId,
                message: data.comment,
                timestamp: Date.now()
            });
        }
        else if (eventName === WebcastEvent.GIFT) {
            broadcastToESP32({
                type: 'gift',
                username: data.user.uniqueId,
                giftName: data.giftName || `Gift ${data.giftId}`,
                giftId: data.giftId,
                timestamp: Date.now()
            });
        }
        else if (eventName === WebcastEvent.LIKE) {
            broadcastToESP32({
                type: 'like',
                username: data.user.uniqueId,
                likeCount: data.likeCount,
                timestamp: Date.now()
            });
        }
        else if (eventName === WebcastEvent.FOLLOW) {
            broadcastToESP32({
                type: 'follow',
                username: data.user.uniqueId,
                timestamp: Date.now()
            });
        }
        else if (eventName === WebcastEvent.ROOMUSER) {
            broadcastToESP32({
                type: 'viewers',
                count: data.viewerCount,
                timestamp: Date.now()
            });
        }
        else {
            // Broadcast any other events with generic format
            try {
                broadcastToESP32({
                    type: eventName.toLowerCase(),
                    data: data,
                    timestamp: Date.now()
                });
            } catch (e) {}
        }
    });
});

tiktokConnection.on('error', (error) => {
    broadcastToESP32({
        type: 'error',
        message: error.message,
        timestamp: Date.now()
    });
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
    const connectWithRetry = () => {
        tiktokConnection.connect().catch(err => {
            if (reconnectAttempts < maxReconnectAttempts) {
                reconnectAttempts++;
                const delay = Math.min(1000 * Math.pow(1.5, reconnectAttempts), 15000);
                setTimeout(connectWithRetry, delay);
            }
        });
    };
    
    connectWithRetry();
});

process.on('SIGINT', () => {
    clearInterval(pingInterval);
    clearTimeout(reconnectTimeout);
    wss.clients.forEach(client => client.terminate());
    tiktokConnection.disconnect();
    server.close(() => process.exit(0));
});