import createMGBA from './mgba.js';

import wasmUrl from './mgba.wasm?url';
import dataUrl from './mgba.data?url';

export default (options = {}) =>
  createMGBA({
    locateFile: (path, prefix) => {
      if (path === 'mgba.wasm') return wasmUrl;
      if (path === 'mgba.data') return dataUrl;
      return prefix + path;
    },
    ...options,
  });
