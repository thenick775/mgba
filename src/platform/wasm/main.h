/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platform/sdl/sdl-audio.h"
#include "platform/sdl/sdl-events.h"

// represents global items used in rendering mGBA in the wasm platform
struct mEmscriptenRenderer {
	struct mCore* core;
	color_t* outputBuffer;

	SDL_Window* window;
	SDL_Texture* sdlTex;
	SDL_Renderer* sdlRenderer;

	struct mSDLAudio audio;

	// fps related variables
	double lastNow;
	int fastForwardSpeed;
	bool renderFirstFrame;
};