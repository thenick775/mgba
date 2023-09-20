/* Copyright (c) 2013-2023 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

Module.loadFile = (name) => {
	var loadGame = cwrap('loadGame', 'number', ['string']);
	Module.loadFile = (name) => {
		if (loadGame(name)) {
			var arr = name.split('.');
			arr.pop();
			Module.gameName = name;
			Module.saveName = arr.join('.') + '.sav';
			return true;
		}
		return false;
	}
	return Module.loadFile(name);
};

Module.getSave = () => {
	return FS.readFile('/data/saves/' + Module.saveName);
}

Module.saveStateSlot = (slot, flags) => {
	var saveStateSlot = cwrap('saveStateSlot', 'number', ['number', 'number']);
	Module.saveStateSlot = (slot, flags) => {
		if (flags === undefined) {
			flags = 0b111111;
		}
		return saveStateSlot(slot, flags);
	}
	return Module.saveStateSlot(slot, flags);
};

Module.loadStateSlot = (slot, flags) => {
	var loadStateSlot = cwrap('loadStateSlot', 'number', ['number', 'number']);
	Module.loadStateSlot = (slot, flags) => {
		if (flags === undefined) {
			flags = 0b111101;
		}
		return loadStateSlot(slot, flags);
	}
	return Module.loadStateSlot(slot, flags);
};
