Module.loadGame = (function() {
	var loadGame = cwrap('loadGame', 'number', ['string']);
	return function(name) {
		if (loadGame(name)) {
			var arr = name.split('.');
			arr.pop();
			Module.gameName = name;
			Module.saveName = arr.join('.') + '.sav';
			return true;
		}
		return false;
	}
})();

Module.getSave = function() {
	return FS.readFile('/data/saves/' + Module.saveName);
}

Module.listRoms = function(){
	return FS.readdir("/data/games/")
}

Module.listSaves = function(){
	return FS.readdir("/data/saves/")
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

Module.loadSaveOrSaveState = function(file) {
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

//vancise what to do about this
const buttonNameToId = new Map();
buttonNameToId.set('a', 0);
buttonNameToId.set('b', 1);
buttonNameToId.set('select', 2);
buttonNameToId.set('start', 3);
buttonNameToId.set('right', 4);
buttonNameToId.set('left', 5);
buttonNameToId.set('up', 6);
buttonNameToId.set('down', 7);
buttonNameToId.set('r', 8);
buttonNameToId.set('l', 9);
const buttonIdToName = new Map();
for (const [key, value] of buttonNameToId) {
  buttonIdToName.set(value, key);
}

Module.buttonPress = function (name) {
	var buttonPress = cwrap('buttonPress', null, ['number']);
	buttonPress(buttonNameToId.get(name.toLowerCase()));
}

Module.buttonUnpress = function (name) {
	var buttonUnpress = cwrap('buttonUnpress', null, ['number']);
	buttonUnpress(buttonNameToId.get(name.toLowerCase()));
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
