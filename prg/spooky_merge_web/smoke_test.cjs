"use strict";
// Browser smoke test for the built game. See README.md for how to run it.

const fs = require("fs");
const http = require("http");
const path = require("path");
const { chromium } = require("playwright");

const ROOT = __dirname;
const MIME = {
  ".html": "text/html",
  ".js": "text/javascript",
  ".css": "text/css",
  ".wasm": "application/wasm",
};
const GUIDE_ROW = 136;
const PREVIEW = { x: 90, y: 65 };
const SPAWN_COLORS = {
  "255,191,35": 1,
  "255,46,140": 2,
  "105,255,50": 3,
  "164,70,255": 4,
};

let failures = 0;

function check(condition, description) {
  console.log(`${condition ? "ok  " : "FAIL"} ${description}`);
  if (!condition) {
    failures += 1;
  }
}

function serve() {
  const server = http.createServer((request, response) => {
    const name = request.url.split("?")[0].replace(/^\/$/, "/index.html");
    const file = path.join(ROOT, path.normalize(name));
    fs.readFile(file, (error, data) => {
      if (error) {
        response.writeHead(404);
        response.end();
      } else {
        response.writeHead(200, { "Content-Type": MIME[path.extname(file)] || "text/plain" });
        response.end(data);
      }
    });
  });
  return new Promise((resolve) => {
    server.listen(0, "127.0.0.1", () => resolve(server));
  });
}

const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

// The driver renders in a popup or in the page, depending on the browser.
async function findGamePage(context) {
  for (let attempt = 0; attempt < 100; attempt += 1) {
    for (const page of context.pages()) {
      const found = await page
        .evaluate(() => [...document.querySelectorAll("canvas")].some((c) => c.width === 360))
        .catch(() => false);
      if (found) {
        return page;
      }
    }
    await sleep(100);
  }
  throw new Error("The game canvas did not appear.");
}

function pixel(page, x, y) {
  return page.evaluate(([px, py]) => {
    const canvas = [...document.querySelectorAll("canvas")].filter((c) => c.width === 360).pop();
    return Array.from(canvas.getContext("2d").getImageData(px, py, 1, 1).data).slice(0, 3);
  }, [x, y]);
}

// The aim guide is the brightest column in this row of the empty upper field.
// Canvas lines are anti-aliased, so the guide is dimmer than its nominal color.
function guideX(page) {
  return page.evaluate((row) => {
    const canvas = [...document.querySelectorAll("canvas")].filter((c) => c.width === 360).pop();
    const data = canvas.getContext("2d").getImageData(0, row, 360, 1).data;
    let brightest = -1;
    let level = 120;
    for (let x = 24; x < 336; x += 1) {
      const sum = data[x * 4] + data[x * 4 + 1] + data[x * 4 + 2];
      if (sum > level) {
        level = sum;
        brightest = x;
      }
    }
    return brightest;
  }, GUIDE_ROW);
}

async function previewTier(page) {
  return SPAWN_COLORS[(await pixel(page, PREVIEW.x, PREVIEW.y)).join(",")];
}

async function startGame(context, launcher, url) {
  await launcher.goto(url);
  await launcher.click("#start");
  const page = await findGamePage(context);
  await sleep(500);
  return page;
}

async function waitForBest(launcher, drop) {
  let drops = 0;
  let best = null;
  while (best === null && drops < 40) {
    await drop();
    drops += 1;
    await sleep(600);
    best = await launcher.evaluate(() => localStorage.getItem("spooky-merge-best"));
  }
  return best;
}

