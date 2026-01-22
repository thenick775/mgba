/* Copyright (c) 2013-2019 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "main.h"

#include <mgba-util/vfs.h>
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba/core/thread.h>
#include <mgba/core/version.h>
#include <mgba/gba/interface.h>
#include <mgba/internal/gba/input.h>

#include "platform/sdl/sdl-audio.h"
#include "platform/sdl/sdl-events.h"
#include "platform/sdl/sdl-text.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_keyboard.h>
#include <emscripten.h>
#include <emscripten/threading.h>

// global renderer
static struct mEmscriptenRenderer* renderer = NULL;

// log utilities
static void _log(struct mLogger*, int category, enum mLogLevel level, const char* format, va_list args);
static struct mLogger logCtx = { .log = _log };

// stored core callback function pointers
typedef struct {
	void (*alarm)(void*);
	void (*coreCrashed)(void*);
	void (*keysRead)(void*);
	void (*savedataUpdated)(void*);
	void (*videoFrameEnded)(void*);
	void (*videoFrameStarted)(void*);
	void (*autoSaveStateCaptured)(void*);
	void (*autoSaveStateLoaded)(void*);
} CallbackStorage;

static CallbackStorage callbackStorage;

// Macro to create wrapper functions, necessary as some core callbacks want to access
// things like the file system on the CPU thread, which can only be done on the main thread.
#define DEFINE_WRAPPER(field)                            \
	static void wrapped_##field(void* context) {         \
		MAIN_THREAD_EM_ASM(                              \
		    {                                            \
			    const funcPtr = $0;                      \
			    const ctx = $1;                          \
			    const func = wasmTable.get(funcPtr);     \
			    if (func)                                \
				    func(ctx);                           \
		    },                                           \
		    (int) callbackStorage.field, (int) context); \
	}

// Generate wrapper functions
DEFINE_WRAPPER(alarm)
DEFINE_WRAPPER(coreCrashed)
DEFINE_WRAPPER(keysRead)
DEFINE_WRAPPER(savedataUpdated)
DEFINE_WRAPPER(videoFrameEnded)
DEFINE_WRAPPER(videoFrameStarted)

/**
 * Exposed core contract methods
 */

EMSCRIPTEN_KEEPALIVE bool screenshot(char* fileName) {
	bool success = false;
	int mode = O_CREAT | O_TRUNC | O_WRONLY;
	struct VFile* vf;

	if (!renderer->core)
		return false;

	struct VDir* dir = renderer->core->dirs.screenshot;

	if (strlen(fileName)) {
		vf = dir->openFile(dir, fileName, mode);
	} else {
		vf = VDirFindNextAvailable(dir, renderer->core->dirs.baseName, "-", ".png", mode);
	}

	if (!vf)
		return false;

	success = mCoreTakeScreenshotVF(renderer->core, vf);
	vf->close(vf);

	return success;
}

EMSCRIPTEN_KEEPALIVE void buttonPress(int id) {
	if (renderer->core && renderer->thread) {
		renderer->core->addKeys(renderer->core, 1 << id);
	}
}

EMSCRIPTEN_KEEPALIVE void buttonUnpress(int id) {
	if (renderer->core && renderer->thread) {
		renderer->core->clearKeys(renderer->core, 1 << id);
	}
}

EMSCRIPTEN_KEEPALIVE void toggleRewind(bool toggle) {
	if (renderer->thread)
		mCoreThreadSetRewinding(renderer->thread, toggle);
}

EMSCRIPTEN_KEEPALIVE void setVolume(float vol) {
	if (vol > 2.0 || vol < 0)
		return; // this is a percentage so more than 200% is insane.

	int volume = (int) (vol * 0x100);
	if (renderer->core) {
		mCoreConfigSetDefaultIntValue(&renderer->core->config, "volume", volume);
		renderer->core->reloadConfigOption(renderer->core, "volume", &renderer->core->config);
	}
}

