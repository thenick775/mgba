/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <mgba/internal/gba/extra/audio-mixer.h>

#include <mgba/core/blip_buf.h>
#include <mgba/internal/gba/gba.h>

static void _mp2kInit(void* cpu, struct mCPUComponent* component);
static void _mp2kDeinit(struct mCPUComponent* component);

static bool _mp2kEngage(struct GBAAudioMixer* mixer, uint32_t address);
static void _mp2kVblank(struct GBAAudioMixer* mixer);
static void _mp2kStep(struct GBAAudioMixer* mixer);

static const float FACTOR = 1.f / 42134400.f; // VIDEO_TOTAL_LENGTH * 150

static const uint8_t _duration[0x30] = {
	01,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
	28, 30, 32, 36, 40, 42, 44, 48, 52, 54, 56, 60,
	64, 66, 68, 72, 76, 78, 80, 84, 88, 90, 92, 96
};

void GBAAudioMixerCreate(struct GBAAudioMixer* mixer) {
	mixer->d.init = _mp2kInit;
	mixer->d.deinit = _mp2kDeinit;
	mixer->engage = _mp2kEngage;
	mixer->vblank = _mp2kVblank;
	mixer->step = _mp2kStep;
}

void _mp2kInit(void* cpu, struct mCPUComponent* component) {
	struct ARMCore* arm = cpu;
	struct GBA* gba = (struct GBA*) arm->master;
	struct GBAAudioMixer* mixer = (struct GBAAudioMixer*) component;
	gba->audio.mixer = mixer;
	mixer->p = &gba->audio;
	mixer->contextAddress = 0;
	mixer->tempoI = 1.f / 120.f;
	mixer->frame = 0;
	memset(&mixer->context, 0, sizeof(mixer->context));
	memset(&mixer->activeTracks, 0, sizeof(mixer->activeTracks));
}

void _mp2kDeinit(struct mCPUComponent* component) {
	struct GBAAudioMixer* mixer = (struct GBAAudioMixer*) component;
}

static void _loadInstrument(struct ARMCore* cpu, struct GBAMP2kInstrument* instrument, uint32_t base) {
	struct ARMMemory* memory = &cpu->memory;
	instrument->type = memory->load8(cpu, base + offsetof(struct GBAMP2kInstrument, type), 0);
	instrument->key = memory->load8(cpu, base + offsetof(struct GBAMP2kInstrument, key), 0);
	instrument->length = memory->load8(cpu, base + offsetof(struct GBAMP2kInstrument, length), 0);
	instrument->pan = memory->load8(cpu, base + offsetof(struct GBAMP2kInstrument, pan), 0);
	if (instrument->type == 0x40 || instrument->type == 0x80) {
		instrument->subTable = memory->load32(cpu, base + offsetof(struct GBAMP2kInstrument, subTable), 0);
		instrument->map = memory->load32(cpu, base + offsetof(struct GBAMP2kInstrument, map), 0);
	} else {
		instrument->waveData = memory->load32(cpu, base + offsetof(struct GBAMP2kInstrument, waveData), 0);
		instrument->adsr.attack = memory->load8(cpu, base + offsetof(struct GBAMP2kInstrument, adsr.attack), 0);
		instrument->adsr.decay = memory->load8(cpu, base + offsetof(struct GBAMP2kInstrument, adsr.decay), 0);
		instrument->adsr.sustain = memory->load8(cpu, base + offsetof(struct GBAMP2kInstrument, adsr.sustain), 0);
		instrument->adsr.release = memory->load8(cpu, base + offsetof(struct GBAMP2kInstrument, adsr.release), 0);
	}
}

static void _lookupInstrument(struct ARMCore* cpu, struct GBAMP2kInstrument* instrument, uint8_t key) {
	struct ARMMemory* memory = &cpu->memory;
	if (instrument->type == 0x40) {
		uint32_t subInstrumentBase = instrument->subTable;
		uint32_t keyTable = instrument->map;
		uint8_t id = memory->load8(cpu, keyTable + key, 0);
		subInstrumentBase += 12 * id;
		_loadInstrument(cpu, instrument, subInstrumentBase);
	}
}

