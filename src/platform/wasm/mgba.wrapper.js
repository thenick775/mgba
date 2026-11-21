import createMGBA from './mgba.js';

const wasmUrl = new URL('./mgba.wasm', import.meta.url).href;
const dataUrl = new URL('./mgba.data', import.meta.url).href;

export default (options = {}) =>
  createMGBA({
    locateFile: (path, prefix) => {
      if (path === 'mgba.wasm') return wasmUrl;
      if (path === 'mgba.data') return dataUrl;
      return prefix + path;
    },
    ...options,
  });
