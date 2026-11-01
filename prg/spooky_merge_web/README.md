# Spooky Merge browser build

The game and its physics are written in Seed7. `launcher.js` only loads the art, starts the generated program, passes the saved best score and display size as startup arguments, and stores the program's `SPOOKY_BEST=<score>` output in browser `localStorage`. It uses the optional `seed7Print` and `seed7PreRun` hooks in `src/pre_js.js`.

The browser game draws at 720x1280 and displays at 360x640 CSS pixels, so it stays sharp on high-density screens. The native build has no art: it draws simple shapes at 360x640.

## Build

Use Linux, GCC, Make, Node.js, and the Emscripten SDK. Install and activate emsdk using its official instructions, then from the repository root:

```sh
source /path/to/emsdk/emsdk_env.sh
cd src
make -f mk_emccl.mak clean
make -f mk_emccl.mak depend
make -f mk_emccl.mak
make -f mk_emccl.mak s7c
cd ..
# Replace /absolute/path/to/seed7 with this repository's absolute path.
node --stack-size=8192 bin/s7c.js -l /absolute/path/to/seed7/lib -b /absolute/path/to/seed7/bin -c emcc -O2 prg/spooky_merge.sd7
cp prg/spooky_merge.js prg/spooky_merge_web/
cp prg/spooky_merge.wasm prg/spooky_merge_web/
```

The clean step clears stale generated headers so `depend` recreates the compiler environment file. This sequence and the game compile were verified in Ubuntu 24.04 under WSL with emcc 6.0.13, Node 18.19.1, GCC 13.3.0, and Make 4.3. The generated Seed7 compiler reports version 3.5.18. Pass absolute library and runtime paths to `-l` and `-b`; relative paths failed to locate the Emscripten libraries when building from the Windows-mounted worktree.

## Run locally

From the repository root, serve the directory over HTTP (opening `index.html` directly does not load WASM reliably):

```sh
python3 -m http.server 8000 --directory prg/spooky_merge_web
```

Open `http://localhost:8000/` and press **Play**. The browser needs JavaScript and WebAssembly enabled. The game runs in the page, not in a popup: `launcher.js` sets `seed7InPage`, because a popup can hide the page that runs the program, and browsers throttle the timers of hidden pages. Phones scale the game to the screen width through the `width=360` viewport setting in `index.html`.

## Native build and rules checks

From the repository root, after building the native Seed7 compiler:

```sh
bin/s7c -l lib -b bin -O2 prg/spooky_merge.sd7
bin/s7c -l lib -b bin -O2 prg/spooky_merge_test.sd7
prg/spooky_merge_test
```

On Windows, use `bin\s7c.exe` and run `prg\spooky_merge_test.exe`.

## Browser smoke test

After copying the build into `prg/spooky_merge_web/`, install Playwright and its Chromium, then run the test. It serves the folder itself and needs no other server:

```sh
cd prg/spooky_merge_web
npm install --no-save playwright
npx playwright install chromium
node smoke_test.cjs
```

It reads the game canvas to check loading, keyboard and mouse aim, varied spawns, touch drag, fit on 412 and 320 px wide phones, and best-score persistence. Set `SMOKE_HEADED=1` to watch it, `SMOKE_CHANNEL=chrome` to use installed Chrome, or `SMOKE_SCREENSHOT_DIR=<folder>` to save phone screenshots.

## Static hosting

Build the Seed7 program, then publish the contents of `prg/spooky_merge_web/`, including `art/*.png` and the generated `spooky_merge.js` and `spooky_merge.wasm`. Keep both generated files beside `index.html`; the game files are intentionally ignored by Git. The host must serve `.wasm` with an `application/wasm` content type for streaming instantiation, although Emscripten may fall back to array-buffer loading.

## Art

Each tier and the background has an SVG source and a rendered PNG in `art/`. After changing an SVG, run `node art/render.cjs` from this folder, with Playwright installed as described above.

To use other artwork, replace a PNG and keep its file name. Tier images are square with a transparent background, and the game scales them to the size of the tier. For the sharpest result use the sizes in `render.cjs`. The top 110 CSS pixels of `background.png` (220 image pixels) must stay the flat color `#0b0820`, because the score text paints its own background. If a PNG cannot be loaded, the game draws a simple shape instead.

## Controls

- Move with the mouse or A/D and arrow keys.
- Drag a finger to aim on touch screens.
- Drop with Space, a mouse click, or a touch release.
- Press R or click after game over to restart.

## Verification record

Last verified 2026-11-01. The rules program passed all checks, both natively compiled on Windows and under the interpreter. The game compiled to JS/WASM with Seed7 compiler 3.5.18 and emcc 6.0.13. `smoke_test.cjs` passed on that build in headless Playwright 1.64.1 Chromium: desktop, a short window, missing art, and touch emulation at 412x915 and 320x640. Tier names and game text are present in the WASM and absent from the JS.

Playwright disables background throttling and emulates page focus, so `smoke_test.cjs` cannot detect throttled timers. The in-page mode was checked separately in installed Chrome through the DevTools protocol, with the page visible, hidden in a background tab, and visible again.

Removing the WASM (HTTP 404, no game canvas) was checked on an earlier build and not repeated.

The target is a desktop browser. Touch works in emulation, but phones are not a target and no physical device was tested. A human return to a hidden tab remains unverified. In headed Chrome at 1280x800 a nearly full field ran at about 44 frames per second.
