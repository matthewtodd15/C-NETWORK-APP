const statusText = document.getElementById("status-text");

function handleJoinGame() {
  // send ready up message
  // set state here or update page to show ready up
  // wait for updates
  if (websocket.readyState != WebSocket.OPEN) {
    console.log("Error: connection to server failed");
    return;
  }

  let message = new Uint8Array(1);
  message[0] = 0x00; // ready op

  websocket.send(message);

  statusText.innerHTML = "Waiting for game...";
}

function handleLogout() {
  // send any close or stop messages to server
  // navigate to logout page
  window.location.href = "/";
}

function handleConfirmPosition() {
  if (websocket.readyState != WebSocket.OPEN) {
    console.log("Error: connection to server failed");
    return;
  }

  let message = new Uint8Array(65);

  for (let i = 0; i < 8; i++) {
    for (let j = 0; j < 8; j++) {
      message[i * 8 + j + 1] = playerBoard.board[i][j];
    }
  }
  message[0] = 0x20; // board setup op

  websocket.send(message);

  statusText.innerHTML = "Confirming Position...";
}

function handleResign() {}

function handleSendShot() {}