async function desktop(browser, url) {
  console.log("Desktop 1280x800");
  const context = await browser.newContext({ viewport: { width: 1280, height: 800 } });
  const launcher = await context.newPage();
  const errors = [];
  launcher.on("pageerror", (error) => errors.push(error.message));
  let wasmStatus = 0;
  context.on("response", (response) => {
    if (response.url().endsWith(".wasm")) {
      wasmStatus = response.status();
    }
  });

  const page = await startGame(context, launcher, url);
  check(wasmStatus === 200, "the WASM file loads over HTTP");
  check(page === launcher && context.pages().length === 1, "the game runs in the page, not a popup");
  const left = await page.evaluate(() => {
    const canvas = [...document.querySelectorAll("canvas")].filter((c) => c.width === 360).pop();
    return canvas.getBoundingClientRect().left;
  });
  check(Math.abs(left - 460) <= 1, `the field is centered (left=${left})`);
  await page.keyboard.press("Space");
  await sleep(300);
  check(Math.abs((await guideX(page)) - 180) <= 2, "the aim starts centered without a mouse");

  for (let press = 0; press < 5; press += 1) {
    await page.keyboard.press("d");
  }
  await sleep(100);
  const aimed = await guideX(page);
  await sleep(500);
  const held = await guideX(page);
  check(Math.abs(aimed - 240) <= 2, `keyboard aim moves the guide (x=${aimed})`);
  check(Math.abs(held - 240) <= 2, `keyboard aim holds without a mouse (x=${held})`);

  const seen = new Set();
  for (let drop = 0; drop < 14; drop += 1) {
    seen.add(await previewTier(page));
    await page.keyboard.press("a");
    await page.keyboard.press("Space");
    await sleep(420);
  }
  seen.delete(undefined);
  check(seen.size >= 3, `spawns vary (tiers ${[...seen].sort().join(", ")})`);

  await page.mouse.move(left + 100, 300);
  await sleep(200);
  check(Math.abs((await guideX(page)) - 100) <= 16, "the mouse moves the aim");

  const best = await waitForBest(launcher, () => page.mouse.click(left + 100, 300));
  check(best !== null && Number(best) > 0, `a merge saves the best score (${best})`);
  check(errors.length === 0, `no page errors ${errors.join("; ")}`);
  await context.close();
}

async function phone(browser, url, width, height) {
  console.log(`Phone ${width}x${height}`);
  const context = await browser.newContext({
    viewport: { width, height },
    hasTouch: true,
    isMobile: true,
  });
  const launcher = await context.newPage();
  await startGame(context, launcher, url);
  const page = await findGamePage(context);
  const client = await context.newCDPSession(page);
  const fit = await page.evaluate(() => {
    const canvas = [...document.querySelectorAll("canvas")].filter((c) => c.width === 360).pop();
    const box = canvas.getBoundingClientRect();
    return { right: box.right, layout: document.documentElement.clientWidth,
      scale: window.visualViewport.scale, visible: window.visualViewport.width };
  });
  check(fit.right <= fit.visible + 0.5, `the field fits the screen (${JSON.stringify(fit)})`);

  const touch = async (type, x, y) => {
    await client.send("Input.dispatchTouchEvent", {
      type,
      touchPoints: type === "touchEnd" ? [] : [{ x, y }],
    });
  };
  await touch("touchStart", 180, 400);
  await touch("touchEnd");
  await sleep(300);

  await touch("touchStart", 100, 400);
  await touch("touchMove", 260, 400);
  await sleep(200);
  const dragged = await guideX(page);
  check(Math.abs(dragged - 260) <= 16, `touch drag aims in field coordinates (x=${dragged})`);
  await touch("touchEnd");
  await sleep(300);
  if (process.env.SMOKE_SCREENSHOT_DIR !== undefined) {
    await page.screenshot({ path: path.join(process.env.SMOKE_SCREENSHOT_DIR, `phone-${width}.png`) });
  }

  const best = await waitForBest(launcher, async () => {
    await touch("touchStart", 180, 400);
    await touch("touchEnd");
  });
  check(best !== null && Number(best) > 0, `touch play saves the best score (${best})`);

  await launcher.reload();
  const saved = await launcher.evaluate(() => localStorage.getItem("spooky-merge-best"));
  check(saved === best, "the best score survives a reload");
  await context.close();
}

async function main() {
  const server = await serve();
  const url = `http://127.0.0.1:${server.address().port}/`;
  const browser = await chromium.launch({
    channel: process.env.SMOKE_CHANNEL || undefined,
    headless: process.env.SMOKE_HEADED === undefined,
  });
  try {
    await desktop(browser, url);
    await phone(browser, url, 412, 915);
    await phone(browser, url, 320, 640);
  } finally {
    await browser.close();
    server.close();
  }
  console.log(failures === 0 ? "Spooky Merge smoke test passed." : `${failures} check(s) failed.`);
  process.exitCode = failures === 0 ? 0 : 1;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
