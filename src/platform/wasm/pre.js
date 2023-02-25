Module.loadGame = (function() {
	var loadGame = cwrap('loadGame', 'number', ['string']);
	return function(name) {
		if (loadGame(name)) {
			var arr = name.split('.');
			arr.pop();
			Module.gameName = name;
			var saveName = arr.join('.') + '.sav';
			Module.saveName = saveName.replace('/data/games/','/data/saves/');
			return true;
		}
		return false;
	}
})();

Module.getSave = function() {
	return FS.readFile(Module.saveName);
}

Module.listRoms = function(){
	return FS.readdir('/data/games/')
}

Module.listSaves = function(){
	return FS.readdir('/data/saves/')
}

// yanked from main.c for ease of use
Module.FSInit = function(){
	FS.mkdir('/data');
	FS.mount(FS.filesystems.IDBFS, {}, '/data');

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
}

Module.uploadSaveOrSaveState = function(file) {
    const split = file.name.split('.');
    if (split.length < 2) {
      window.alert('unrecognized file extension: ' + file.name);
      return;
    }
    const extension = split[split.length - 1].toLowerCase();

    let dir = null;
    if (extension == 'sav') {
      dir = '/data/saves/';
    } else if (extension.startsWith('ss')) {
      dir = '/data/states/';
    } else {
      window.alert('unrecognized file extension: ' + extension);
      return;
    }

	FS.writeFile(dir+name, new Uint8Array(buffer));
}

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
}

Module.buttonUnpress = function (name) {
	var buttonUnpress = cwrap('buttonUnpress', null, ['number']);
	buttonUnpress(keyBindings.get(name.toLowerCase()));
}

// bindingName is the key name you want to associate to an input, ex. 'p' key binding -> 'a' input
// inputName is the name of the input to bind to, ex 'a', 'b', 'up' etc.
Module.bindKey = function (bindingName, inputName) {
	var bindKey = cwrap('bindKey', null, ['string', 'number']);
	bindKey(bindingName, keyBindings.get(inputName.toLowerCase()));
}

Module.pauseGame = function () {
	var pauseGame = cwrap('pauseGame', null, []);
	pauseGame();
}

Module.resumeGame = function () {
	var resumeGame = cwrap('resumeGame', null, []);
	resumeGame();
}

Module.getVolume = function () {
	var getVolume = cwrap('getVolume', 'number', []);
	return getVolume();
}

Module.setVolume = function (percent) {
	var setVolume = cwrap('setVolume', null, ['number']);
	setVolume(percent);
}

Module.getMainLoopTiming = function () {
	var getMainLoopTiming = cwrap('getMainLoopTiming', null, []);
}

Module.setMainLoopTiming = function (mode, value) {
	var setMainLoopTiming = cwrap('setMainLoopTiming', 'number', ['number', 'number']);
	setMainLoopTiming(mode, value);
}

Module.quitGame = function () {
	var quitGame = cwrap('quitGame', null, []);
	quitGame();
}

Module.quitMgba = function () {
	var quitMgba = cwrap('quitMgba', null, []);
	quitMgba();
}

Module.quickReload = function () {
	var quickReload = cwrap('quickReload', null, []);
	quickReload();
}

Module.toggleInput = function (toggle) {
	var setEventEnable = cwrap('setEventEnable', null, ['boolean'])
	setEventEnable(toggle);
}

Module.screenShot = function (callback) {
	ptr = addFunction(callback);
	var screenShot = cwrap('screenShot', null, ['number']);
	screenShot(ptr);
	removeFunction(ptr);
}

Module.saveState = function(slot) {
	var saveState = cwrap('saveState', 'boolean', ['number'])
	return saveState(slot);
}

Module.loadState = function(slot) {
	var loadState = cwrap('loadState', 'boolean', ['number'])
	return loadState(slot);
}
