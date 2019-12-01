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

static void handleKeypress(const struct SDL_KeyboardEvent* event) {
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
			handleKeypress(&event.key);
			break;
		};
	}
	if (core) {
		core->runFrame(core);

		SDL_UnlockTexture(tex);
		SDL_RenderCopy(renderer, tex, 0, 0);
		SDL_RenderPresent(renderer);

		int stride;
		SDL_LockTexture(tex, 0, (void**) &buffer, &stride);
		core->setVideoBuffer(core, buffer, stride / BYTES_PER_PIXEL);
		return;
	}
}

EMSCRIPTEN_KEEPALIVE bool loadGame(void) {
	if (core) {
		core->deinit(core);
		core = NULL;
	}
	core = mCoreFind("rom.bin");
	if (!core) {
		return false;
	}
	core->init(core);

	mCoreLoadFile(core, "rom.bin");
	mCoreConfigInit(&core->config, "wasm");
	mInputMapInit(&core->inputMap, &GBAInputInfo);
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
	SDL_SetWindowSize(window, w, h);
	audio.core = core;
	mSDLResumeAudio(&audio);
	return true;
}

int main() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
	window = SDL_CreateWindow(projectName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 16, 16, SDL_WINDOW_OPENGL);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	mSDLInitAudio(&audio, NULL);
	emscripten_set_main_loop(testLoop, 0, 1);
	return 0;
}