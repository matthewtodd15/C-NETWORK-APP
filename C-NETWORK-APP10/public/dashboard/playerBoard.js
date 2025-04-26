class PlayerBoard {
  constructor() {
    this.board = Array(BOARD_DIM)
      .fill()
      .map(() => Array(BOARD_DIM).fill(0));
  }

  draw() {
    CTX.save();

    /* another way to draw the board
    CTX.strokeStyle = BOARD_COL;

    // draw vertical gridlines
    for (let col = 0; col < BOARD_DIM + 1; col++) {
      CTX.beginPath();
      CTX.moveTo(col * BOARD_UNIT_SIZE + BOARD_OFFSET_X, BOARD_OFFSET_Y);
      CTX.lineTo(
        col * BOARD_UNIT_SIZE + BOARD_OFFSET_X,
        BOARD_OFFSET_Y + BOARD_HEIGHT,
      );
      CTX.stroke();
    }
    // draw horizontal gridlines
    for (let row = 0; row < BOARD_DIM + 1; row++) {
      CTX.beginPath();
      CTX.moveTo(BOARD_OFFSET_X, row * BOARD_UNIT_SIZE + BOARD_OFFSET_Y);
      CTX.lineTo(
        BOARD_OFFSET_X + BOARD_WIDTH,
        row * BOARD_UNIT_SIZE + BOARD_OFFSET_Y,
      );
      CTX.stroke();
    }
    */

    CTX.fillStyle = BOARD_COL;
    const padding = 2;
    for (let row = 0; row < BOARD_DIM; row++) {
      for (let col = 0; col < BOARD_DIM; col++) {
        CTX.fillRect(
          BOARD_OFFSET_X + row * BOARD_UNIT_SIZE + padding,
          BOARD_OFFSET_Y + col * BOARD_UNIT_SIZE + padding,
          BOARD_UNIT_SIZE - padding * 2,
          BOARD_UNIT_SIZE - padding * 2,
        );
      }
    }
    CTX.restore();
  }

  /**
   * helper for forcing a board position from an x, y position
   * returns { boardRow, boardCol } on board relative to top left corner
   *
   * 1 2 ... 8
   * 2
   * ...
   * 8
   *
   */
  getBoardPosFromXY(x, y) {
    let boardXPosPixels = Math.max(x - BOARD_OFFSET_X, 0);
    let boardYPosPixels = Math.max(y - BOARD_OFFSET_Y, 0);
    let row = Math.floor(boardYPosPixels / BOARD_UNIT_SIZE);
    let col = Math.floor(boardXPosPixels / BOARD_UNIT_SIZE);

    row = Math.min(row, BOARD_DIM - 1);
    col = Math.min(col, BOARD_DIM - 1);

    return {
      boardRow: row,
      boardCol: col,
      x: BOARD_OFFSET_X + boardXPosPixels,
      y: BOARD_OFFSET_Y + boardYPosPixels,
    };
  }

  placeShip(ship) {
    const { placeX, placeY } = ship.getPlacePoints();
    const { boardRow, boardCol, x, y } = playerBoard.getBoardPosFromXY(
      placeX,
      placeY,
    );

    let rowOffset = 0,
      columnOffset = 0;

    if (ship.direction == DIRECTION_DOWN) {
      rowOffset = ship.length;
    } else {
      columnOffset = ship.length;
    }

    let adjRow = Math.min(boardRow, BOARD_DIM - rowOffset);
    let adjCol = Math.min(boardCol, BOARD_DIM - columnOffset);

    // make sure no overlap, if overlap, place back in initial position.
    if (
      !this.isValidDrop(ship.id, adjRow, adjCol, ship.length, ship.direction)
    ) {
      ship.x = ship.boardCol * BOARD_UNIT_SIZE + BOARD_OFFSET_X;
      ship.y = ship.boardRow * BOARD_UNIT_SIZE + BOARD_OFFSET_Y;
      ship.direction = ship.originalDir;
      return;
    }

    ship.x = adjCol * BOARD_UNIT_SIZE + BOARD_OFFSET_X;
    ship.y = adjRow * BOARD_UNIT_SIZE + BOARD_OFFSET_Y;

    // update board
    this.moveShip(ship, adjRow, adjCol);

    ship.boardRow = adjRow;
    ship.boardCol = adjCol;
    ship.originalDir = ship.direction;
  }

  // check all positions under ship to make sure we can drop it
  isValidDrop(id, row, col, length, dir) {
    for (let i = 0; i < length; i++) {
      if (this.board[row][col] !== 0 && this.board[row][col] !== id) {
        return false;
      }

      if (dir === DIRECTION_DOWN) {
        row++;
      } else {
        col++;
      }
    }

    return true;
  }

  moveShip(ship, newRow, newCol) {
    // clear ship from board
    let row = ship.boardRow;
    let col = ship.boardCol;
    for (let i = 0; i < ship.length; i++) {
      this.board[row][col] = 0;

      if (ship.originalDir === DIRECTION_DOWN) {
        row++;
      } else {
        col++;
      }
    }

    // add ship to new position
    row = newRow;
    col = newCol;
    for (let i = 0; i < ship.length; i++) {
      this.board[row][col] = ship.id;

      if (ship.direction === DIRECTION_DOWN) {
        row++;
      } else {
        col++;
      }
    }
  }
}
