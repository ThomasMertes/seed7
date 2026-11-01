"use strict";
// Renders the SVG art to the PNG files used by the game. See ../README.md.

const fs = require("fs");
const path = require("path");
const { chromium } = require("playwright");

// The sprite sizes are twice the diameter of the tiers in spooky_merge_core.s7i.
const ART = [
  ["candy_corn", 56, 56], ["eyeball", 72, 72], ["spider", 92, 92], ["bat", 116, 116],
  ["ghost", 144, 144], ["skull", 176, 176], ["potion", 212, 212], ["cauldron", 252, 252],
  ["jack_o_lantern", 296, 296], ["pumpkin_king", 344, 344], ["background", 720, 1280],
];

async function main() {
  const browser = await chromium.launch();
  const page = await browser.newPage();
  for (const [name, width, height] of ART) {
    const svg = fs.readFileSync(path.join(__dirname, `${name}.svg`), "utf8")
      .replace(/width="\d+" height="\d+" viewBox/, `width="${width}" height="${height}" viewBox`);
    await page.setViewportSize({ width, height });
    await page.setContent(`<!doctype html><style>html,body{margin:0;background:transparent}svg{display:block}</style>${svg}`);
    await page.screenshot({ path: path.join(__dirname, `${name}.png`), omitBackground: true });
  }
  await browser.close();
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