static void _stepSample(struct GBAAudioMixer* mixer, struct GBAMP2kTrack* track) {
	struct ARMCore* cpu = mixer->p->p->cpu;
	struct ARMMemory* memory = &cpu->memory;
	uint32_t headerAddress;
	struct GBAMP2kInstrument instrument = track->track.instrument;

	uint8_t note = track->lastNote;
	_lookupInstrument(cpu, &instrument, note);
	switch (instrument.type) {
	case 0x00:
	case 0x08:
	case 0x40:
	case 0x80:
		break;
	default:
		// We don't care about PSG channels
		return;
	}
	headerAddress = instrument.waveData;
	if (headerAddress < 0x20) {
		mLOG(GBA_AUDIO, ERROR, "Audio track has invalid instrument");
		return;
	}
	uint32_t pitch = memory->load32(cpu, headerAddress + 0x4, 0);
	uint32_t loopOffset = memory->load32(cpu, headerAddress + 0x8, 0);
	uint32_t endOffset = memory->load32(cpu, headerAddress + 0xC, 0);
	uint32_t sampleBase = headerAddress + 0x10;
	endOffset += sampleBase;

	if (track->samplePlaying) {
		sampleBase = track->samplePlaying;
	}

	struct GBAStereoSample lastSample = track->lastSample;
	float distance = 1024.0f / (pitch * powf(2.0f, (note - 60) / 12.0f));
	float base;
	for (base = track->currentOffset; base < mixer->tempoI; base += distance) {
		int8_t sample = memory->load8(cpu, sampleBase, 0);
		blip_add_delta(mixer->p->psg.left, mixer->p->clock + base * GBA_ARM7TDMI_FREQUENCY, ((sample * track->track.volML) >> 1) - lastSample.left);
		blip_add_delta(mixer->p->psg.right, mixer->p->clock + base * GBA_ARM7TDMI_FREQUENCY, ((sample * track->track.volMR) >> 1) - lastSample.right);
		lastSample.left = (sample * track->track.volML) >> 1;
		lastSample.right = (sample * track->track.volMR) >> 1;
		++sampleBase;
		if (sampleBase >= endOffset) {
			sampleBase = headerAddress + loopOffset + 0x10;
		}
	}

	track->lastSample = lastSample;
	track->samplePlaying = sampleBase;
	track->currentOffset = base - mixer->tempoI;
}

static int _playNote(struct GBAAudioMixer* mixer, struct GBAMP2kTrack* track, uint32_t base) {
	struct ARMCore* cpu = mixer->p->p->cpu;
	struct ARMMemory* memory = &cpu->memory;
	int8_t arguments[3] = {
		track->lastNote
	};
	int nArgs = 0;
	do {
		arguments[0] = memory->load8(cpu, base, 0);
		if (arguments[0] < 0) {
			break;
		}
		++nArgs;
		arguments[1] = memory->load8(cpu, base + 1, 0);
		if (arguments[1] < 0) {
			break;
		}
		++nArgs;
		arguments[2] = memory->load8(cpu, base + 2, 0);
		if (arguments[2] < 0) {
			break;
		}
		++nArgs;
	} while (false);
	if (arguments[0] >= 0) {
		track->lastNote = arguments[0];
	}
	mLOG(GBA_AUDIO, DEBUG, "Playing note %02X", track->lastNote);
	_stepSample(mixer, track);
	return nArgs;
}

