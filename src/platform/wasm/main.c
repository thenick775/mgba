/* Copyright (c) 2013-2019 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <mgba-util/vfs.h>
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba/core/version.h>
#include <mgba/gba/interface.h>
#include <mgba/internal/gba/input.h>

#include "platform/sdl/sdl-audio.h"
#include "platform/sdl/sdl-events.h"

#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_keyboard.h>

static struct mCore* core = NULL;
static color_t* buffer = NULL;
static struct mSDLAudio audio = {
	.sampleRate = 48000,
	.samples = 512,
	.fpsTarget = 60.0,
};

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* tex = NULL;

// fps related variables
static double lastNow;
static bool renderFirstFrame = true;
static int fastForwardSpeed = 1;

static void _log(struct mLogger*, int category, enum mLogLevel level, const char* format, va_list args);
static struct mLogger logCtx = { .log = _log };

static void handleKeypressCore(const struct SDL_KeyboardEvent* event) {
	if (event->keysym.sym == SDLK_f) {
		fastForwardSpeed = event->type == SDL_KEYDOWN ? 2 : 1;
		return;
	}
	int key = -1;
	if (!(event->keysym.mod & ~(KMOD_NUM | KMOD_CAPS))) {
		key = mInputMapKey(&core->inputMap, SDL_BINDING_KEY, event->keysym.sym);
	}
	if (key != -1) {
		if (event->type == SDL_KEYDOWN) {
			core->addKeys(core, 1 << key);
		} else {
			core->clearKeys(core, 1 << key);
		}
	}
}

void testLoop() {
	union SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if (core) {
				handleKeypressCore(&event.key);
			}
			break;
		};
	}
	if (core) {
		double now = emscripten_get_now();
		double elapsedNow = now - (lastNow > 0.0 ? lastNow : now);
		double nowFrames = elapsedNow / (1000.0 / 60.0); // 60fps target

		lastNow = now;

		Uint32 nowFramesInt = round(nowFrames - 0.3);

		if (nowFramesInt < 1) {
			nowFramesInt = 1;
		}

		if (fastForwardSpeed > 1)
			nowFramesInt *= fastForwardSpeed;

		if (nowFramesInt > 20)
			nowFramesInt = 20;

		if (renderFirstFrame) {
			renderFirstFrame = false;
			core->runFrame(core);
		} else {
			for (Uint32 i = 0; i < nowFramesInt; i++) {
				core->runFrame(core);
			}
		}

		unsigned w, h;
		core->currentVideoSize(core, &w, &h);

		SDL_Rect rect = {
			.x = 0,
			.y = 0,
			.w = w,
			.h = h
		};
		SDL_UnlockTexture(tex);
		SDL_RenderCopy(renderer, tex, &rect, &rect);
		SDL_RenderPresent(renderer);

		int stride;
		SDL_LockTexture(tex, 0, (void**) &buffer, &stride);
		core->setVideoBuffer(core, buffer, stride / BYTES_PER_PIXEL);
		return;
	} else {
		// dont run the main loop if there is no core,  we don't
		// want to handle events unless the core is running for now
		renderFirstFrame = true;
		emscripten_pause_main_loop();
	}
}

EMSCRIPTEN_KEEPALIVE bool screenshot(char* fileName) {
	bool success = false;
	int mode = O_CREAT | O_TRUNC | O_WRONLY;
	struct VFile* vf;

	if (!core)
		return false;

	struct VDir* dir = core->dirs.screenshot;

	if (strlen(fileName)) {
		vf = dir->openFile(dir, fileName, mode);
	} else {
		vf = VDirFindNextAvailable(dir, core->dirs.baseName, "-", ".png", mode);
	}

	if (!vf)
		return false;

	success = mCoreTakeScreenshotVF(core, vf);
	vf->close(vf);

	return success;
}

EMSCRIPTEN_KEEPALIVE void buttonPress(int id) {
  if (core) {
    core->addKeys(core, 1 << id);
  }
}

EMSCRIPTEN_KEEPALIVE void buttonUnpress(int id) {
  if (core) {
    core->clearKeys(core, 1 << id);
  }
}

EMSCRIPTEN_KEEPALIVE void setVolume(float vol) {
	if (vol > 2.0 || vol < 0)
		return; // this is a percentage so more than 200% is insane.

	int volume = (int) (vol * 0x100);
	if (core) {
		if (volume == 0)
			return mSDLPauseAudio(&audio);
		else {
			mCoreConfigSetDefaultIntValue(&core->config, "volume", volume);
			core->reloadConfigOption(core, "volume", &core->config);
			mSDLResumeAudio(&audio);
		}
	}
}

EMSCRIPTEN_KEEPALIVE float getVolume() {
  return (float) core->opts.volume / 0x100;
}

EMSCRIPTEN_KEEPALIVE int getMainLoopTimingMode() {
  int mode = -1;
  int value = -1;
  emscripten_get_main_loop_timing(&mode, &value);
  return mode;
}

EMSCRIPTEN_KEEPALIVE int getMainLoopTimingValue() {
  int mode = -1;
  int value = -1;
  emscripten_get_main_loop_timing(&mode, &value);
  return value;
}

EMSCRIPTEN_KEEPALIVE void setMainLoopTiming(int mode, int value) {
  emscripten_set_main_loop_timing(mode, value);
}

EMSCRIPTEN_KEEPALIVE void setFastForwardMultiplier(int multiplier) {
	if (multiplier > 0) {
		fastForwardSpeed = multiplier;
		audio.fpsTarget = (double) 60 * multiplier;

		mCoreConfigSetDefaultIntValue(&core->config, "frameskip", multiplier);
		core->reloadConfigOption(core, "frameskip", &core->config);
	}
}

EMSCRIPTEN_KEEPALIVE int getFastForwardMultiplier() {
	return fastForwardSpeed;
}

EMSCRIPTEN_KEEPALIVE void quitGame() {
  if (core) {
	renderFirstFrame = true;
	mSDLPauseAudio(&audio);
	emscripten_pause_main_loop();
    core->deinit(core);
    core = NULL;
  }
}

EMSCRIPTEN_KEEPALIVE void quitMgba() {
  exit(0);
}

EMSCRIPTEN_KEEPALIVE void quickReload() {
  renderFirstFrame = true;
  core->reset(core);
}

EMSCRIPTEN_KEEPALIVE void pauseGame() {
	renderFirstFrame = true;
	mSDLPauseAudio(&audio);
	emscripten_pause_main_loop();
}

EMSCRIPTEN_KEEPALIVE void resumeGame() {
	mSDLResumeAudio(&audio);
	emscripten_resume_main_loop();
}

EMSCRIPTEN_KEEPALIVE void setEventEnable(bool toggle) {
    int state = toggle ? SDL_ENABLE : SDL_DISABLE;
    SDL_EventState(SDL_TEXTINPUT, state);
    SDL_EventState(SDL_KEYDOWN, state);
    SDL_EventState(SDL_KEYUP, state);
    SDL_EventState(SDL_MOUSEMOTION, state);
    SDL_EventState(SDL_MOUSEBUTTONDOWN, state);
    SDL_EventState(SDL_MOUSEBUTTONUP, state);
}

// bindingName is the key name of what you want to bind to an input
// inputCode is the code of the key input, see keyBindings in pre.js
// this should work for a good variety of keys, but not all are supported yet
EMSCRIPTEN_KEEPALIVE void bindKey(char* bindingName, int inputCode) {
	int bindingSDLKeyCode = SDL_GetKeyFromName(bindingName);
	mInputBindKey(&core->inputMap, SDL_BINDING_KEY, bindingSDLKeyCode, inputCode);
}

EMSCRIPTEN_KEEPALIVE bool saveState(int slot) {
  if (core) {
	  return mCoreSaveState(core, slot, SAVESTATE_ALL);
  }
  return false;
}

EMSCRIPTEN_KEEPALIVE bool loadState(int slot) {
  if (core) {
    return mCoreLoadState(core, slot, SAVESTATE_ALL);
  }
  return false;
}

// loads all cheats files located in the cores cheatsPath,
// cheat files must match the name of the rom they are
// to be applied to, and must end with the extension .cheats
// supported cheat formats: 
//  - mGBA custom format
//  - libretro format
//  - EZFCht format
EMSCRIPTEN_KEEPALIVE bool autoLoadCheats() {
	return mCoreAutoloadCheats(core);
}

EMSCRIPTEN_KEEPALIVE bool loadGame(const char* name) {
	if (core) {
		core->deinit(core);
		core = NULL;
	}
	core = mCoreFind(name);
	if (!core) {
		return false;
	}
	core->init(core);
	core->opts.savegamePath = strdup("/data/saves");
	core->opts.savestatePath = strdup("/data/states");
	core->opts.cheatsPath = strdup("/data/cheats");
	core->opts.screenshotPath = strdup("/data/screenshots");

	mCoreLoadFile(core, name);
	mCoreConfigInit(&core->config, "wasm");
	mCoreConfigSetDefaultValue(&core->config, "idleOptimization", "detect");
	mCoreConfigSetDefaultIntValue(&core->config, "volume", 0x100);
	mInputMapInit(&core->inputMap, &GBAInputInfo);
	mDirectorySetMapOptions(&core->dirs, &core->opts);
	mCoreAutoloadSave(core);
	mCoreAutoloadCheats(core);
	mSDLInitBindingsGBA(&core->inputMap);

	unsigned w, h;
	core->baseVideoSize(core, &w, &h);
	if (tex) {
		SDL_DestroyTexture(tex);
	}
	tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, w, h);

	int stride;
	SDL_LockTexture(tex, 0, (void**) &buffer, &stride);
	core->setVideoBuffer(core, buffer, stride / BYTES_PER_PIXEL);
	core->setAudioBufferSize(core, audio.samples * 8);

	core->reset(core);

	core->currentVideoSize(core, &w, &h);
	SDL_SetWindowSize(window, w, h);
	EM_ASM({
		Module.canvas.width = $0;
		Module.canvas.height = $1;
	}, w, h);

	audio.core = core;
	mSDLResumeAudio(&audio);
	emscripten_resume_main_loop();
	return true;
}

EMSCRIPTEN_KEEPALIVE bool saveStateSlot(int slot, int flags) {
	if (!core) {
		return false;
	}
	return mCoreSaveState(core, slot, flags);
}

EMSCRIPTEN_KEEPALIVE bool loadStateSlot(int slot, int flags) {
	if (!core) {
		return false;
	}
	return mCoreLoadState(core, slot, flags);
}

void _log(struct mLogger* logger, int category, enum mLogLevel level, const char* format, va_list args) {
	UNUSED(logger);
	UNUSED(category);
	UNUSED(level);
	UNUSED(format);
	UNUSED(args);
}


EMSCRIPTEN_KEEPALIVE void setupConstants(void) {
	EM_ASM(({
		Module.version = {
			gitCommit: UTF8ToString($0),
			gitShort: UTF8ToString($1),
			gitBranch: UTF8ToString($2),
			gitRevision: $3,
			binaryName: UTF8ToString($4),
			projectName: UTF8ToString($5),
			projectVersion: UTF8ToString($6)
		};
	}), gitCommit, gitCommitShort, gitBranch, gitRevision, binaryName, projectName, projectVersion);
}

CONSTRUCTOR(premain) {
	setupConstants();
}

int excludeKeys(void *userdata, SDL_Event *event) {
	UNUSED(userdata);

	switch (event->key.keysym.sym) {
		case SDLK_TAB: // ignored for a11y during gameplay
		case SDLK_SPACE:
			return 0; // Value will be ignored
		default:
			return 1;
	};
}

int main() {
	mLogSetDefaultLogger(&logCtx);

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO| SDL_INIT_TIMER | SDL_INIT_EVENTS);
	window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GBA_VIDEO_HORIZONTAL_PIXELS, GBA_VIDEO_VERTICAL_PIXELS, SDL_WINDOW_OPENGL);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	mSDLInitAudio(&audio, NULL);
	
	// exclude specific key events
	SDL_SetEventFilter(excludeKeys, NULL);

	emscripten_set_main_loop(testLoop, 0, 0);
	return 0;
}
