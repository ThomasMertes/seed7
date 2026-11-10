"use strict";

const button = document.getElementById("start");
const status = document.getElementById("status");
const GAME_HEIGHT = 640;
const ROTATION_FRAMES = 36;
const ART = ["background", "candy_corn", "eyeball", "spider", "bat", "ghost", "skull",
  "potion", "cauldron", "jack_o_lantern", "pumpkin_king"];

// Keep the game in this page. A popup would hide this page, and the browser
// would then throttle the timers of the game.
window.seed7InPage = true;

// src/pre_js.js passes standard output of the Seed7 program to this hook.
window.seed7Print = (text) => {
  const match = /^SPOOKY_BEST=(\d+)$/.exec(String(text));
  if (match === null) {
    console.log(text);
    return;
  }
  try {
    localStorage.setItem("spooky-merge-best", match[1]);
  } catch (_) {
    console.warn("Spooky Merge could not save the best score.");
  }
};

// Loads a PNG for the Seed7 program as a header with the width, height and
// number of frames, followed by the RGBA bytes of the rotated frames.
// A missing image is skipped: the game then draws simple shapes.
async function loadArt(name) {
  const image = new Image();
  image.src = `art/${name}.png`;
  await image.decode();
  const width = image.naturalWidth;
  const height = image.naturalHeight;
  const frames = name === "background" ? 1 : ROTATION_FRAMES;
  const canvas = document.createElement("canvas");
  canvas.width = width;
  canvas.height = height;
  const context = canvas.getContext("2d", { willReadFrequently: true });
  context.imageSmoothingQuality = "high";
  const bytes = new Uint8Array(6 + frames * width * height * 4);
  bytes.set([width >> 8, width & 255, height >> 8, height & 255, frames >> 8, frames & 255]);
  for (let frame = 0; frame < frames; frame += 1) {
    context.clearRect(0, 0, width, height);
    context.setTransform(1, 0, 0, 1, width / 2, height / 2);
    context.rotate(2 * Math.PI * frame / frames);
    context.drawImage(image, -width / 2, -height / 2);
    context.resetTransform();
    bytes.set(context.getImageData(0, 0, width, height).data, 6 + frame * width * height * 4);
  }
  return [name, bytes];
}

button.addEventListener("click", async () => {
  let best = "0";
  try {
    best = localStorage.getItem("spooky-merge-best") || "0";
  } catch (_) {
    status.textContent = "Best score storage is unavailable in this browser.";
  }

  button.disabled = true;
  button.textContent = "Loading...";
  status.textContent = "Starting the Seed7 game…";

  const results = await Promise.allSettled(ART.map(loadArt));
  window.seed7PreRun = () => {
    for (const result of results) {
      if (result.status === "fulfilled") {
        FS.writeFile(`/${result.value[0]}.rgba`, result.value[1]);
      }
    }
  };

  // The game keeps logical pointer coordinates, so it needs the display size.
  const percent = Math.min(100, Math.floor(window.innerHeight * 100 / GAME_HEIGHT));
  document.documentElement.style.setProperty("--game-scale", percent / 100);

  const program = document.createElement("script");
  program.src = `spooky_merge.js?${encodeURIComponent(best)}&${percent}`;
  program.onerror = () => {
    button.disabled = false;
    button.textContent = "Try again";
    status.textContent = "Game files are missing. Build the WASM demo first.";
  };
  document.body.appendChild(program);
});
