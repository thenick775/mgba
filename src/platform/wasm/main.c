/* Copyright (c) 2013-2019 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <mgba/core/core.h>
#include <mgba/core/version.h>
#include <mgba/internal/gba/input.h>
#include <mgba-util/vfs.h>

#include "platform/sdl/sdl-audio.h"
#include "platform/sdl/sdl-events.h"

#include <emscripten.h>
#include <SDL2/SDL.h>

static struct mCore* core = NULL;
static color_t* buffer = NULL;
static struct mSDLAudio audio = {
	.sampleRate = 48000,
	.samples = 512
};

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* tex = NULL;

static void _log(struct mLogger*, int category, enum mLogLevel level, const char* format, va_list args);
static struct mLogger logCtx = { .log = _log };

static void handleKeypressCore(const struct SDL_KeyboardEvent* event) {
	if (event->keysym.sym == SDLK_TAB) {
		emscripten_set_main_loop_timing(event->type == SDL_KEYDOWN ? EM_TIMING_SETTIMEOUT : EM_TIMING_RAF, 0);
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

static void handleKeypress(const struct SDL_KeyboardEvent* event) {
	UNUSED(event);
	// Nothing here yet
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
			handleKeypress(&event.key);
			break;
		};
	}
	if (core) {
		core->runFrame(core);

		unsigned w, h;
		core->desiredVideoDimensions(core, &w, &h);

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
	}
}

EMSCRIPTEN_KEEPALIVE bool loadGame(const char* name) {
	printf("%p\n", name);
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

	mCoreLoadFile(core, name);
	mCoreConfigInit(&core->config, "wasm");
	mInputMapInit(&core->inputMap, &GBAInputInfo);
	mDirectorySetMapOptions(&core->dirs, &core->opts);
	mCoreAutoloadSave(core);
	mSDLInitBindingsGBA(&core->inputMap);

	unsigned w, h;
	core->desiredVideoDimensions(core, &w, &h);
	if (tex) {
		SDL_DestroyTexture(tex);
	}
	tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, w, h);

	int stride;
	SDL_LockTexture(tex, 0, (void**) &buffer, &stride);
	core->setVideoBuffer(core, buffer, stride / BYTES_PER_PIXEL);

	core->reset(core);

	core->desiredVideoDimensions(core, &w, &h);
	SDL_SetWindowSize(window, w, h);
	audio.core = core;
	mSDLResumeAudio(&audio);
	return true;
}

void _log(struct mLogger* logger, int category, enum mLogLevel level, const char* format, va_list args) {
	UNUSED(logger);
	UNUSED(category);
	UNUSED(level);
	UNUSED(format);
	UNUSED(args);
}


EMSCRIPTEN_KEEPALIVE void setupConstants(void) {
	EM_ASM({
		mGBA.version.gitCommit = UTF8ToString($0);
		mGBA.version.gitShort = UTF8ToString($1);
		mGBA.version.gitBranch = UTF8ToString($2);
		mGBA.version.gitRevision = $3;
		mGBA.version.binaryName = UTF8ToString($4);
		mGBA.version.projectName = UTF8ToString($5);
		mGBA.version.projectVersion = UTF8ToString($6);
	}, gitCommit, gitCommitShort, gitBranch, gitRevision, binaryName, projectName, projectVersion);
}

EM_JS(void, jsSetup, (void), {
	mGBA = {
		loadFile: cwrap('loadGame', 'number', ['string']),
		version: {}
	}
});

CONSTRUCTOR(premain) {
	jsSetup();
	setupConstants();
}

int main() {
	mLogSetDefaultLogger(&logCtx);

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
	window = SDL_CreateWindow(projectName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 16, 16, SDL_WINDOW_OPENGL);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	mSDLInitAudio(&audio, NULL);

	EM_ASM(
		FS.mkdir('/data');
		FS.mount(IDBFS, {}, '/data');
		FS.mkdir('/data/saves');
		FS.mkdir('/data/states');
		FS.syncfs(true, function (err) {});
	);
	emscripten_set_main_loop(testLoop, 0, 1);
	return 0;
}
