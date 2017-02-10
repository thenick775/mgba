/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef M_NETPLAY_SERVER_H
#define M_NETPLAY_SERVER_H

#include <mgba-util/common.h>

CXX_GUARD_START

#include <mgba/internal/netplay/netplay.h>
#include <mgba-util/socket.h>

mLOG_DECLARE_CATEGORY(NP_SERVER)

struct mNPServerOptions {
	struct Address address;
	uint16_t port;
};

enum mNPPacketType {
	mNP_PKT_ACK,
	mNP_PKT_CONNECT,
	mNP_PKT_SHUTDOWN,
	mNP_PKT_JOIN,
	mNP_PKT_LEAVE,
	mNP_PKT_LIST,
	mNP_PKT_DATA,
	mNP_PKT_REQUEST,
	mNP_PKT_SYNC,
	mNP_PKT_EVENT,
	mNP_PKT_REGISTER_CORE,
	mNP_PKT_CLONE_CORE,
	mNP_PKT_MAX
};

enum mNPDataType {
	mNP_DATA_GENERIC,
	mNP_DATA_CORE_CUSTOM,
	mNP_DATA_SAVESTATE,
	mNP_DATA_SCREENSHOT,
	mNP_DATA_MAX
};

enum mNPReplyType {
	mNP_REPLY_OK = 1,
	mNP_REPLY_NO_SUCH_CORE = -1,
	mNP_REPLY_FULL = -2,
	mNP_REPLY_PROTO_TOO_OLD = -3,
	mNP_REPLY_MALFORMED = -4,
	mNP_REPLY_DOES_NOT_EXIST = -5,
	mNP_REPLY_DISALLOWED = -6,
	mNP_REPLY_BUSY = -7
};

enum mNPListType {
	mNP_LIST_CORES,
	mNP_LIST_ROOMS,
	mNP_LIST_MAX
};

struct mNPPacketHeader {
	uint32_t packetType;
	uint32_t size;
	uint32_t flags;
};

struct mNPPacketAck {
	int32_t reply;
};

struct mNPPacketConnect {
	uint32_t protocolVersion;
	uint8_t commitHash[20];
};

struct mNPPacketJoin {
	uint32_t roomId;
	uint32_t coreId;
	uint32_t syncPeriod;
	uint32_t capacity;
	uint32_t flags;
};

struct mNPPacketLeave {
	uint32_t roomId;
	uint32_t coreId;
};

struct mNPPacketList {
	uint32_t type;
	uint32_t parent;
	uint32_t padding;
};

struct mNPPacketListCores {
	uint32_t type;
	uint32_t parent;
	uint32_t nCores;
	struct mNPCoreInfo cores[];
};

struct mNPPacketListRooms {
	uint32_t type;
	uint32_t parent;
	uint32_t nRooms;
	struct mNPRoomInfo rooms[];
};

struct mNPPacketData {
	uint32_t type;
	uint32_t coreId;
	uint8_t data[];
};

struct mNPPacketRequest {
	uint32_t coreId;
	uint32_t type;
};

struct mNPPacketSync {
	uint32_t nEvents;
	struct mNPEvent events[];
};

struct mNPPacketEvent {
	struct mNPEvent event;
};

struct mNPPacketRegisterCore {
	struct mNPCoreInfo info;
	uint32_t nonce;
};

struct mNPServer;
struct mNPServer* mNPServerStart(const struct mNPServerOptions*);
void mNPServerStop(struct mNPServer*);
void mNPServerRunOnce(struct mNPServer*);

CXX_GUARD_END

#endif
