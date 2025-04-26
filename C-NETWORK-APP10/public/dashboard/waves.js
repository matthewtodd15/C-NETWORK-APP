function drawWaves() {
  for (let i = 0; i < waves.length; i++) {
    const { x, y, img } = waves[i];
    CTX.drawImage(img, x, y);
  }
}

function updateWaves() {
  // move waves from top left to bottom right
  for (let i = 0; i < waves.length; i++) {
    waves[i].x += WAVE_SPEED * getDeltaTime();
    waves[i].y += WAVE_SPEED * getDeltaTime();

    // if out of bounds, remove
    if (waves[i].x > canvas.width && waves[i].y > canvas.height) {
      let temp = waves[i];
      waves[i] = waves[waves.length - 1];
      waves[waves.length - 1] = temp;

      waves.pop();
    }
  }
}

const waveIntervalId = setInterval(
  () => {
    let img = wavesImgs[Math.ceil(Math.random() * 100) % wavesImgs.length];
    waves.push({
      x: 0 - Math.random() * 500 - img.width,
      y: 0 - Math.random() * 500 - img.height,
      img: img,
    });
  },
  Math.max(Math.random() * 4000, 2000),
);
