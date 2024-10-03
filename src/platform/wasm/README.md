# mGBA-wasm

This package is a bundled version of my [mGBA fork](https://github.com/thenick775/mgba/tree/feature/wasm) compiled to webassembly.

This core currently powers [gbajs3](https://gba.nicholas-vancise.dev)!

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

See the feature/wasm [README](https://github.com/thenick775/mgba/tree/feature/wasm#readme) for further details such as:

- available emulator interface methods
- building from source
- embedding and usage in vanilla javascript
