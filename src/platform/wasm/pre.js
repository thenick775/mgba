Module.loadGame = (name) => {
  const loadGame = cwrap('loadGame', 'number', ['string']);

  if (loadGame(name)) {
    const arr = name.split('.');
    arr.pop();

    const saveName = arr.join('.') + '.sav';

    Module.gameName = name;
    Module.saveName = saveName.replace('/data/games/', '/data/saves/');
    return true;
  }

  return false;
};

Module.getSave = () => {
  return FS.readFile(Module.saveName);
};

Module.listRoms = () => {
  return FS.readdir('/data/games/');
};

Module.listSaves = () => {
  return FS.readdir('/data/saves/');
};

// yanked from main.c for ease of use
Module.FSInit = (callback) => {
  FS.mkdir('/data');
  FS.mount(FS.filesystems.IDBFS, {}, '/data');

  // load data from IDBFS
  FS.syncfs(true, (err) => {
    if (err) {
      console.warn('Error syncing app data from IndexedDB: ', err);
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

    // using a callback to indicate fs ready state if desired
    if (callback) {
      callback();
    }
  });
};

Module.FSSync = () => {
  // write data to IDBFS
  FS.syncfs((err) => {
    if (err) {
      console.warn('Error syncing app data to IndexedDB', err);
    }
  });
};

Module.filePaths = () => {
  return {
    root: '/data',
    cheatsPath: '/data/cheats',
    gamePath: '/data/games',
    savePath: '/data/saves',
    saveStatePath: '/data/states',
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
  if (extension == 'gba' || extension == 'gbc' || extension == 'gb') {
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

Module.getVolume = () => {
  const getVolume = cwrap('getVolume', 'number', []);
  return getVolume();
};

Module.setVolume = (percent) => {
  const setVolume = cwrap('setVolume', null, ['number']);
  setVolume(percent);
};

Module.getMainLoopTiming = () => {
  const getMainLoopTiming = cwrap('getMainLoopTiming', 'number', []);
  return getMainLoopTiming();
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

Module.screenShot = (callback) => {
  const ptr = addFunction(callback);
  const screenShot = cwrap('screenShot', null, ['number']);
  screenShot(ptr);
  removeFunction(ptr);
};

Module.saveState = (slot) => {
  const saveState = cwrap('saveState', 'boolean', ['number']);
  return saveState(slot);
};

Module.loadState = (slot) => {
  const loadState = cwrap('loadState', 'boolean', ['number']);
  return loadState(slot);
};

Module.autoLoadCheats = () => {
  const autoLoadCheats = cwrap('autoLoadCheats', 'bool', []);
  return autoLoadCheats();
};
