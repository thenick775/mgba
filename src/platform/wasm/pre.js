Module.loadGame = (function () {
  var loadGame = cwrap('loadGame', 'number', ['string']);
  return function (name) {
    if (loadGame(name)) {
      var arr = name.split('.');
      arr.pop();
      Module.gameName = name;
      var saveName = arr.join('.') + '.sav';
      Module.saveName = saveName.replace('/data/games/', '/data/saves/');
      return true;
    }
    return false;
  };
})();

Module.getSave = function () {
  return FS.readFile(Module.saveName);
};

Module.listRoms = function () {
  return FS.readdir('/data/games/');
};

Module.listSaves = function () {
  return FS.readdir('/data/saves/');
};

// yanked from main.c for ease of use
Module.FSInit = function (callback) {
  FS.mkdir('/data');
  FS.mount(FS.filesystems.IDBFS, {}, '/data');

  // load data from IDBFS
  FS.syncfs(true, function (err) {
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

Module.FSSync = function () {
  // write data to IDBFS
  FS.syncfs(function (err) {
    if (err) {
      console.warn('Error syncing app data to IndexedDB', err);
    }
  });
};

Module.filePaths = function () {
  return {
    root: '/data',
    cheatsPath: '/data/cheats',
    gamePath: '/data/games',
    savePath: '/data/saves',
    saveStatePath: '/data/states',
  };
};

Module.uploadSaveOrSaveState = function (file, callback) {
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

  var reader = new FileReader();
  reader.onload = function (e) {
    FS.writeFile(dir + file.name, new Uint8Array(e.target.result));
    if (callback) {
      callback();
    }
  };

  reader.readAsArrayBuffer(file);
};

Module.uploadRom = function (file, callback) {
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

  var reader = new FileReader();
  reader.onload = function (e) {
    FS.writeFile(dir + file.name, new Uint8Array(e.target.result));
    if (callback) {
      callback();
    }
  };

  reader.readAsArrayBuffer(file);
};

Module.uploadCheats = function (file, callback) {
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

  var reader = new FileReader();
  reader.onload = function (e) {
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

Module.buttonPress = function (name) {
  var buttonPress = cwrap('buttonPress', null, ['number']);
  buttonPress(keyBindings.get(name.toLowerCase()));
};

Module.buttonUnpress = function (name) {
  var buttonUnpress = cwrap('buttonUnpress', null, ['number']);
  buttonUnpress(keyBindings.get(name.toLowerCase()));
};

// bindingName is the key name you want to associate to an input, ex. 'p' key binding -> 'a' input
// inputName is the name of the input to bind to, ex 'a', 'b', 'up' etc.
Module.bindKey = function (bindingName, inputName) {
  var bindKey = cwrap('bindKey', null, ['string', 'number']);
  bindKey(bindingName, keyBindings.get(inputName.toLowerCase()));
};

Module.pauseGame = function () {
  var pauseGame = cwrap('pauseGame', null, []);
  pauseGame();
};

Module.resumeGame = function () {
  var resumeGame = cwrap('resumeGame', null, []);
  resumeGame();
};

Module.getVolume = function () {
  var getVolume = cwrap('getVolume', 'number', []);
  return getVolume();
};

Module.setVolume = function (percent) {
  var setVolume = cwrap('setVolume', null, ['number']);
  setVolume(percent);
};

Module.getMainLoopTiming = function () {
  var getMainLoopTiming = cwrap('getMainLoopTiming', 'number', []);
  return getMainLoopTiming();
};

Module.setMainLoopTiming = function (mode, value) {
  var setMainLoopTiming = cwrap('setMainLoopTiming', 'number', [
    'number',
    'number',
  ]);
  setMainLoopTiming(mode, value);
};

Module.quitGame = function () {
  var quitGame = cwrap('quitGame', null, []);
  quitGame();
};

Module.quitMgba = function () {
  var quitMgba = cwrap('quitMgba', null, []);
  quitMgba();
};

Module.quickReload = function () {
  var quickReload = cwrap('quickReload', null, []);
  quickReload();
};

Module.toggleInput = function (toggle) {
  var setEventEnable = cwrap('setEventEnable', null, ['boolean']);
  setEventEnable(toggle);
};

Module.screenShot = function (callback) {
  const ptr = addFunction(callback);
  var screenShot = cwrap('screenShot', null, ['number']);
  screenShot(ptr);
  removeFunction(ptr);
};

Module.saveState = function (slot) {
  var saveState = cwrap('saveState', 'boolean', ['number']);
  return saveState(slot);
};

Module.loadState = function (slot) {
  var loadState = cwrap('loadState', 'boolean', ['number']);
  return loadState(slot);
};

Module.autoLoadCheats = function () {
  var autoLoadCheats = cwrap('autoLoadCheats', 'bool', []);
  return autoLoadCheats();
};
