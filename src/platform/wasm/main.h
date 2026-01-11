/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platform/sdl/sdl-audio.h"
#include "platform/sdl/sdl-events.h"

#include <mgba/core/thread.h>

typedef struct {
	double lastTime;
	double fps;
	int frames;
} FPSCounter;

typedef struct {
	double lastTime;
	int intervalSeconds;
} AutoSaveStateTimer;

// represents global items used in rendering mGBA in the wasm platform
struct mEmscriptenRenderer {
	struct mCore* core;
	mColor* outputBuffer;

	SDL_Window* window;
	SDL_Texture* sdlTex;
	SDL_Renderer* sdlRenderer;
	FPSCounter fpsCounter;
	AutoSaveStateTimer autoSaveStateTimer;

	struct mSDLAudio audio;
	struct mCoreThread* thread;

	// persistent options for the core at runtime, limited
	// subset of mCoreOptions and custom functionality
	double fastForwardMultiplier;
	int frameSkip;
	int baseFpsTarget;
	int rewindBufferCapacity;
	int rewindBufferInterval;
	bool videoSync;
	bool audioSync;
	bool timestepSync;
	bool threadedVideo;
	bool rewindEnable;
	bool showFpsCounter;
	bool restoreAutoSaveStateOnLoad;
	bool autoSaveStateEnable;
};