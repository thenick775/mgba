# mGBA-wasm

This package is a bundled version of my [mGBA fork](https://github.com/thenick775/mgba/tree/feature/wasm) compiled to webassembly.

This core is framework agnostic, and can be instantiated in any framework, including vanilla javascript.

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

## Vanilla Browser

For a vanilla javascript project with no bundler, first download the published package tarball:

```bash
npm pack @thenick775/mgba-wasm
```

Extract these files from `package/dist/` and serve them from the same origin as your page:

- `mgba.js`
- `mgba.wasm`

Then instantiate the emulator like this:

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

## Serving Requirements

This core uses threads, so the page serving these files must enable cross-origin isolation:

```
Cross-Origin-Opener-Policy: same-origin
Cross-Origin-Embedder-Policy: require-corp
```

`mgba.js` must be same-origin with the page because the threaded runtime starts its pthread worker from that script URL.

See the feature/wasm [README](https://github.com/thenick775/mgba/tree/feature/wasm#readme) for further details such as:

- available emulator interface methods
- building from source
- embedding and usage in vanilla javascript
