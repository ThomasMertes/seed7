"use strict";
// Renders the SVG art to the PNG files used by the game. See ../README.md.

const fs = require("fs");
const path = require("path");
const { chromium } = require("playwright");

// The physics treats every piece as a circle. Each tier image is centered on the
// center of mass of its art and scaled, so that the art fills the circle. Its tips
// may reach past the circle, up to ART_OVERSIZE (ART_PERCENT in spooky_merge.sd7).
const ART_OVERSIZE = 1.25;
const SOURCE_SIZE = 1024;
const OPAQUE_ALPHA = 128;

// Twice the tier radii of spooky_merge_core.s7i, in image pixels per radius unit.
const TIERS = [
  ["candy_corn", 14], ["eyeball", 18], ["spider", 23], ["bat", 29], ["ghost", 36],
  ["skull", 44], ["potion", 53], ["cauldron", 63], ["jack_o_lantern", 74], ["pumpkin_king", 86],
];

function readSvg(name) {
  return fs.readFileSync(path.join(__dirname, `${name}.svg`), "utf8");
}

async function screenshot(page, svg, width, height, target) {
  await page.setViewportSize({ width, height });
  await page.setContent(`<!doctype html><style>html,body{margin:0;background:transparent}svg{display:block}</style>${svg}`);
  return page.screenshot({ path: target, omitBackground: true });
}

// Finds the center of mass, and a radius between the circle of equal area, which
// is exact for round art, and the farthest opaque pixel, which keeps thin tips in.
async function measure(page, png) {
  return page.evaluate(async ({ data, opaque }) => {
    const image = new Image();
    image.src = `data:image/png;base64,${data}`;
    await image.decode();
    const canvas = document.createElement("canvas");
    canvas.width = image.naturalWidth;
    canvas.height = image.naturalHeight;
    const context = canvas.getContext("2d", { willReadFrequently: true });
    context.drawImage(image, 0, 0);
    const pixels = context.getImageData(0, 0, canvas.width, canvas.height).data;
    let area = 0;
    let sumX = 0;
    let sumY = 0;
    for (let y = 0; y < canvas.height; y += 1) {
      for (let x = 0; x < canvas.width; x += 1) {
        if (pixels[(y * canvas.width + x) * 4 + 3] >= opaque) {
          area += 1;
          sumX += x;
          sumY += y;
        }
      }
    }
    const cx = sumX / area;
    const cy = sumY / area;
    let farthest = 0;
    for (let y = 0; y < canvas.height; y += 1) {
      for (let x = 0; x < canvas.width; x += 1) {
        if (pixels[(y * canvas.width + x) * 4 + 3] >= opaque) {
          farthest = Math.max(farthest, Math.hypot(x - cx, y - cy));
        }
      }
    }
    return { cx, cy, farthest, equalArea: Math.sqrt(area / Math.PI) };
  }, { data: png.toString("base64"), opaque: OPAQUE_ALPHA });
}

async function main() {
  const browser = await chromium.launch();
  try {
    await renderAll(await browser.newPage());
  } finally {
    await browser.close();
  }
}

async function renderAll(page) {
  for (const [name, radius] of TIERS) {
    const svg = readSvg(name);
    const png = await screenshot(page, svg, SOURCE_SIZE, SOURCE_SIZE);
    const art = await measure(page, png);
    const fitted = Math.max((art.equalArea + art.farthest) / 2, art.farthest / ART_OVERSIZE);
    const half = fitted * ART_OVERSIZE;
    const size = Math.round(4 * radius * ART_OVERSIZE);
    const viewBox = `${art.cx - half} ${art.cy - half} ${2 * half} ${2 * half}`;
    await screenshot(page,
      svg.replace(/width="\d+" height="\d+" viewBox="[^"]*"/,
        `width="${size}" height="${size}" viewBox="${viewBox}"`),
      size, size, path.join(__dirname, `${name}.png`));
    console.log(`${name}: filled ${(fitted / (SOURCE_SIZE / 2) * 100).toFixed(0)}% of the old circle, ` +
      `tips reach ${(art.farthest / fitted * 100).toFixed(0)}%, ${size}px`);
  }
  await screenshot(page, readSvg("background"), 720, 1280, path.join(__dirname, "background.png"));
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
