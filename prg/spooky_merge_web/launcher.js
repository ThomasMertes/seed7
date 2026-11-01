"use strict";

const button = document.getElementById("start");
const status = document.getElementById("status");

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

button.addEventListener("click", () => {
  let best = "0";
  try {
    best = localStorage.getItem("spooky-merge-best") || "0";
  } catch (_) {
    status.textContent = "Best score storage is unavailable in this browser.";
  }

  button.disabled = true;
  button.textContent = "Loading...";
  status.textContent = "Starting the Seed7 game…";

  const program = document.createElement("script");
  program.src = `spooky_merge.js?${encodeURIComponent(best)}`;
  program.onerror = () => {
    button.disabled = false;
    button.textContent = "Try again";
    status.textContent = "Game files are missing. Build the WASM demo first.";
  };
  document.body.appendChild(program);
});
