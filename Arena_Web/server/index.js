const WebSocket = require('ws');

const wss = new WebSocket.Server({port: 8082});

// ws refers to a single connection, wss is the server
wss.on("connection", ws => {
    console.log("Client connected!");

    ws.on("close", () => {
        console.log("Client has disconnected!");
    })
});