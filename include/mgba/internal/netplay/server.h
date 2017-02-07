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
	mNP_PKT_MAX
};

enum mNPDataType {
	mNP_DATA_GENERIC,
	mNP_DATA_KEY_INPUT,
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
};

struct mNPPacketHeader {
	uint32_t packetType;
	uint32_t size;
	uint32_t flags;
	uint32_t coreId;
};

struct mNPPacketAck {
	uint32_t reply;
};

struct mNPPacketConnect {
	uint32_t protocolVersion;
	uint8_t commitHash[20];
};

struct mNPPacketJoin {
	uint32_t roomId;
};

struct mNPPacketLeave {
	uint32_t roomId;
};

struct mNPPacketList {
	uint32_t nCores;
	struct mNPCoreInfo cores[];
};

struct mNPPacketData {
	uint32_t type;
	uint8_t data[];
};

struct mNPPacketSmallData {
	uint32_t type;
	uint32_t datum;
};

struct mNPPacketRequest {
	uint32_t type;
};

struct mNPServer;
struct mNPServer* mNPServerStart(const struct mNPServerOptions*);
void mNPServerStop(struct mNPServer*);
void mNPServerRunOnce(struct mNPServer*);

CXX_GUARD_END

#endif