static uint32_t _runCommand(struct GBAAudioMixer* mixer, size_t channelId, uint8_t command, uint32_t base) {
	struct ARMCore* cpu = mixer->p->p->cpu;
	struct ARMMemory* memory = &cpu->memory;
	struct GBAMP2kTrack* track = &mixer->activeTracks[channelId];
	uint32_t address;
	int nArgs = 0;

	switch (command) {
	case 0x80:
		// NOP
		break;
	case 0xB1:
		// FINE
		mLOG(GBA_AUDIO, DEBUG, "FINE");
		track->notePlaying = 0;
		break;
	case 0xB2:
		// GOTO
		address = memory->load8(cpu, base + 1, 0);
		address |= memory->load8(cpu, base + 2, 0) << 8;
		address |= memory->load8(cpu, base + 3, 0) << 16;
		address |= memory->load8(cpu, base + 4, 0) << 24;
		mLOG(GBA_AUDIO, DEBUG, "GOTO %08X", address);
		return address;
	case 0xB3:
		// PATT
		address = memory->load8(cpu, base + 1, 0);
		address |= memory->load8(cpu, base + 2, 0) << 8;
		address |= memory->load8(cpu, base + 3, 0) << 16;
		address |= memory->load8(cpu, base + 4, 0) << 24;
		nArgs = 4;
		mLOG(GBA_AUDIO, DEBUG, "PATT %08X", address);
		break;
	case 0xB4:
		mLOG(GBA_AUDIO, DEBUG, "PEND");
		break;
	case 0xB5:
		mLOG(GBA_AUDIO, DEBUG, "REPT");
		break;
	case 0xB6:
	case 0xB7:
	case 0xB8:
	case 0xB9:
	case 0xC6:
	case 0xC7:
	case 0xC9:
	case 0xCA:
	case 0xCB:
	case 0xCD:
		mLOG(GBA_AUDIO, DEBUG, "Unknown command, reseting");
		track->lastCommand = command;
		mixer->p->externalMixing = false;
		return 0;
	case 0xBA:
		mLOG(GBA_AUDIO, DEBUG, "PRIO");
		break;
	case 0xBB:
		mLOG(GBA_AUDIO, DEBUG, "TEMPO");
		mixer->tempoI = 0.5f / memory->load8(cpu, base + 1, 0);
		nArgs = 1;
		break;
	case 0xBC:
		mLOG(GBA_AUDIO, DEBUG, "KEYSH");
		nArgs = 1;
		break;
	case 0xBD:
		mLOG(GBA_AUDIO, DEBUG, "VOICE");
		nArgs = 1;
		break;
	case 0xBE:
		mLOG(GBA_AUDIO, DEBUG, "VOL");
		track->lastCommand = command;
		nArgs = 1;
		break;
	case 0xBF:
		mLOG(GBA_AUDIO, DEBUG, "PAN");
		track->lastCommand = command;
		nArgs = 1;
		break;
	case 0xC0:
		mLOG(GBA_AUDIO, DEBUG, "BEND");
		track->lastCommand = command;
		nArgs = 1;
		break;
	case 0xC1:
		mLOG(GBA_AUDIO, DEBUG, "BENDR");
		track->lastCommand = command;
		nArgs = 1;
		break;
	case 0xC2:
		mLOG(GBA_AUDIO, DEBUG, "LFOS");
		nArgs = 1;
		break;
	case 0xC3:
		mLOG(GBA_AUDIO, DEBUG, "LFODL");
		nArgs = 1;
		break;
	case 0xC4:
		mLOG(GBA_AUDIO, DEBUG, "MOD");
		nArgs = 1;
		track->lastCommand = command;
		break;
	case 0xC5:
		mLOG(GBA_AUDIO, DEBUG, "MODT");
		nArgs = 1;
		break;
	case 0xC8:
		mLOG(GBA_AUDIO, DEBUG, "TUNE");
		nArgs = 1;
		break;
	case 0xCC:
		mLOG(GBA_AUDIO, DEBUG, "PORT");
		nArgs = 1;
		break;
	case 0xCE:
		mLOG(GBA_AUDIO, DEBUG, "ENDTIE");
		track->notePlaying = 0;
		track->lastCommand = command;
		return 0;
	case 0xCF:
		mLOG(GBA_AUDIO, DEBUG, "TIE");
		track->notePlaying = -1;
		nArgs += _playNote(mixer, track, base + 1);
		track->lastCommand = command;
		break;
	default:
		if (command >= 0xD0) {
			track->lastCommand = command;
			track->notePlaying = _duration[command - 0xD0];
			nArgs += _playNote(mixer, track, base + 1);
		}
		if (command < 0x80) {
			mLOG(GBA_AUDIO, DEBUG, "Argument");
			if (track->lastCommand < 0x80) {
				break;
			}
			return _runCommand(mixer, channelId, track->lastCommand, base - 1);
		}
		if (command <= 0xB0) {
			mLOG(GBA_AUDIO, DEBUG, "Waiting: %02X", _duration[command - 0x81]);
			return 0;
		}
		break;
	}
	return base + nArgs + 1;
}

