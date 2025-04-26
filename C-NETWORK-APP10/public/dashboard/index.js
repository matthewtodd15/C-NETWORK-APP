const GameMsg = {
  READY: 0, // client -> server (put me in a game)
  SYNC: 1, // client refresh or reconnect (includes all game data)
  BOARD_SETUP: 2, // client -> server (my board is ready)
  TURN: 3, // server -> client (whose turn, and what happened)
  SHOT: 4, // client -> server (shot x, y)
  RESIGN: 5, // client -> server (player resigns)
  ERROR: 6, // general error
  CLOSE: 7, // game over or user leaves
  PING: 8, // client -> server (are we still in an active game?)
  PONG: 9, // client (yes, game still active)
};

console.log("Connecting to server...");
const websocket = new WebSocket("ws://localhost:4242/ws");
websocket.binaryType = "arraybuffer";

// Must send after connections are open
websocket.onopen = (event) => {
  console.log("Websocket successfully connected");
};

websocket.onmessage = (event) => {
  console.log(event.data);

  const msgData = new Uint8Array(event.data);
  const opcode = msgData[0] >> 4;

  switch (opcode) {
    case GameMsg.READY:
      console.log("Player joined a game");
      statusText.innerHTML = "Joined game!";
      break;
    case GameMsg.SYNC:
      console.log("Syncing game state");
      break;
    case GameMsg.BOARD_SETUP:
      console.log("Board setup received");
      canMoveShips = false;
      statusText.innerHTML = "Board Confirmed!";
      break;
    case GameMsg.TURN:
      console.log("Turn update from server");
      break;
    case GameMsg.SHOT:
      console.log("Player fired a shot");
      break;
    case GameMsg.RESIGN:
      console.log("Player resigned");
      break;
    case GameMsg.ERROR:
      const decoder = new TextDecoder();
      console.error("Received an error:", decoder.decode(msgData.slice(1)));
      break;
    case GameMsg.CLOSE:
      console.log("Game closed");
      break;
    case GameMsg.PING:
      console.log("Ping from client");
      break;
    case GameMsg.PONG:
      console.log("Pong from server");
      break;
    default:
      console.warn("Unknown opcode:", opcode);
  }
};

websocket.onerror = (event) => {
  console.log("Connection error");
  console.log(event);
};

websocket.onclose = (event) => {
  console.log("Connection closed");
  console.log(event);
};
