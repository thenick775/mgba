/* Copyright (c) 2013-2023 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

Module.loadGame = (romPath, savePathOverride) => {
  const loadGame = cwrap('loadGame', 'number', ['string', 'string']);

  if (loadGame(romPath, savePathOverride)) {
    const arr = romPath.split('.');
    arr.pop();

    const saveName = arr.join('.') + '.sav';
    const autoSaveStateName = arr.join('.') + '_auto.ss';

    Module.gameName = romPath;
    Module.saveName =
      savePathOverride ?? saveName.replace('/data/games/', '/data/saves/');
    Module.autoSaveStateName = autoSaveStateName.replace(
      '/data/games/',
      '/autosave/'
    );
    return true;
  }

  return false;
};

Module.getSave = () => {
  const exists = FS.analyzePath(Module.saveName).exists;

  return exists ? FS.readFile(Module.saveName) : null;
};

Module.listRoms = () => {
  return FS.readdir('/data/games/');
};

Module.listSaves = () => {
  return FS.readdir('/data/saves/');
};

Module.FSInit = () => {
  return new Promise((resolve, reject) => {
    FS.mkdir('/data');
    FS.mount(FS.filesystems.IDBFS, {}, '/data');

    // mount auto save directory, this should auto persist, while the data mount should not
    FS.mkdir('/autosave');
    FS.mount(FS.filesystems.IDBFS, { autoPersist: true }, '/autosave');

    // load data from IDBFS
    FS.syncfs(true, (err) => {
      if (err) {
        reject(new Error(`Error syncing app data from IndexedDB: ${err}`));
      }

      // When we read from indexedb, these directories may or may not exist.
      // If we mkdir and they already exist they throw, so just catch all of them.
      try {
        FS.mkdir('/data/saves');
      } catch (e) {}
      try {
        FS.mkdir('/data/states');
      } catch (e) {}
      try {
        FS.mkdir('/data/games');
      } catch (e) {}
      try {
        FS.mkdir('/data/cheats');
      } catch (e) {}
      try {
        FS.mkdir('/data/screenshots');
      } catch (e) {}
      try {
        FS.mkdir('/data/patches');
      } catch (e) {}

      resolve();
    });
  });
};

Module.FSSync = () => {
  return new Promise((resolve, reject) => {
    // write data to IDBFS
    FS.syncfs((err) => {
      if (err) {
        reject(new Error(`Error syncing app data to IndexedDB: ${err}`));
      }

      resolve();
    });
  });
};

Module.filePaths = () => {
  return {
    root: '/data',
    cheatsPath: '/data/cheats',
    gamePath: '/data/games',
    savePath: '/data/saves',
    saveStatePath: '/data/states',
    screenshotsPath: '/data/screenshots',
    patchPath: '/data/patches',
    autosave: '/autosave',
  };
};

Module.uploadSaveOrSaveState = (file, callback) => {
  const split = file.name.split('.');
  if (split.length < 2) {
    console.warn('unrecognized file extension: ' + file.name);
    return;
  }
  const extension = split[split.length - 1].toLowerCase();

  let dir = null;
  if (extension == 'sav') {
    dir = '/data/saves/';
  } else if (extension.startsWith('ss')) {
    dir = '/data/states/';
  } else {
    console.warn('unrecognized file extension: ' + extension);
    return;
  }

  const reader = new FileReader();
  reader.onload = (e) => {
    FS.writeFile(dir + file.name, new Uint8Array(e.target.result));
    if (callback) {
      callback();
    }
  };

  reader.readAsArrayBuffer(file);
};

Module.uploadRom = (file, callback) => {
  const split = file.name.split('.');
  if (split.length < 2) {
    console.warn('unrecognized file extension: ' + file.name);
    return;
  }
  const extension = split[split.length - 1].toLowerCase();

  let dir = null;
  if (['gba', 'gbc', 'gb', 'zip', '7z'].includes(extension)) {
    dir = '/data/games/';
  } else {
    console.warn('unrecognized file extension: ' + extension);
    return;
  }

  const reader = new FileReader();
  reader.onload = (e) => {
    FS.writeFile(dir + file.name, new Uint8Array(e.target.result));
    if (callback) {
      callback();
    }
  };

  reader.readAsArrayBuffer(file);
};

Module.uploadCheats = (file, callback) => {
  const split = file.name.split('.');
  if (split.length < 2) {
    console.warn('unrecognized file extension: ' + file.name);
    return;
  }
  const extension = split[split.length - 1].toLowerCase();

  let dir = null;
  if (extension == 'cheats') {
    dir = '/data/cheats/';
  } else {
    console.warn('unrecognized file extension: ' + extension);
    return;
  }

  const reader = new FileReader();
  reader.onload = (e) => {
    FS.writeFile(dir + file.name, new Uint8Array(e.target.result));
    if (callback) {
      callback();
    }
  };

  reader.readAsArrayBuffer(file);
};

Module.uploadScreenshot = (file, callback) => {
  const split = file.name.split('.');
  if (split.length < 2) {
    console.warn('unrecognized file extension: ' + file.name);
    return;
  }
  const extension = split[split.length - 1].toLowerCase();

  let dir = null;
  if (extension == 'png') {
    dir = '/data/screenshots/';
  } else {
    console.warn('unrecognized file extension: ' + extension);
    return;
  }

  const reader = new FileReader();
  reader.onload = (e) => {
    FS.writeFile(dir + file.name, new Uint8Array(e.target.result));
    if (callback) {
      callback();
    }
  };

  reader.readAsArrayBuffer(file);
};

Module.uploadPatch = (file, callback) => {
  const split = file.name.split('.');
  if (split.length < 2) {
    console.warn('unrecognized file extension: ' + file.name);
    return;
  }
  const extension = split[split.length - 1].toLowerCase();

  let dir = null;
  if (['ips', 'ups', 'bps'].includes(extension)) {
    dir = '/data/patches/';
  } else {
    console.warn('unrecognized file extension: ' + extension);
    return;
  }

  const reader = new FileReader();
  reader.onload = (e) => {
    FS.writeFile(dir + file.name, new Uint8Array(e.target.result));
    if (callback) {
      callback();
    }
  };

  reader.readAsArrayBuffer(file);
};

const keyBindings = new Map([
  ['a', 0],
  ['b', 1],
  ['select', 2],
  ['start', 3],
  ['right', 4],
  ['left', 5],
  ['up', 6],
  ['down', 7],
  ['r', 8],
  ['l', 9],
]);

Module.buttonPress = (name) => {
  const buttonPress = cwrap('buttonPress', null, ['number']);
  buttonPress(keyBindings.get(name.toLowerCase()));
};

Module.buttonUnpress = (name) => {
  const buttonUnpress = cwrap('buttonUnpress', null, ['number']);
  buttonUnpress(keyBindings.get(name.toLowerCase()));
};

// bindingName is the key name you want to associate to an input, ex. 'p' key binding -> 'a' input
// inputName is the name of the input to bind to, ex 'a', 'b', 'up' etc.
Module.bindKey = (bindingName, inputName) => {
  const bindKey = cwrap('bindKey', null, ['string', 'number']);
  bindKey(bindingName, keyBindings.get(inputName.toLowerCase()));
};

Module.pauseGame = () => {
  const pauseGame = cwrap('pauseGame', null, []);
  pauseGame();
};

Module.resumeGame = () => {
  const resumeGame = cwrap('resumeGame', null, []);
  resumeGame();
};

Module.pauseAudio = () => {
  const pauseAudio = cwrap('pauseAudio', null, []);
  pauseAudio();
};

Module.resumeAudio = () => {
  const resumeAudio = cwrap('resumeAudio', null, []);
  resumeAudio();
};

Module.getVolume = () => {
  const getVolume = cwrap('getVolume', 'number', []);
  return getVolume();
};

Module.setVolume = (percent) => {
  const setVolume = cwrap('setVolume', null, ['number']);
  setVolume(percent);
};

Module.getMainLoopTimingMode = () => {
  const getMainLoopTimingMode = cwrap('getMainLoopTimingMode', 'number', []);
  return getMainLoopTimingMode();
};

Module.getMainLoopTimingValue = () => {
  const getMainLoopTimingValue = cwrap('getMainLoopTimingValue', 'number', []);
  return getMainLoopTimingValue();
};

Module.setMainLoopTiming = (mode, value) => {
  const setMainLoopTiming = cwrap('setMainLoopTiming', 'number', [
    'number',
    'number',
  ]);
  setMainLoopTiming(mode, value);
};

Module.quitGame = () => {
  const quitGame = cwrap('quitGame', null, []);
  quitGame();
};

Module.quitMgba = () => {
  const quitMgba = cwrap('quitMgba', null, []);
  quitMgba();
};

Module.quickReload = () => {
  const quickReload = cwrap('quickReload', null, []);
  quickReload();
};

Module.toggleInput = (toggle) => {
  const setEventEnable = cwrap('setEventEnable', null, ['boolean']);
  setEventEnable(toggle);
};

Module.screenshot = (fileName) => {
  const screenshot = cwrap('screenshot', 'boolean', ['string']);
  return screenshot(fileName);
};

Module.saveState = (slot) => {
  const saveState = cwrap('saveState', 'boolean', ['number']);
  return saveState(slot);
};

Module.loadState = (slot) => {
  const loadState = cwrap('loadState', 'boolean', ['number']);
  return loadState(slot);
};

Module.forceAutoSaveState = () => {
  const autoSaveState = cwrap('autoSaveState', 'boolean', []);
  return autoSaveState();
};

Module.loadAutoSaveState = () => {
  const loadAutoSaveState = cwrap('loadAutoSaveState', 'boolean', []);
  return loadAutoSaveState();
};

Module.getAutoSaveState = () => {
  const exists = FS.analyzePath(Module.autoSaveStateName).exists;

  return exists
    ? {
        autoSaveStateName: Module.autoSaveStateName,
        data: FS.readFile(Module.autoSaveStateName),
      }
    : null;
};

Module.uploadAutoSaveState = async (autoSaveStateName, data) => {
  return new Promise((resolve, reject) => {
    try {
      if (!(data instanceof Uint8Array)) {
        console.warn('Auto save state data must be a Uint8Array');
        return;
      }

      if (!autoSaveStateName.length) {
        console.warn('Auto save state file name invalid');
        return;
      }

      const path = `${autoSaveStateName}`;
      FS.writeFile(path, data);

      resolve();
    } catch (err) {
      reject(err);
    }
  });
};

Module.saveStateSlot = (slot, flags) => {
  var saveStateSlot = cwrap('saveStateSlot', 'number', ['number', 'number']);
  Module.saveStateSlot = (slot, flags) => {
    if (flags === undefined) {
      flags = 0b111111;
    }
    return saveStateSlot(slot, flags);
  };
  return Module.saveStateSlot(slot, flags);
};

Module.loadStateSlot = (slot, flags) => {
  var loadStateSlot = cwrap('loadStateSlot', 'number', ['number', 'number']);
  Module.loadStateSlot = (slot, flags) => {
    if (flags === undefined) {
      flags = 0b111101;
    }
    return loadStateSlot(slot, flags);
  };
  return Module.loadStateSlot(slot, flags);
};

Module.autoLoadCheats = () => {
  const autoLoadCheats = cwrap('autoLoadCheats', 'bool', []);
  return autoLoadCheats();
};

Module.setFastForwardMultiplier = (multiplier) => {
  const setFastForwardMultiplier = cwrap('setFastForwardMultiplier', null, [
    'number',
  ]);
  setFastForwardMultiplier(multiplier);
};

Module.getFastForwardMultiplier = () => {
  const getFastForwardMultiplier = cwrap(
    'getFastForwardMultiplier',
    'number',
    []
  );
  return getFastForwardMultiplier();
};

// core callback store, used to persist long lived js function pointers for use in core callbacks in c
const coreCallbackStore = {
  alarmCallbackPtr: null,
  coreCrashedCallbackPtr: null,
  keysReadCallbackPtr: null,
  saveDataUpdatedCallbackPtr: null,
  videoFrameEndedCallbackPtr: null,
  videoFrameStartedCallbackPtr: null,
  autoSaveStateCapturedCallbackPtr: null,
  autoSaveStateLoadedCallbackPtr: null,
};

// adds user callbacks to the callback store, and makes function(s) available to the core in c
// passing null clears the callback, allowing for partial additions/removals of callbacks
Module.addCoreCallbacks = (callbacks) => {
  const addCoreCallbacks = cwrap('addCoreCallbacks', null, ['number']);

  Object.keys(coreCallbackStore).forEach((callbackKey) => {
    const callbackName = callbackKey.replace('CallbackPtr', 'Callback');
    const callback = callbacks[callbackName];

    // if the pointer is stored remove the old function pointer if a new callback was passed, or the callback is null
    if (callback !== undefined && !!coreCallbackStore[callbackKey]) {
      removeFunction(coreCallbackStore[callbackKey]);
      coreCallbackStore[callbackKey] = null;
    }

    // add the new function pointer to the store if present
    if (!!callback)
      coreCallbackStore[callbackKey] = addFunction(callback, 'vi');
  });

  // add the callbacks from the store to the core
  addCoreCallbacks(
    coreCallbackStore.alarmCallbackPtr,
    coreCallbackStore.coreCrashedCallbackPtr,
    coreCallbackStore.keysReadCallbackPtr,
    coreCallbackStore.saveDataUpdatedCallbackPtr,
    coreCallbackStore.videoFrameEndedCallbackPtr,
    coreCallbackStore.videoFrameStartedCallbackPtr,
    coreCallbackStore.autoSaveStateCapturedCallbackPtr,
    coreCallbackStore.autoSaveStateLoadedCallbackPtr
  );
};

Module.toggleRewind = (toggle) => {
  const toggleRewind = cwrap('toggleRewind', null, ['boolean']);

  toggleRewind(toggle);
};

Module.setCoreSettings = (coreSettings) => {
  const setIntegerCoreSetting = cwrap('setIntegerCoreSetting', null, [
    'string',
    'number',
  ]);

  if (coreSettings.allowOpposingDirections !== undefined)
    setIntegerCoreSetting(
      'allowOpposingDirections',
      coreSettings.allowOpposingDirections
    );

  if (coreSettings.rewindBufferCapacity !== undefined)
    setIntegerCoreSetting(
      'rewindBufferCapacity',
      coreSettings.rewindBufferCapacity
    );

  if (coreSettings.rewindBufferInterval !== undefined)
    setIntegerCoreSetting(
      'rewindBufferInterval',
      coreSettings.rewindBufferInterval
    );

  if (coreSettings?.frameSkip !== undefined)
    setIntegerCoreSetting('frameSkip', coreSettings.frameSkip);

  if (coreSettings.audioSampleRate !== undefined)
    setIntegerCoreSetting('audioSampleRate', coreSettings.audioSampleRate);

  if (coreSettings.audioBufferSize !== undefined)
    setIntegerCoreSetting('audioBufferSize', coreSettings.audioBufferSize);

  if (coreSettings.videoSync !== undefined)
    setIntegerCoreSetting('videoSync', coreSettings.videoSync);

  if (coreSettings.audioSync !== undefined)
    setIntegerCoreSetting('audioSync', coreSettings.audioSync);

  if (coreSettings.threadedVideo !== undefined)
    setIntegerCoreSetting('threadedVideo', coreSettings.threadedVideo);

  if (coreSettings.rewindEnable !== undefined)
    setIntegerCoreSetting('rewindEnable', coreSettings.rewindEnable);

  if (coreSettings.baseFpsTarget !== undefined)
    setIntegerCoreSetting('baseFpsTarget', coreSettings.baseFpsTarget);

  if (coreSettings.timestepSync !== undefined)
    setIntegerCoreSetting('timestepSync', coreSettings.timestepSync);

  if (coreSettings.showFpsCounter !== undefined)
    setIntegerCoreSetting('showFpsCounter', coreSettings.showFpsCounter);

  if (coreSettings.autoSaveStateTimerIntervalSeconds !== undefined)
    setIntegerCoreSetting(
      'autoSaveStateTimerIntervalSeconds',
      coreSettings.autoSaveStateTimerIntervalSeconds
    );

  if (coreSettings.autoSaveStateEnable !== undefined)
    setIntegerCoreSetting(
      'autoSaveStateEnable',
      coreSettings.autoSaveStateEnable
    );

  if (coreSettings.restoreAutoSaveStateOnLoad !== undefined)
    setIntegerCoreSetting(
      'restoreAutoSaveStateOnLoad',
      coreSettings.restoreAutoSaveStateOnLoad
    );
};