static void _stepTrack(struct GBAAudioMixer* mixer, size_t channelId) {
	struct ARMCore* cpu = mixer->p->p->cpu;
	struct ARMMemory* memory = &cpu->memory;
	struct GBAMP2kTrack* track = &mixer->activeTracks[channelId];

	uint32_t base = track->track.cmdPtr;
	if (base < 0x20) {
		return;
	}
	while (true) {
		uint8_t command = memory->load8(cpu, base, 0);

		mLOG(GBA_AUDIO, DEBUG, "Updating track for channel %zu, command %02X", channelId, command);
		uint32_t newAddress = _runCommand(mixer, channelId, command, base);
		if (newAddress == 0) {
			++base;
			break;
		}
		base = newAddress;
	}
}

static void _mp2kReload(struct GBAAudioMixer* mixer) {
	struct ARMCore* cpu = mixer->p->p->cpu;
	struct ARMMemory* memory = &cpu->memory;
	mixer->context.magic = memory->load32(cpu, mixer->contextAddress + offsetof(struct GBAMP2kContext, magic), 0);
	int i;
	for (i = 0; i < MP2K_MAX_SOUND_CHANNELS; ++i) {
		struct GBAMP2kSoundChannel* ch = &mixer->context.chans[i];
		struct GBAMP2kTrack* track = &mixer->activeTracks[i];
		track->waiting = false;
		uint32_t base = mixer->contextAddress + offsetof(struct GBAMP2kContext, chans[i]);

		ch->status = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, status), 0);
		ch->type = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, type), 0);
		ch->rightVolume = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, rightVolume), 0);
		ch->leftVolume = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, leftVolume), 0);
		ch->adsr.attack = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, adsr.attack), 0);
		ch->adsr.decay = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, adsr.decay), 0);
		ch->adsr.sustain = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, adsr.sustain), 0);
		ch->adsr.release = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, adsr.release), 0);
		ch->ky = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, ky), 0);
		ch->envelopeV = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, envelopeV), 0);
		ch->envelopeRight = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, envelopeRight), 0);
		ch->envelopeLeft = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, envelopeLeft), 0);
		ch->echoVolume = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, echoVolume), 0);
		ch->echoLength = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, echoLength), 0);
		ch->d1 = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, d1), 0);
		ch->d2 = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, d2), 0);
		ch->gt = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, gt), 0);
		ch->midiKey = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, midiKey), 0);
		ch->ve = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, ve), 0);
		ch->pr = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, pr), 0);
		ch->rp = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, rp), 0);
		ch->d3[0] = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, d3[0]), 0);
		ch->d3[1] = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, d3[1]), 0);
		ch->d3[2] = memory->load8(cpu, base + offsetof(struct GBAMP2kSoundChannel, d3[2]), 0);
		ch->ct = memory->load32(cpu, base + offsetof(struct GBAMP2kSoundChannel, ct), 0);
		ch->fw = memory->load32(cpu, base + offsetof(struct GBAMP2kSoundChannel, fw), 0);
		ch->freq = memory->load32(cpu, base + offsetof(struct GBAMP2kSoundChannel, freq), 0);
		ch->waveData = memory->load32(cpu, base + offsetof(struct GBAMP2kSoundChannel, waveData), 0);
		ch->cp = memory->load32(cpu, base + offsetof(struct GBAMP2kSoundChannel, cp), 0);
		ch->track = memory->load32(cpu, base + offsetof(struct GBAMP2kSoundChannel, track), 0);
		ch->pp = memory->load32(cpu, base + offsetof(struct GBAMP2kSoundChannel, pp), 0);
		ch->np = memory->load32(cpu, base + offsetof(struct GBAMP2kSoundChannel, np), 0);
		ch->d4 = memory->load32(cpu, base + offsetof(struct GBAMP2kSoundChannel, d4), 0);
		ch->xpi = memory->load16(cpu, base + offsetof(struct GBAMP2kSoundChannel, xpi), 0);
		ch->xpc = memory->load16(cpu, base + offsetof(struct GBAMP2kSoundChannel, xpc), 0);

		base = ch->track;
		if (base) {
			track->track.flags = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, flags), 0);
			track->track.wait = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, wait), 0);
			track->track.patternLevel = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, patternLevel), 0);
			track->track.repN = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, repN), 0);
			track->track.gateTime = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, gateTime), 0);
			track->track.key = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, key), 0);
			track->track.velocity = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, velocity), 0);
			track->track.runningStatus = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, runningStatus), 0);
			track->track.keyM = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, keyM), 0);
			track->track.pitM = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, pitM), 0);
			track->track.keyShift = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, keyShift), 0);
			track->track.keyShiftX = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, keyShiftX), 0);
			track->track.tune = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, tune), 0);
			track->track.pitX = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, pitX), 0);
			track->track.bend = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, bend), 0);
			track->track.bendRange = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, bendRange), 0);
			track->track.volMR = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, volMR), 0);
			track->track.volML = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, volML), 0);
			track->track.vol = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, vol), 0);
			track->track.volX = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, volX), 0);
			track->track.pan = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, pan), 0);
			track->track.panX = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, panX), 0);
			track->track.modM = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, modM), 0);
			track->track.mod = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, mod), 0);
			track->track.modT = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, modT), 0);
			track->track.lfoSpeed = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, lfoSpeed), 0);
			track->track.lfoSpeedC = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, lfoSpeedC), 0);
			track->track.lfoDelay = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, lfoDelay), 0);
			track->track.lfoDelayC = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, lfoDelayC), 0);
			track->track.priority = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, priority), 0);
			track->track.echoVolume = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, echoVolume), 0);
			track->track.echoLength = memory->load8(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, echoLength), 0);
			track->track.chan = memory->load32(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, chan), 0);
			_loadInstrument(cpu, &track->track.instrument, base + offsetof(struct GBAMP2kMusicPlayerTrack, instrument));
			track->track.cmdPtr = memory->load32(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, cmdPtr), 0);
			track->track.patternStack[0] = memory->load32(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, patternStack[0]), 0);
			track->track.patternStack[1] = memory->load32(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, patternStack[1]), 0);
			track->track.patternStack[2] = memory->load32(cpu, base + offsetof(struct GBAMP2kMusicPlayerTrack, patternStack[2]), 0);
		} else {
			memset(&track->track, 0, sizeof(track->track));
		}
	}
}

