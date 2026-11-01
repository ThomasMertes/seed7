"use strict";

const button = document.getElementById("start");
const status = document.getElementById("status");
const GAME_HEIGHT = 640;
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

// Loads a PNG as a size header followed by RGBA bytes for the Seed7 program.
// A missing image is skipped: the game then draws simple shapes.
async function loadArt(name) {
  const image = new Image();
  image.src = `art/${name}.png`;
  await image.decode();
  const canvas = document.createElement("canvas");
  canvas.width = image.naturalWidth;
  canvas.height = image.naturalHeight;
  const context = canvas.getContext("2d", { willReadFrequently: true });
  context.drawImage(image, 0, 0);
  const pixels = context.getImageData(0, 0, canvas.width, canvas.height).data;
  const bytes = new Uint8Array(pixels.length + 4);
  bytes.set([canvas.width >> 8, canvas.width & 255, canvas.height >> 8, canvas.height & 255]);
  bytes.set(pixels, 4);
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