EMSCRIPTEN_KEEPALIVE float getVolume() {
	if (renderer->core)
		return (float) renderer->core->opts.volume / 0x100;
	else
		return 0.0;
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

// full handling of fast forward, interrupts and thread logic included
void updateFastForward(double multiplier) {
	if (renderer->thread && renderer->videoSync) {
		mCoreThreadInterrupt(renderer->thread);
		renderer->thread->impl->sync.videoFrameWait = (multiplier > 1) ? false : renderer->videoSync;
		renderer->thread->impl->sync.audioWait = (multiplier > 1) ? true : renderer->audioSync;
		mCoreThreadContinue(renderer->thread);
	}

	if (renderer->core && renderer->thread && multiplier != 0) {
		renderer->thread->impl->sync.fpsTarget = (double) renderer->baseFpsTarget * multiplier;
		mCoreConfigSetDefaultFloatValue(&renderer->core->config, "fpsTarget",
		                                (double) renderer->baseFpsTarget * multiplier);
		renderer->core->reloadConfigOption(renderer->core, "fpsTarget", &renderer->core->config);

		if (renderer->frameSkip == 0) {
			// fast forward starts at 1, frameskip starts at 0
			mCoreConfigSetDefaultIntValue(&renderer->core->config, "frameskip", multiplier - 1);
			renderer->core->reloadConfigOption(renderer->core, "frameskip", &renderer->core->config);
		}
	}
}

EMSCRIPTEN_KEEPALIVE void setFastForwardMultiplier(int multiplier) {
	if (multiplier > 0) {
		renderer->fastForwardMultiplier = multiplier;
	} else if (multiplier < 0) {
		renderer->fastForwardMultiplier = -1.0 / (double) multiplier;
	}

	updateFastForward(renderer->fastForwardMultiplier);
}

EMSCRIPTEN_KEEPALIVE int getFastForwardMultiplier() {
	double m = renderer->fastForwardMultiplier;
	// reverse the translation used in setFastForwardMultiplier
	return (m < 1.0) ? (int) (-1.0 / m) : (int) m;
}

EMSCRIPTEN_KEEPALIVE void quitGame() {
	if (renderer->core && renderer->thread) {
		emscripten_pause_main_loop();
		mSDLPauseAudio(&renderer->audio);
		mSDLDeinitAudio(&renderer->audio);

		mCoreThreadEnd(renderer->thread);
		mCoreThreadJoin(renderer->thread);
		free(renderer->thread);
		renderer->thread = NULL;

		renderer->core->unloadROM(renderer->core);
		mCoreConfigDeinit(&renderer->core->config);
		mInputMapDeinit(&renderer->core->inputMap);
		renderer->core->deinit(renderer->core);
		renderer->core = NULL;
		renderer->audio.core = NULL;

		renderer->autoSaveStateTimer.lastTime = 0;
	}
}

EMSCRIPTEN_KEEPALIVE void quitMgba() {
	exit(0);
}

EMSCRIPTEN_KEEPALIVE bool autoSaveState() {
	if (renderer->core && renderer->thread) {
		bool result = false;
		struct VDir* autosaveDir = VDirOpen("/autosave");
		char autoSaveName[PATH_MAX + 14];
		snprintf(autoSaveName, sizeof(autoSaveName), "%s_auto.ss", renderer->core->dirs.baseName);

		struct VFile* vf = autosaveDir->openFile(autosaveDir, autoSaveName, (O_CREAT | O_TRUNC | O_RDWR));

		if (!vf)
			return false;

		mCoreThreadInterrupt(renderer->thread);
		result = mCoreSaveStateNamed(renderer->core, vf, SAVESTATE_ALL);
		mCoreThreadContinue(renderer->thread);
		bool successfullyClosed = vf->close(vf);

		return result && successfullyClosed;
	}

	return false;
}

EMSCRIPTEN_KEEPALIVE bool loadAutoSaveState() {
	bool result = false;
	if (renderer->core && renderer->thread) {
		struct VDir* autosaveDir = VDirOpen("/autosave");
		char autoSaveName[PATH_MAX + 14];
		snprintf(autoSaveName, sizeof(autoSaveName), "%s_auto.ss", renderer->core->dirs.baseName);

		struct VFile* vf = autosaveDir->openFile(autosaveDir, autoSaveName, O_RDONLY);

		if (!vf)
			return false;

		mCoreThreadInterrupt(renderer->thread);
		result = mCoreLoadStateNamed(renderer->core, vf, SAVESTATE_ALL);
		mCoreThreadContinue(renderer->thread);
		return result;
	}
	return false;
}

EMSCRIPTEN_KEEPALIVE void quickReload() {
	if (renderer->core && renderer->thread) {
		mCoreThreadInterrupt(renderer->thread);

		renderer->core->reset(renderer->core);

		if (renderer->restoreAutoSaveStateOnLoad) {
			bool successfulLoad = loadAutoSaveState();
			if (successfulLoad && callbackStorage.autoSaveStateLoaded)
				callbackStorage.autoSaveStateLoaded(NULL);
		}

		mCoreThreadContinue(renderer->thread);
	}
}

EMSCRIPTEN_KEEPALIVE void pauseGame() {
	mSDLPauseAudio(&renderer->audio);

	if (renderer->thread)
		mCoreThreadPause(renderer->thread);

	renderer->autoSaveStateTimer.lastTime = 0;

	emscripten_pause_main_loop();
}

EMSCRIPTEN_KEEPALIVE void resumeGame() {
	mSDLResumeAudio(&renderer->audio);

	if (renderer->thread)
		mCoreThreadUnpause(renderer->thread);

	emscripten_resume_main_loop();
}

EMSCRIPTEN_KEEPALIVE void pauseAudio() {
	mSDLPauseAudio(&renderer->audio);
}

EMSCRIPTEN_KEEPALIVE void resumeAudio() {
	mSDLResumeAudio(&renderer->audio);
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

	if (renderer->core)
		mInputBindKey(&renderer->core->inputMap, SDL_BINDING_KEY, bindingSDLKeyCode, inputCode);
}

EMSCRIPTEN_KEEPALIVE bool saveState(int slot) {
	bool result = false;
	if (renderer->core && renderer->thread) {
		mCoreThreadInterrupt(renderer->thread);
		result = mCoreSaveState(renderer->core, slot, SAVESTATE_ALL);
		mCoreThreadContinue(renderer->thread);
		return result;
	}
	return false;
}

EMSCRIPTEN_KEEPALIVE bool loadState(int slot) {
	bool result = false;
	if (renderer->core && renderer->thread) {
		mCoreThreadInterrupt(renderer->thread);
		result = mCoreLoadState(renderer->core, slot, SAVESTATE_ALL);
		mCoreThreadContinue(renderer->thread);
		return result;
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
	if (renderer->core && renderer->thread) {
		bool result = false;
		mCoreThreadInterrupt(renderer->thread);
		result = mCoreAutoloadCheats(renderer->core);
		mCoreThreadContinue(renderer->thread);
		return result;
	}
	return false;
}

EMSCRIPTEN_KEEPALIVE bool loadGame(const char* name, const char* savePathOverride) {
	if (renderer->thread && renderer->core) {
		quitGame();
	}
	renderer->core = mCoreFind(name);
	if (!renderer->core) {
		return false;
	}
	renderer->core->init(renderer->core);
	renderer->core->opts.savegamePath = strdup("/data/saves");
	renderer->core->opts.savestatePath = strdup("/data/states");
	renderer->core->opts.cheatsPath = strdup("/data/cheats");
	renderer->core->opts.screenshotPath = strdup("/data/screenshots");
	renderer->core->opts.patchPath = strdup("/data/patches");
	renderer->core->opts.audioBuffers = renderer->audio.samples;

	mCoreConfigInit(&renderer->core->config, "wasm");
	struct mCoreOptions defaultConfigOpts = { .useBios = true,
		                                      .rewindEnable = renderer->rewindEnable,
		                                      .rewindBufferCapacity = renderer->rewindBufferCapacity,
		                                      .rewindBufferInterval = renderer->rewindBufferInterval,
		                                      .videoSync = renderer->videoSync,
		                                      .audioSync = renderer->audioSync,
		                                      .fpsTarget = renderer->baseFpsTarget,
		                                      .volume = 0x100,
		                                      .logLevel = mLOG_WARN | mLOG_ERROR | mLOG_FATAL };

	mCoreConfigLoadDefaults(&renderer->core->config, &defaultConfigOpts);
	mCoreLoadConfig(renderer->core);

	mCoreLoadFile(renderer->core, name);
	mCoreConfigSetDefaultIntValue(&renderer->core->config, "timestepSync", renderer->timestepSync);
	mCoreConfigSetDefaultValue(&renderer->core->config, "idleOptimization", "detect");
	renderer->core->reloadConfigOption(renderer->core, "idleOptimization", &renderer->core->config);
	mCoreConfigSetDefaultIntValue(&renderer->core->config, "allowOpposingDirections", true);
	renderer->core->reloadConfigOption(renderer->core, "allowOpposingDirections", &renderer->core->config);
	mCoreConfigSetDefaultIntValue(&renderer->core->config, "threadedVideo", renderer->threadedVideo);
	renderer->core->reloadConfigOption(renderer->core, "threadedVideo", &renderer->core->config);
	mInputMapInit(&renderer->core->inputMap, &GBAInputInfo);
	mDirectorySetMapOptions(&renderer->core->dirs, &renderer->core->opts);

	if (savePathOverride)
		mCoreLoadSaveFile(renderer->core, savePathOverride, false);
	else
		mCoreAutoloadSave(renderer->core);

	mCoreAutoloadCheats(renderer->core);
	mCoreAutoloadPatch(renderer->core);
	mSDLInitBindingsGBA(&renderer->core->inputMap);

	unsigned w, h;
	renderer->core->baseVideoSize(renderer->core, &w, &h);
	if (renderer->sdlTex) {
		SDL_DestroyTexture(renderer->sdlTex);
	}
	renderer->sdlTex =
	    SDL_CreateTexture(renderer->sdlRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, w, h);

	int stride;
	SDL_LockTexture(renderer->sdlTex, 0, (void**) &renderer->outputBuffer, &stride);
	renderer->core->setVideoBuffer(renderer->core, renderer->outputBuffer, stride / BYTES_PER_PIXEL);
	renderer->core->setAudioBufferSize(renderer->core, renderer->audio.samples);

	renderer->core->reset(renderer->core);

	renderer->core->currentVideoSize(renderer->core, &w, &h);
	SDL_SetWindowSize(renderer->window, w, h);
	EM_ASM(
	    {
		    Module.canvas.width = $0;
		    Module.canvas.height = $1;
	    },
	    w, h);

	emscripten_resume_main_loop();
	return true;
}

EMSCRIPTEN_KEEPALIVE bool saveStateSlot(int slot, int flags) {
	if (!renderer->core) {
		return false;
	}
	return mCoreSaveState(renderer->core, slot, flags);
}

EMSCRIPTEN_KEEPALIVE bool loadStateSlot(int slot, int flags) {
	if (!renderer->core) {
		return false;
	}
	return mCoreLoadState(renderer->core, slot, flags);
}

// Function to ensure all callbacks execute on the main thread
EMSCRIPTEN_KEEPALIVE void
addCoreCallbacks(void (*alarmCallbackPtr)(void*), void (*coreCrashedCallbackPtr)(void*),
                 void (*keysReadCallbackPtr)(void*), void (*saveDataUpdatedCallbackPtr)(void*),
                 void (*videoFrameEndedCallbackPtr)(void*), void (*videoFrameStartedCallbackPtr)(void*),
                 void (*autoSaveStateCapturedCallbackPtr)(void*), void (*autoSaveStateLoadedCallbackPtr)(void*)) {
	if (renderer->core) {
		struct mCoreCallbacks callbacks = {};
		// clear core callbacks
		renderer->core->clearCoreCallbacks(renderer->core);
		// clear ad-hoc callbacks
		callbackStorage.autoSaveStateCaptured = NULL;
		callbackStorage.autoSaveStateLoaded = NULL;

		// the thread has its own suite of callbacks where ours should overlay on top,
		// after clearing the current core callbacks since there is not a mechanism to
		// filter the callbacks, we need to re-add the thread callbacks
		if (renderer->thread)
			mCoreThreadAddCoreCallbacks(renderer->thread);

		// store original function pointers
		if (alarmCallbackPtr)
			callbackStorage.alarm = alarmCallbackPtr;
		if (coreCrashedCallbackPtr)
			callbackStorage.coreCrashed = coreCrashedCallbackPtr;
		if (keysReadCallbackPtr)
			callbackStorage.keysRead = keysReadCallbackPtr;
		if (saveDataUpdatedCallbackPtr)
			callbackStorage.savedataUpdated = saveDataUpdatedCallbackPtr;
		if (videoFrameEndedCallbackPtr)
			callbackStorage.videoFrameEnded = videoFrameEndedCallbackPtr;
		if (videoFrameStartedCallbackPtr)
			callbackStorage.videoFrameStarted = videoFrameStartedCallbackPtr;

		// store original ad-hoc function pointers
		if (autoSaveStateCapturedCallbackPtr)
			callbackStorage.autoSaveStateCaptured = autoSaveStateCapturedCallbackPtr;
		if (autoSaveStateLoadedCallbackPtr)
			callbackStorage.autoSaveStateLoaded = autoSaveStateLoadedCallbackPtr;

		// assign wrapped functions
		if (alarmCallbackPtr)
			callbacks.alarm = wrapped_alarm;
		if (coreCrashedCallbackPtr)
			callbacks.coreCrashed = wrapped_coreCrashed;
		if (keysReadCallbackPtr)
			callbacks.keysRead = wrapped_keysRead;
		if (saveDataUpdatedCallbackPtr)
			callbacks.savedataUpdated = wrapped_savedataUpdated;
		if (videoFrameEndedCallbackPtr)
			callbacks.videoFrameEnded = wrapped_videoFrameEnded;
		if (videoFrameStartedCallbackPtr)
			callbacks.videoFrameStarted = wrapped_videoFrameStarted;

		renderer->core->addCoreCallbacks(renderer->core, &callbacks);
	}
}

EMSCRIPTEN_KEEPALIVE void setIntegerCoreSetting(char* settingName, int value) {
	// set renderer settings
	if (strcmp(settingName, "audioSampleRate") == 0 && value >= 0) {
		renderer->audio.sampleRate = value;
	} else if (strcmp(settingName, "audioBufferSize") == 0 && value >= 0) {
		renderer->audio.samples = value;
	} else if (strcmp(settingName, "videoSync") == 0 && (value == true || value == false)) {
		renderer->videoSync = value;
	} else if (strcmp(settingName, "audioSync") == 0 && (value == true || value == false)) {
		renderer->audioSync = value;
	} else if (strcmp(settingName, "threadedVideo") == 0 && (value == true || value == false)) {
		renderer->threadedVideo = value;
	} else if (strcmp(settingName, "rewindEnable") == 0 && (value == true || value == false)) {
		renderer->rewindEnable = value;
	} else if (strcmp(settingName, "rewindBufferCapacity") == 0 && value > 0) {
		renderer->rewindBufferCapacity = value;
	} else if (strcmp(settingName, "rewindBufferInterval") == 0 && value > 0) {
		renderer->rewindBufferInterval = value;
	} else if (strcmp(settingName, "frameSkip") == 0 && value >= 0) {
		renderer->frameSkip = value;
	} else if (strcmp(settingName, "baseFpsTarget") == 0 && value >= 0) {
		renderer->baseFpsTarget = value;
	} else if (strcmp(settingName, "timestepSync") == 0 && (value == true || value == false)) {
		renderer->timestepSync = value;
	} else if (strcmp(settingName, "showFpsCounter") == 0 && (value == true || value == false)) {
		renderer->showFpsCounter = value;
	} else if (strcmp(settingName, "autoSaveStateEnable") == 0 && (value == true || value == false)) {
		renderer->autoSaveStateEnable = value;
	} else if (strcmp(settingName, "restoreAutoSaveStateOnLoad") == 0 && (value == true || value == false)) {
		renderer->restoreAutoSaveStateOnLoad = value;
	} else if (strcmp(settingName, "autoSaveStateTimerIntervalSeconds") == 0 && value > 0) {
		renderer->autoSaveStateTimer.intervalSeconds = value;
	}

	// core settings when running
	if (renderer->core) {
		if (strcmp(settingName, "allowOpposingDirections") == 0 && (value == true || value == false)) {
			mCoreConfigSetDefaultIntValue(&renderer->core->config, "allowOpposingDirections", value);
			renderer->core->reloadConfigOption(renderer->core, "allowOpposingDirections", &renderer->core->config);
		} else if (strcmp(settingName, "rewindBufferCapacity") == 0 && value > 0) {
			renderer->core->opts.rewindBufferCapacity = value;
			mCoreThreadRewindParamsChanged(renderer->thread);
			mCoreConfigSetDefaultIntValue(&renderer->core->config, "rewindBufferCapacity", value);
			renderer->core->reloadConfigOption(renderer->core, "rewindBufferCapacity", &renderer->core->config);
		} else if (strcmp(settingName, "rewindBufferInterval") == 0 && value > 0) {
			renderer->core->opts.rewindBufferInterval = value;
			mCoreConfigSetDefaultIntValue(&renderer->core->config, "rewindBufferInterval", value);
			renderer->core->reloadConfigOption(renderer->core, "rewindBufferInterval", &renderer->core->config);
		} else if (strcmp(settingName, "frameSkip") == 0 && value >= 0) {
			mCoreConfigSetDefaultIntValue(&renderer->core->config, "frameskip", renderer->frameSkip);
			renderer->core->reloadConfigOption(renderer->core, "frameskip", &renderer->core->config);
		} else if (strcmp(settingName, "threadedVideo") == 0 && (value == true || value == false)) {
			mCoreConfigSetDefaultIntValue(&renderer->core->config, "threadedVideo", value);
			renderer->core->reloadConfigOption(renderer->core, "threadedVideo", &renderer->core->config);
		} else if (strcmp(settingName, "rewindEnable") == 0 && (value == true || value == false)) {
			renderer->core->opts.rewindEnable = value;
			mCoreThreadRewindParamsChanged(renderer->thread);
			mCoreConfigSetDefaultIntValue(&renderer->core->config, "rewindEnable", value);
			renderer->core->reloadConfigOption(renderer->core, "rewindEnable", &renderer->core->config);
		} else if (strcmp(settingName, "baseFpsTarget") == 0 && value >= 0.0) {
			renderer->thread->impl->sync.fpsTarget = (double) value * renderer->fastForwardMultiplier;
			mCoreConfigSetDefaultFloatValue(&renderer->core->config, "fpsTarget",
			                                (double) value * renderer->fastForwardMultiplier);
			renderer->core->reloadConfigOption(renderer->core, "fpsTarget", &renderer->core->config);
		} else if (strcmp(settingName, "timestepSync") == 0 && (value == true || value == false)) {
			mCoreConfigSetDefaultIntValue(&renderer->core->config, "timestepSync", value);
			renderer->core->reloadConfigOption(renderer->core, "timestepSync", &renderer->core->config);
		}

		// thread settings when running
		if (renderer->thread && (value == true || value == false)) {
			mCoreThreadInterrupt(renderer->thread);
			if (strcmp(settingName, "videoSync") == 0) {
				renderer->thread->impl->sync.videoFrameWait = value;
			} else if (strcmp(settingName, "audioSync") == 0) {
				renderer->thread->impl->sync.audioWait = value;
			}
			mCoreThreadContinue(renderer->thread);
		}
	}
}

/*
 * Rendering utilities and emscripten main rendering loop
 */

// keypress utilities
static void handleKeypressCore(const struct SDL_KeyboardEvent* event) {
	if (event->keysym.sym == SDLK_f && renderer->fastForwardMultiplier == 1) {
		updateFastForward(event->type == SDL_KEYDOWN ? 2 : 1);
		return;
	}
	if (event->keysym.sym == SDLK_r) {
		mCoreThreadSetRewinding(renderer->thread, event->type == SDL_KEYDOWN);
		return;
	}
	int key = -1;
	if (!(event->keysym.mod & ~(KMOD_NUM | KMOD_CAPS))) {
		key = mInputMapKey(&renderer->core->inputMap, SDL_BINDING_KEY, event->keysym.sym);
	}
	if (key != -1) {
		if (event->type == SDL_KEYDOWN) {
			renderer->core->addKeys(renderer->core, 1 << key);
		} else {
			renderer->core->clearKeys(renderer->core, 1 << key);
		}
	}
}

// fps utilities
void updateFPS() {
	double now = emscripten_get_now();

	if (renderer->fpsCounter.lastTime == 0) {
		renderer->fpsCounter.lastTime = now;
		return;
	}

	renderer->fpsCounter.frames++;

	double elapsed = now - renderer->fpsCounter.lastTime;
	if (elapsed >= 1000.0) { // every ~1 second
		renderer->fpsCounter.fps = (double) renderer->fpsCounter.frames * 1000.0 / elapsed;
		renderer->fpsCounter.frames = 0;
		renderer->fpsCounter.lastTime = now;
	}
}

void drawFPS(unsigned x, unsigned y) {
	char fpsBuf[32];
	snprintf(fpsBuf, sizeof(fpsBuf), "%.1f", renderer->fpsCounter.fps);

	int scale = 1;
	int charWidth = 8 * scale;
	int charHeight = 10 * scale;
	int width = strlen(fpsBuf) * charWidth;
	int height = charHeight;

	// save current draw color
	Uint8 prevR, prevG, prevB, prevA;
	SDL_GetRenderDrawColor(renderer->sdlRenderer, &prevR, &prevG, &prevB, &prevA);

	// draw gray background
	SDL_SetRenderDrawColor(renderer->sdlRenderer, 64, 64, 64, 255);
	SDL_FRect bgRect = { x - 2, y - 2, width + 4, height + 4 };
	SDL_RenderFillRectF(renderer->sdlRenderer, &bgRect);

	// draw white text
	SDL_SetRenderDrawColor(renderer->sdlRenderer, 255, 255, 255, 255);
	SDL_RenderText(renderer->sdlRenderer, fpsBuf, scale, x, y);

	// restore original draw color
	SDL_SetRenderDrawColor(renderer->sdlRenderer, prevR, prevG, prevB, prevA);
}

// auto save state utilities
void updateAutoSaveState() {
	double now = emscripten_get_now();

	if (renderer->autoSaveStateTimer.lastTime == 0) {
		renderer->autoSaveStateTimer.lastTime = now;
		return;
	}

	double elapsed = now - renderer->autoSaveStateTimer.lastTime;
	if (elapsed >= renderer->autoSaveStateTimer.intervalSeconds * 1000) {
		bool successfulAutoSave = autoSaveState();
		if (successfulAutoSave) {
			renderer->autoSaveStateTimer.lastTime = now;
			if (callbackStorage.autoSaveStateCaptured)
				callbackStorage.autoSaveStateCaptured(NULL);
		}
	}
}

// emscripten main run loop
void runLoop() {
	union SDL_Event event;

	if (renderer->core) {
		if (!renderer->thread) {
			renderer->thread = malloc(sizeof(struct mCoreThread));
			memset(renderer->thread, 0, sizeof(struct mCoreThread));

			renderer->thread->core = renderer->core;
			bool didFail = !mCoreThreadStart(renderer->thread);
			if (didFail)
				EM_ASM({ console.error("thread instantiation failed") });

			mSDLInitAudio(&renderer->audio, renderer->thread);
			mSDLResumeAudio(&renderer->audio);
			updateFastForward(renderer->fastForwardMultiplier);

			if (renderer->restoreAutoSaveStateOnLoad) {
				bool successfulLoad = loadAutoSaveState();
				if (successfulLoad && callbackStorage.autoSaveStateLoaded)
					callbackStorage.autoSaveStateLoaded(NULL);
			}
		}

		if (mCoreThreadIsActive(renderer->thread)) {
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_KEYDOWN:
				case SDL_KEYUP:
					if (renderer->core && renderer->thread) {
						handleKeypressCore(&event.key);
					}
					break;
				};
			}
			if (mCoreSyncWaitFrameStart(&renderer->thread->impl->sync)) {
				unsigned w, h;
				renderer->core->currentVideoSize(renderer->core, &w, &h);

				SDL_Rect rect = { .x = 0, .y = 0, .w = w, .h = h };

				SDL_UnlockTexture(renderer->sdlTex);
				SDL_RenderCopy(renderer->sdlRenderer, renderer->sdlTex, &rect, &rect);
				if (renderer->showFpsCounter)
					drawFPS(w - 35, h - LINE_HEIGHT);
				SDL_RenderPresent(renderer->sdlRenderer);
				int stride;
				SDL_LockTexture(renderer->sdlTex, 0, (void**) &renderer->outputBuffer, &stride);
				renderer->core->setVideoBuffer(renderer->core, renderer->outputBuffer, stride / BYTES_PER_PIXEL);
			}
			mCoreSyncWaitFrameEnd(&renderer->thread->impl->sync);
		}
		if (renderer->showFpsCounter)
			updateFPS();
		if (renderer->autoSaveStateEnable)
			updateAutoSaveState();
	} else {
		// dont run the main loop if there is no core,  we don't
		// want to handle events unless the core is running for now
		emscripten_pause_main_loop();
	}
}

