/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <mgba/internal/gba/extra/audio-mixer.h>

#include <mgba/internal/gba/gba.h>

static void _mks4agbInit(void* cpu, struct mCPUComponent* component);
static void _mks4agbDeinit(struct mCPUComponent* component);

static bool _mks4agbEngage(struct GBAAudioMixer* mixer, uint32_t address);
static void _mks4agbStep(struct GBAAudioMixer* mixer);

void GBAAudioMixerCreate(struct GBAAudioMixer* mixer) {
	mixer->d.init = _mks4agbInit;
	mixer->d.deinit = _mks4agbDeinit;
	mixer->engage = _mks4agbEngage;
	mixer->step = _mks4agbStep;
}

void _mks4agbInit(void* cpu, struct mCPUComponent* component) {
	struct ARMCore* arm = cpu;
	struct GBA* gba = (struct GBA*) arm->master;
	struct GBAAudioMixer* mixer = (struct GBAAudioMixer*) component;
	gba->audio.mixer = mixer;
	mixer->p = &gba->audio;
	mixer->contextAddress = 0;
	memset(&mixer->context, 0, sizeof(mixer->context));
}

void _mks4agbDeinit(struct mCPUComponent* component) {
	// TODO
}

static void _mks4agbReload(struct GBAAudioMixer* mixer) {
	struct ARMCore* cpu = mixer->p->p->cpu;
	struct ARMMemory* memory = &cpu->memory;
	mixer->context.magic = cpu->memory.load32(cpu, mixer->contextAddress + offsetof(struct GBAMKS4AGBContext, magic), 0);
	mixer->context.musicPlayer = cpu->memory.load32(cpu, mixer->contextAddress + offsetof(struct GBAMKS4AGBContext, musicPlayer), 0);
}

bool _mks4agbEngage(struct GBAAudioMixer* mixer, uint32_t address) {
	mixer->contextAddress = address;
	_mks4agbReload(mixer);
	return false;
}

void _mks4agbStep(struct GBAAudioMixer* mixer) {
	_mks4agbReload(mixer);
}
