/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef M_NETPLAY_PRIVATE_H
#define M_NETPLAY_PRIVATE_H

#include <mgba-util/common.h>

#include <mgba/internal/netplay/netplay.h>
#include <mgba/internal/netplay/server.h>
#include <mgba-util/ring-fifo.h>
#include <mgba-util/table.h>
#include <mgba-util/threading.h>

#define PKT_CHUNK_SIZE 0x10000
#define PKT_MAX_SIZE 0x4000000
#define COMM_FIFO_SIZE 0x40000

bool mNPContextRecv(struct mNPContext*, const struct mNPPacketHeader* header, const void* body);
void mNPContextSend(struct mNPContext*, const struct mNPPacketHeader* header, const void* body);

void mNPAck(Socket sock, enum mNPReplyType reply);

struct mNPCommFIFO {
	Mutex mutex;
	struct RingFIFO fifo;
	Condition fifoFull;
	Condition fifoEmpty;
};

void mNPCommFIFOInit(struct mNPCommFIFO*);
void mNPCommFIFODeinit(struct mNPCommFIFO*);
void mNPCommFIFOWrite(struct mNPCommFIFO*, const void*, size_t);
void mNPCommFIFORead(struct mNPCommFIFO*, void*, size_t);
bool mNPCommFIFOTryRead(struct mNPCommFIFO*, void*, size_t);

#endif