/*
 * Initialization and main method (entrypoint)
 */

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
			       gitCommit : UTF8ToString($0),
			       gitShort : UTF8ToString($1),
			       gitBranch : UTF8ToString($2),
			       gitRevision : $3,
			       binaryName : UTF8ToString($4),
			       projectName : UTF8ToString($5),
			       projectVersion : UTF8ToString($6)
		       };
	       }),
	       gitCommit, gitCommitShort, gitBranch, gitRevision, binaryName, projectName, projectVersion);
}

CONSTRUCTOR(premain) {
	setupConstants();
}

int excludeKeys(void* userdata, SDL_Event* event) {
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
	renderer = malloc(sizeof(struct mEmscriptenRenderer));
	memset(renderer, 0, sizeof(struct mEmscriptenRenderer));

	renderer->audio.sampleRate = 48000;
	renderer->audio.samples = 1024;
	renderer->baseFpsTarget = 60.0;
	renderer->rewindBufferCapacity = 600;
	renderer->rewindBufferInterval = 1;
	renderer->fastForwardMultiplier = 1;
	renderer->videoSync = false;
	renderer->audioSync = false;
	renderer->timestepSync = true;
	renderer->threadedVideo = false;
	renderer->rewindEnable = true;
	renderer->showFpsCounter = false;
	renderer->fpsCounter.lastTime = 0;
	renderer->fpsCounter.fps = 0;
	renderer->fpsCounter.frames = 0;
	renderer->autoSaveStateEnable = true;
	renderer->restoreAutoSaveStateOnLoad = true;
	renderer->autoSaveStateTimer.lastTime = 0;
	renderer->autoSaveStateTimer.intervalSeconds = 30;

	mLogSetDefaultLogger(&logCtx);

	SDL_Init(SDL_INIT_VIDEO);
	renderer->window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                                    GBA_VIDEO_HORIZONTAL_PIXELS, GBA_VIDEO_VERTICAL_PIXELS, SDL_WINDOW_OPENGL);
	renderer->sdlRenderer =
	    SDL_CreateRenderer(renderer->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	// exclude specific key events
	SDL_SetEventFilter(excludeKeys, NULL);

	emscripten_set_main_loop(runLoop, 0, 1);

	free(renderer);
	return 0;
}