bool _mp2kEngage(struct GBAAudioMixer* mixer, uint32_t address) {
	if (address < BASE_WORKING_RAM) {
		return false;
	}
	if (address != mixer->contextAddress) {
		mixer->contextAddress = address;
		mixer->p->externalMixing = true;
		_mp2kReload(mixer);
	}
	return true;
}

void _mp2kStep(struct GBAAudioMixer* mixer) {
	if (!mixer->p->externalMixing) {
		return;
	}
	mixer->frame += mixer->p->sampleInterval * FACTOR;
	while (mixer->frame >= mixer->tempoI) {
		int i;
		for (i = 0; i < MP2K_MAX_SOUND_CHANNELS; ++i) {
			struct GBAMP2kTrack* track = &mixer->activeTracks[i];
			if (track->notePlaying > 0) {
				--track->notePlaying;
				if (!track->notePlaying) {
					track->currentOffset = 0;
					track->samplePlaying = 0;
				}
			}
			if (track->notePlaying) {
				_stepSample(mixer, track);
			}
			if (!track->track.wait && !track->waiting) {
				_stepTrack(mixer, i);
				track->waiting = true;
			}
		}
		mixer->frame -= mixer->tempoI;
	}
}

void _mp2kVblank(struct GBAAudioMixer* mixer) {
	if (!mixer->contextAddress) {
		return;
	}
	mLOG(GBA_AUDIO, DEBUG, "Frame");
	mixer->p->externalMixing = true;
	_mp2kReload(mixer);
}
