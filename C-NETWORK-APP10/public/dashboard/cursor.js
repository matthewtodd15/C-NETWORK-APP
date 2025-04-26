class Cursor {
  constructor(spritesheet) {
    this.currentFrame = 2;
    this.spritesheet = spritesheet;
    this.x = 0;
    this.y = 0;
    this.clickX = 0;
    this.clickY = 0;
    this.isClicked = false;
    this.clickAnimTimeout = 200; // ms
    this.shipAttached = null;
    this.initialOffsetX = 0;
    this.initialOffsetY = 0;
  }

  update() {
    if (this.shipAttached != null) {
      this.shipAttached.x = this.x - this.initialOffsetX;
      this.shipAttached.y = this.y - this.initialOffsetY;
    }
  }

  draw() {
    CTX.save();
    CTX.drawImage(
      this.spritesheet,
      this.currentFrame * UNIT_SIZE,
      0,
      UNIT_SIZE,
      UNIT_SIZE,
      this.x - UNIT_SIZE / 2,
      this.y - UNIT_SIZE / 2,
      UNIT_SIZE,
      UNIT_SIZE,
    );
    CTX.restore();
  }

  click(x, y, callback) {
    this.isClicked = true;
    this.currentFrame = 0;
    setTimeout(() => {
      this.isClicked = false;
      this.currentFrame = 2;
    }, 100);

    this.clickX = x;
    this.clickY = y;

    callback(this);
  }

  attachShip(ship) {
    this.shipAttached = ship;
    this.initialOffsetX = this.x - ship.x;
    this.initialOffsetY = this.y - ship.y;
  }

  dropShipOnGrid() {
    playerBoard.placeShip(this.shipAttached);
    this.shipAttached = null;
  }

  getRowCell() {
    return Math.floor(this.x / UNIT_SIZE);
  }

  getColumnCell() {
    return Math.floor(this.y / UNIT_SIZE);
  }

  rotateShip() {
    if (this.shipAttached != null) {
      this.initialOffsetX = BOARD_UNIT_SIZE / 2;
      this.initialOffsetY = BOARD_UNIT_SIZE / 2;
      this.shipAttached.rotate();
    }
  }
}
