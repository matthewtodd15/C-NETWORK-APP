const WebSocket = require("ws");

const GameMsg = {
  READY: 0,
  BOARD_SETUP: 2,
  SHOT: 4,
  RESIGN: 5,
  PING: 8
};

const TOTAL_CLIENTS = 6;
const clients = [];

function sendBoardSetup(ws) {
  const dummyBoard = new Array(64).fill(0); // simplified board
  const message = new Uint8Array([GameMsg.BOARD_SETUP, ...dummyBoard]);
  ws.send(message);
  console.log("📤 Sent BOARD_SETUP");
}

function sendRandomShot(ws) {
  const x = Math.floor(Math.random() * 8);
  const y = Math.floor(Math.random() * 8);
  const message = new Uint8Array([GameMsg.SHOT, x, y]);
  ws.send(message);
  console.log(`📤 Sent SHOT at (${x}, ${y})`);
}

for (let i = 0; i < TOTAL_CLIENTS; i++) {
  const ws = new WebSocket("ws://localhost:4242/");
  
  ws.binaryType = "arraybuffer";

  ws.onopen = () => {
    console.log(`✅ Client ${i} connected`);
    ws.send(Uint8Array.from([GameMsg.READY]));

    setTimeout(() => {
      sendBoardSetup(ws);
    }, 1000 + i * 200);  // stagger board setup

    setInterval(() => {
      sendRandomShot(ws);
    }, 3000 + Math.random() * 2000);  // send shots over time
  };

  ws.onmessage = (event) => {
    console.log(`📨 Client ${i} got message:`, new Uint8Array(event.data));
  };

  ws.onerror = (err) => {
    console.error(`❌ Client ${i} error:`, err.message);
  };

  ws.onclose = () => {
    console.log(`🚪 Client ${i} disconnected`);
  };

  clients.push(ws);
}
