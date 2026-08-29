# mGBA-wasm

This package is a bundled version of my [mGBA fork](https://github.com/thenick775/mgba/tree/feature/wasm) compiled to webassembly.

This core is framework agnostic, and can be instantiated in any framework, including vanilla javascript.

See the [api documentation](https://thenick775.github.io/mgba/) for in depth details.

This core currently powers [gbajs3](https://gba.nicholas-vancise.dev)!

## Install

```bash
npm i @thenick775/mgba-wasm
```

## React

To instantiate the emulator using react:

```
import mGBA, { type mGBAEmulator } from '@thenick775/mgba-wasm';
import { useEffect, useState } from 'react';

export const useEmulator = (canvas: HTMLCanvasElement | null) => {
  const [emulator, setEmulator] = useState<mGBAEmulator | null>(null);

  useEffect(() => {
    const initialize = async () => {
      if (canvas) {
        const Module = await mGBA({ canvas });

        const mGBAVersion =
          Module.version.projectName + ' ' + Module.version.projectVersion;
        console.log(mGBAVersion);

        await Module.FSInit();

        setEmulator(Module);
      }
    };

    initialize();
  }, [canvas]);

  return emulator;
};
```

## Vanilla

In a vanilla javascript project with no bundler, first download the published package tarball:

```bash
npm pack @thenick775/mgba-wasm
```

Extract these files from `package/dist/` and serve them from the same origin as your page:

- `mgba.js`
- `mgba.wasm`

Then instantiate the emulator as follows:

```html
<canvas id="canvas" width="240" height="160"></canvas>

<script type="module">
  import mGBA from "./mgba.js";

  const canvas = document.getElementById("canvas");
  const Module = await mGBA({ canvas });
  await Module.FSInit();

  console.log(
    `version ${Module.version.projectName + " " + Module.version.projectVersion}`,
  );
</script>
```

## Cross-origin Isolation

This core uses threads, so the page serving these files must enable cross-origin isolation:

```
Cross-Origin-Opener-Policy: same-origin
Cross-Origin-Embedder-Policy: require-corp
```

`mgba.js` must be same-origin with the page because the threaded runtime starts its pthread worker from that script URL.

## API

The core exposes the following contract in addition to some standard Emscripten `Module` utilities such as `Module.FS`. This is a stateful emulator runtime designed to be created once, retained by the host application, and driven through the imperative API below.

Lifecycle:

- `FSInit()`
- `FSSync()`
- `pauseGame()`
- `resumeGame()`
- `pauseAudio()`
- `resumeAudio()`
- `quickReload()`
- `quitGame()`
- `quitMgba()`

ROM and file management:

- `loadGame(romPath, savePathOverride)`
- `listRoms()`
- `listSaves()`
- `filePaths()`
- `uploadRom(file, callback)`
- `uploadSaveOrSaveState(file, callback)`
- `uploadCheats(file, callback)`
- `uploadPatch(file, callback)`
- `uploadScreenshot(file, callback)`
- `uploadAutoSaveState(autoSaveStateName, data)`

Input management:

- `bindKey(bindingName, inputName)`
- `buttonPress(name)`
- `buttonUnpress(name)`
- `toggleInput(enabled)`

Save data and save states:

- `getSave()`
- `saveState(slot)`
- `loadState(slot)`
- `saveStateSlot(slot, flags)`
- `loadStateSlot(slot, flags)`
- `forceAutoSaveState()`
- `loadAutoSaveState()`
- `getAutoSaveState()`

Audio, timing, and speed:

- `getVolume()`
- `setVolume(percent)`
- `getMainLoopTimingMode()`
- `getMainLoopTimingValue()`
- `setMainLoopTiming(mode, value)`
- `getFastForwardMultiplier()`
- `setFastForwardMultiplier(multiplier)`
- `screenshot(fileName)`

Callbacks and core settings:

- `addCoreCallbacks(coreCallbacks)`
- `autoLoadCheats()`
- `setLogger(callback)`
- `setCoreSettings(coreSettings)`
- `toggleRewind(toggle)`

See `dist/mgba.d.ts` for full signatures and inline documentation.

## Source

See the repo [README](https://github.com/thenick775/mgba/tree/feature/wasm#readme) for further details such as:

- building from source
- running the bundled demo
- repository and implementation details
