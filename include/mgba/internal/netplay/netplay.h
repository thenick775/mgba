/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef M_NETPLAY_H
#define M_NETPLAY_H

#include <mgba-util/common.h>

CXX_GUARD_START

#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba-util/vector.h>

extern const uint32_t mNP_PROTOCOL_VERSION;

mLOG_DECLARE_CATEGORY(NP);

struct mNPContext;

struct mNPCallbacks {
	void (*serverShutdown)(struct mNPContext*, void* user);
};

enum mNPCoreFlags {
	mNP_CORE_ALLOW_OBSERVE = 1,
	mNP_CORE_ALLOW_CONTROL = 2,
};

struct mNPCoreInfo {
	uint32_t platform;
	char gameTitle[16];
	char gameCode[8];
	uint32_t crc32;
	uint32_t coreId;
	uint32_t flags;
};

enum mNPRoomFlags {
	mNP_ROOM_REQUIRE_VERSION = 1,
};

struct mNPRoomInfo {
	uint8_t requiredCommitHash[20];
	uint32_t nCores;
	uint32_t flags;
};

struct mNPContext* mNPContextCreate(void);
void mNPContextAttachCallbacks(struct mNPContext*, struct mNPCallbacks*, void* user);
void mNPContextDestroy(struct mNPContext*);

struct mCoreThread;
void mNPContextWillAttachCore(struct mNPContext*, struct mCoreThread*, uint32_t roomId);
void mNPContextAttachCore(struct mNPContext*, struct mCoreThread*, uint32_t coreId, bool hasControl);
void mNPContextPushInput(struct mNPContext*, uint32_t coreId, uint32_t input);

struct mNPServer;
struct mNPServerOptions;
bool mNPContextConnect(struct mNPContext*, const struct mNPServerOptions*);
void mNPContextDisconnect(struct mNPContext*);

CXX_GUARD_END

#endif
