const canvas = document.getElementById("gridCanvas");
const cursorSpritesheet = document.getElementById("cursor");
const battleshipSpritesheet = document.getElementById("battleships");
const waveImg1 = document.getElementById("waves1");
const waveImg2 = document.getElementById("waves2");
const wavesImgs = [waveImg1, waveImg2];

// use if we want canvas to be max width & height
//canvas.width = window.innerWidth;
//canvas.height = window.innerHeight;

// all caps means global constant
// these values are designed to scale properly when the window scales
const FPS = 20;
var LAST_FRAME_TIME = null;
const UNIT_SIZE = 32; // px
const NUM_ROWS = canvas.width / UNIT_SIZE;
const NUM_COLUMNS = canvas.height / UNIT_SIZE;

const BACKGROUND_COL = "#5b6ee1";
const BOARD_COL = "#5162c8";
const TEXT_COL = "green";

const BOARD_DIM = 8;
const BOARD_WIDTH = BOARD_DIM * UNIT_SIZE; // px
const BOARD_HEIGHT = BOARD_DIM * UNIT_SIZE; // px
const BOARD_OFFSET_X = canvas.width / 2 - 50 - BOARD_WIDTH; // px (anchored to center)
const BOARD_OFFSET_Y = canvas.height / 2 - BOARD_HEIGHT / 2; // px (vertically centered)
const BOARD_UNIT_SIZE = BOARD_WIDTH / BOARD_DIM;

const SHOT_BOARD_WIDTH = BOARD_DIM * UNIT_SIZE * 0.8; // px
const SHOT_BOARD_HEIGHT = BOARD_DIM * UNIT_SIZE * 0.8; // px
const SHOT_BOARD_OFFSET_X = canvas.width / 2 + 50; // (anchored to center)
const SHOT_BOARD_OFFSET_Y = canvas.height / 2 - SHOT_BOARD_HEIGHT / 2; // px (vertically centered)
const SHOT_BOARD_UNIT_SIZE = SHOT_BOARD_WIDTH / BOARD_DIM;

// Directions
const DIRECTION_DOWN = 0;
const DIRECTION_RIGHT = 1;
const NUM_DIRECTIONS = 2;

const WAVE_SPEED = 2;
const waves = [];

var canMoveShips = true;

// Debug
const DEBUG_MODE = false;

var CTX = null;

var gameIntervalId = null;

let playerBoard = new PlayerBoard();
let ships = createShips();
let cursor = new Cursor(cursorSpritesheet);

document.addEventListener("mousemove", (e) => {
  const relativeX = e.clientX - canvas.offsetLeft;
  const relativeY = e.clientY - canvas.offsetTop;

  if (relativeX > 0 && relativeX < canvas.width) {
    cursor.x = relativeX;
  }
  if (relativeY > 0 && relativeY < canvas.width) {
    cursor.y = relativeY;
  }
});

// mouseup and mousedown for dragging ships
canvas.addEventListener("mouseup", (e) => {
  const x = e.offsetX;
  const y = e.offsetY;

  // check for click on ship for placement
  // check for click on shotboard
  // check for click on playerboard

  if (cursor.shipAttached != null) {
    cursor.dropShipOnGrid();
  }
});

canvas.addEventListener("mousedown", (e) => {
  const x = e.offsetX;
  const y = e.offsetY;

  // check for click on ship for placement
  // check for click on shotboard
  // check for click on playerboard

  if (cursor.shipAttached == null && canMoveShips) {
    for (let i = 0; i < ships.length; i++) {
      let ship = ships[i];
      const { x1, x2, y1, y2 } = ship.getBounds();
      if (x >= x1 && x <= x2 && y >= y1 && y <= y2) {
        cursor.attachShip(ship);
        break;
      }
    }
  }

  cursor.click(x, y, (c) =>
    console.log("This is a click callback: ", c.x, c.y),
  );
});

window.addEventListener("keydown", (event) => {
  let key = event.key;

  switch (key) {
    case "r":
      console.log("rotate");

      cursor.rotateShip();
    default:
  }
});

function getDeltaTime() {
  return (new Date().getTime() - LAST_FRAME_TIME) / 100;
}

window.onload = () => initCanvas();
