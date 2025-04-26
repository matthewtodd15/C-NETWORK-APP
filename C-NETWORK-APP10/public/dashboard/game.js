var gameRunning = true;

function initCanvas() {
  if (!canvas.getContext) {
    console.error("Error: canvas not supported in this browser");
    return;
  }

  CTX = canvas.getContext("2d");

  // start the game loop
  LAST_FRAME_TIME = new Date().getTime();
  gameIntervalId = setInterval(() => gameloop(), 1 / FPS);
}

// all position updates, frame updates should happen here (no draws)
function update() {
  updateWaves();
  cursor.update();

  LAST_FRAME_TIME = new Date().getTime();
}

// all drawing should happen here
function draw() {
  // draw background
  CTX.save();
  CTX.fillStyle = BACKGROUND_COL;
  CTX.fillRect(0, 0, canvas.width, canvas.height);
  CTX.restore();

  playerBoard.draw();
  drawShotBoard();
  drawWaves();

  ships.forEach((ship) => {
    ship.draw();
  });
  cursor.draw();
}

function gameloop() {
  if (!gameRunning) {
    return;
  }

  draw();
  update();
}

function drawShotBoard() {
  CTX.save();
  CTX.fillStyle = BOARD_COL;
  const padding = 1;
  for (let row = 0; row < BOARD_DIM; row++) {
    for (let col = 0; col < BOARD_DIM; col++) {
      CTX.fillRect(
        SHOT_BOARD_OFFSET_X + row * SHOT_BOARD_UNIT_SIZE + padding,
        SHOT_BOARD_OFFSET_Y + col * SHOT_BOARD_UNIT_SIZE + padding,
        SHOT_BOARD_UNIT_SIZE - padding * 2,
        SHOT_BOARD_UNIT_SIZE - padding * 2,
      );
    }
  }

  CTX.restore();
}
