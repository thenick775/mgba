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
struct mNPCoreInfo;
struct mNPRoomInfo;

struct mNPCallbacks {
	void (*serverConnected)(struct mNPContext*, void* user);
	void (*serverShutdown)(struct mNPContext*, void* user);
	void (*coreRegistered)(struct mNPContext*, const struct mNPCoreInfo*, uint32_t nonce, void* user);
	void (*roomJoined)(struct mNPContext*, uint32_t roomId, uint32_t coreId, void* user);
	void (*listRooms)(struct mNPContext*, const struct mNPRoomInfo* rooms, uint32_t nRooms, void* user);
	void (*listCores)(struct mNPContext*, const struct mNPCoreInfo* cores, uint32_t nCores, uint32_t roomId, void* user);
};

enum mNPCoreFlags {
	mNP_CORE_ALLOW_OBSERVE = 1,
	mNP_CORE_ALLOW_CONTROL = 2
};

struct mNPCoreInfo {
	uint32_t platform;
	char gameTitle[16];
	char gameCode[8];
	uint32_t crc32;
	uint32_t coreId;
	uint32_t roomId;
	int32_t frameOffset;
	uint32_t flags;
};

enum mNPRoomFlags {
	mNP_ROOM_REQUIRE_VERSION = 1,
};

struct mNPRoomInfo {
	uint8_t requiredCommitHash[20];
	uint32_t nCores;
	uint32_t syncDuration;
	uint32_t flags;
};

enum mNPEventType {
	mNP_EVENT_NONE = 0,
	mNP_EVENT_KEY_INPUT,
	mNP_EVENT_FRAME,
	mNP_EVENT_RESET,
	mNP_EVENT_MAX
};

struct mNPEvent {
	uint32_t eventType;
	uint32_t coreId;
	uint32_t eventDatum;
	uint32_t frameId;
};

struct mNPContext* mNPContextCreate(void);
void mNPContextAttachCallbacks(struct mNPContext*, const struct mNPCallbacks*, void* user);
void mNPContextDestroy(struct mNPContext*);

struct mCoreThread;
void mNPContextRegisterCore(struct mNPContext*, struct mCoreThread*, uint32_t nonce);
void mNPContextJoinRoom(struct mNPContext*, uint32_t roomId, uint32_t coreId);
void mNPContextAttachCore(struct mNPContext*, struct mCoreThread*, uint32_t nonce);
void mNPContextPushInput(struct mNPContext*, uint32_t coreId, uint32_t input);

void mNPContextListRooms(struct mNPContext*);
void mNPContextListCores(struct mNPContext*, uint32_t roomId);

struct mNPServer;
struct mNPServerOptions;
bool mNPContextConnect(struct mNPContext*, const struct mNPServerOptions*);
void mNPContextDisconnect(struct mNPContext*);

CXX_GUARD_END

#endif
