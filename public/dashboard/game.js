const canvas = document.getElementById("gridCanvas");
const ctx = canvas.getContext("2d");
const gridSize = 8;
const squareSize = canvas.width / gridSize;

let ships = [
  { size: 4, placed: false, tiles: [] },
  { size: 3, placed: false, tiles: [] },
  { size: 2, placed: false, tiles: [] },
  { size: 1, placed: false, tiles: [] },
];
let currentShipIndex = 0; // Track the current ship being placed
let grid = Array(gridSize)
  .fill()
  .map(() => Array(gridSize).fill(false)); // Grid to track placements

//Draw the grid and any placed ships
function drawGrid() {
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  for (let row = 0; row < gridSize; row++) {
    for (let col = 0; col < gridSize; col++) {
      ctx.strokeRect(
        col * squareSize,
        row * squareSize,
        squareSize,
        squareSize,
      );
      if (grid[row][col]) {
        ctx.fillStyle = "gray"; // Color for filled squares
        ctx.fillRect(
          col * squareSize,
          row * squareSize,
          squareSize,
          squareSize,
        );
      }
    }
  }
  //Draw ships
  ships.forEach((ship) => {
    ship.tiles.forEach((tile) => {
      ctx.fillStyle = "gray";
      ctx.fillRect(
        tile.col * squareSize,
        tile.row * squareSize,
        squareSize,
        squareSize,
      );
    });
  });
}

//Listens for clicks then places a "ship tile"
canvas.addEventListener("click", (e) => {
  if (currentShipIndex >= ships.length) return; // No more ships to place

  const x = e.offsetX;
  const y = e.offsetY;
  const row = Math.floor(y / squareSize);
  const col = Math.floor(x / squareSize);

  let currentShip = ships[currentShipIndex];

  //Checks if the square is already occupied or not
  if (grid[row][col]) return;

  //This checks if there is enough tiles left to place
  if (currentShip.tiles.length < currentShip.size) {
    currentShip.tiles.push({ row, col });
    grid[row][col] = true;
  }

  if (currentShip.tiles.length === currentShip.size) {
    currentShip.placed = true;
    currentShipIndex++;
    if (currentShipIndex < ships.length) {
      document.getElementById("status").textContent =
        `Place your ${ships[currentShipIndex].size}-sized ship...`;
    } else {
      document.getElementById("status").textContent =
        "All ships placed! Locking the board.";
      setTimeout(() => {
        alert("Game started!");
        // Here you can proceed to start the actual game logic
      }, 1000);
    }
  }
  drawGrid();
});

// Initial drawing of the grid
drawGrid();
