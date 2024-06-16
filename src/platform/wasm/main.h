/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platform/sdl/sdl-audio.h"
#include "platform/sdl/sdl-events.h"

#include <mgba/core/thread.h>

struct mEmscriptenRenderer {
	struct mCore* core;
	color_t* outputBuffer;

	struct mSDLAudio audio;

	struct mCoreThread* thread;

	bool (*init)(struct mEmscriptenRenderer* renderer);
	void (*runloop)(struct mEmscriptenRenderer* renderer, void* thread);
	void (*deinit)(struct mEmscriptenRenderer* renderer);

	SDL_Window* window;
	SDL_Texture* sdlTex;
	SDL_Renderer* sdlRenderer;

	// fps related variables
	Uint32 emscriptenLastTick;
	bool renderFirstFrame;
	int fastForwardSpeed;
};
