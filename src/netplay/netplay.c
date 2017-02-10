/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <mgba/internal/netplay/netplay.h>

#include <mgba/core/core.h>
#include <mgba/core/thread.h>
#include <mgba/core/version.h>
#include <mgba/internal/netplay/server.h>
#include <mgba-util/string.h>
#include <mgba-util/vector.h>
#include "netplay-private.h"

mLOG_DEFINE_CATEGORY(NP, "Netplay")

const uint32_t mNP_PROTOCOL_VERSION = 1;

static void _coreReset(void* context);
static void _coreFrame(void* context);

struct mNPCore;

static THREAD_ENTRY _commThread(void* context);

DECLARE_VECTOR(mNPEventQueue, struct mNPEvent);
DEFINE_VECTOR(mNPEventQueue, struct mNPEvent);

struct mNPContext {
	struct mNPCallbacks callbacks;
	void* userContext;
	Socket server;
	bool connected;

	Thread commThread;
	struct mNPCommFIFO commFifo;

	struct Table cores;
	struct Table pending;
};

struct mNPCore {
	struct mNPContext* p;
	struct mCoreThread* thread;
	Mutex mutex;
	uint32_t coreId;
	uint32_t roomId;
	int32_t frameOffset;
	uint32_t flags;
	struct mNPEventQueue queue;
	uint32_t framesRemaining;
	bool waitingForEvent;
};

struct mNPContext* mNPContextCreate(void) {
	struct mNPContext* context = malloc(sizeof(*context));
	if (!context) {
		return NULL;
	}
	memset(context, 0, sizeof(*context));
	TableInit(&context->cores, 8, free); // TODO: Properly free
	TableInit(&context->pending, 4, free);
	return context;
}

void mNPContextAttachCallbacks(struct mNPContext* context, const struct mNPCallbacks* callbacks, void* user) {
	context->callbacks = *callbacks;
	context->userContext = user;
}

void mNPContextDestroy(struct mNPContext* context) {
	TableDeinit(&context->cores);
	TableDeinit(&context->pending);
	free(context);
}

void mNPContextRegisterCore(struct mNPContext* context, struct mCoreThread* thread, uint32_t nonce) {
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_REGISTER_CORE,
		.size = sizeof(struct mNPPacketRegisterCore),
		.flags = 0
	};
	mCoreThreadInterrupt(thread);
	struct mNPPacketRegisterCore data = {
		.info = {
			.platform = thread->core->platform(thread->core),
			.frameOffset = 0,
			.flags = 0
		},
		.nonce = nonce
	};
	thread->core->getGameTitle(thread->core, data.info.gameTitle);
	thread->core->getGameCode(thread->core, data.info.gameCode);
	thread->core->checksum(thread->core, &data.info.crc32, CHECKSUM_CRC32);
	mCoreThreadContinue(thread);
	struct mNPPacketRegisterCore* pending = malloc(sizeof(*pending));
	memcpy(pending, &data, sizeof(data));
	TableInsert(&context->pending, nonce, pending);
	mNPContextSend(context, &header, &data);
}

void mNPContextJoinRoom(struct mNPContext* context, uint32_t roomId, uint32_t coreId) {
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_JOIN,
		.size = sizeof(struct mNPPacketJoin),
		.flags = 0
	};
	struct mNPPacketJoin data = {
		.roomId = roomId,
		.coreId = coreId,
		.syncPeriod = 6,
		.capacity = 4
	};
	mNPContextSend(context, &header, &data);
}

void mNPContextAttachCore(struct mNPContext* context, struct mCoreThread* thread, uint32_t nonce) {
	struct mNPCoreInfo* info = TableLookup(&context->pending, nonce);
	if (!info) {
		return;
	}

	struct mNPCore* core = malloc(sizeof(*core));
	if (!core) {
		return;
	}

	core->p = context;
	core->thread = thread;
	MutexInit(&core->mutex);
	mCoreThreadInterrupt(thread);
	mNPEventQueueInit(&core->queue, 0);
	core->flags = info->flags;
	core->roomId = info->roomId;
	core->coreId = info->coreId;
	core->frameOffset = info->frameOffset - thread->core->frameCounter(thread->core),
	core->waitingForEvent = false;
	struct mCoreCallbacks callbacks = {
		.context = core,
		.videoFrameStarted = _coreFrame,
		.coreReset = _coreReset
	};
	thread->core->addCoreCallbacks(thread->core, &callbacks);
	mCoreThreadContinue(thread);
	TableInsert(&context->cores, info->coreId, core);
	TableRemove(&context->pending, nonce);
}

void mNPContextPushInput(struct mNPContext* context, uint32_t coreId, uint32_t input) {
	struct mNPCore* core = TableLookup(&context->cores, coreId);
	if (!(core->flags & mNP_CORE_ALLOW_CONTROL) || !core->roomId) {
		return;
	}

	mLOG(NP, DEBUG, "Recieved input for coreId %" PRIi32 ": %" PRIx32, coreId, input);

	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_EVENT,
		.size = sizeof(struct mNPPacketEvent),
		.flags = 0
	};
	struct mNPPacketEvent data = {
		.event = {
			.eventType = mNP_EVENT_KEY_INPUT,
			.coreId = coreId,
			.eventDatum = input,
			.frameId = core->thread->core->frameCounter(core->thread->core) + core->frameOffset
		}
	};
	mNPContextSend(context, &header, &data);
}

void mNPContextListRooms(struct mNPContext* context) {
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_LIST,
		.size = sizeof(struct mNPPacketList),
		.flags = 0
	};
	struct mNPPacketList data = {
		.type = mNP_LIST_ROOMS,
		.parent = 0,
		.padding = 0
	};
	mNPContextSend(context, &header, &data);
}

void mNPContextListCores(struct mNPContext* context, uint32_t roomId) {
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_LIST,
		.size = sizeof(struct mNPPacketList),
		.flags = 0
	};
	struct mNPPacketList data = {
		.type = mNP_LIST_CORES,
		.parent = roomId,
		.padding = 0
	};
	mNPContextSend(context, &header, &data);
}

bool mNPContextConnect(struct mNPContext* context, const struct mNPServerOptions* opts) {
	if (context->connected) {
		return false;
	}
	Socket serverSocket = SocketConnectTCP(opts->port, &opts->address);
	if (SOCKET_FAILED(serverSocket)) {
		mLOG(NP, ERROR, "Failed to connect to server");
		return false;
	}
	SocketSetTCPPush(serverSocket, 1);

	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_CONNECT,
		.size = sizeof(struct mNPPacketConnect),
		.flags = 0
	};
	struct mNPPacketConnect data = {
		.protocolVersion = mNP_PROTOCOL_VERSION
	};
	const char* ptr = gitCommit;
	size_t i;
	for (i = 0; i < 20; ++i) {
		ptr = hex8(ptr, &data.commitHash[i]);
		if (!ptr) {
			break;
		}
	}

	SocketSetBlocking(serverSocket, false);
	context->server = serverSocket;
	mNPCommFIFOInit(&context->commFifo);

	mNPContextSend(context, &header, &data);
	ThreadCreate(&context->commThread, _commThread, context);
	return true;
}

void mNPContextDisconnect(struct mNPContext* context) {
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_SHUTDOWN,
		.size = 0,
		.flags = 0
	};
	mNPContextSend(context, &header, NULL);
	ThreadJoin(context->commThread);
	mLOG(NP, INFO, "Disconnected from server");
	context->connected = false;
	TableClear(&context->cores);
	TableClear(&context->pending);
	SocketClose(context->server);
	mNPCommFIFODeinit(&context->commFifo);
}

static void _handleEvent(struct mNPCore* core, const struct mNPEvent* event) {
	switch (event->eventType) {
	case mNP_EVENT_NONE:
	case mNP_EVENT_FRAME:
		return;
	case mNP_EVENT_RESET:
		mCoreThreadReset(core->thread);
		break;
	case mNP_EVENT_KEY_INPUT:
		core->thread->core->setKeys(core->thread->core, event->eventDatum);
		break;
	}
}

static void _pollEvent(struct mNPCore* core) {
	// TODO: Implement rollback
	uint32_t currentFrame = core->thread->core->frameCounter(core->thread->core) - core->frameOffset;
	if (!core->roomId) {
		return;
	}
	bool needsToWait = core->framesRemaining == 0;
	MutexLock(&core->mutex);
	while (needsToWait && mNPEventQueueSize(&core->queue)) {
		// Copy event so we don't have to hold onto the mutex
		struct mNPEvent event = *mNPEventQueueGetPointer(&core->queue, 0);
		MutexUnlock(&core->mutex);
		if (event.frameId > currentFrame) {
			needsToWait = false;
			MutexLock(&core->mutex);
		} else {
			_handleEvent(core, &event);
			MutexLock(&core->mutex);
			mNPEventQueueShift(&core->queue, 0, 1);
		}
	}
	MutexUnlock(&core->mutex);
	if (needsToWait) {
		core->waitingForEvent = true;
		mCoreThreadWaitFromThread(core->thread);
	}
}

static void _coreReset(void* context) {
	struct mNPCore* core = context;
	_pollEvent(core);
}

static void _coreFrame(void* context) {
	struct mNPCore* core = context;
	_pollEvent(core);
	--core->framesRemaining;
}

static bool _commRecv(struct mNPContext* context) {
	struct mNPPacketHeader header;
	Socket reads[1] = { context->server };
	Socket errors[1] = { context->server };
	SocketPoll(1, reads, NULL, errors, 4);
	if (!SOCKET_FAILED(*errors)) {
		return false;
	}
	if (SOCKET_FAILED(*reads)) {
		return true;
	}
	ssize_t len = SocketRecv(context->server, &header, sizeof(header));
	if (len < 0 && !SocketWouldBlock()) {
		return false;
	}
	if (len != sizeof(header)) {
		return true;
	}
	void* data = NULL;
	if (header.size && header.size < PKT_MAX_SIZE) {
		data = malloc(header.size);
		if (!data) {
			return false;
		}
		uint8_t* ptr = data;
		size_t size = header.size;
		while (size) {
			size_t chunkSize = size;
			if (chunkSize > PKT_CHUNK_SIZE) {
				chunkSize = PKT_CHUNK_SIZE;
			}
			ssize_t received = SocketRecv(context->server, ptr, chunkSize);
			if (received < 0 || (size_t) received != chunkSize) {
				free(data);
				return false;
			}
			size -= chunkSize;
			ptr += chunkSize;
		}
		if (size > 0) {
			free(data);
			data = NULL;
		}
	}
	bool result = mNPContextRecv(context, &header, data);
	if (data) {
		free(data);
	}
	return result;
}

static THREAD_ENTRY _commThread(void* user) {
	ThreadSetName("Netplay Client Thread");
	mLOG(NP, INFO, "Client thread started");
	struct mNPContext* context = user;
	while (true) {
		if (!_commRecv(context)) {
			break;
		}
		if (!mNPCommFIFOFlush(&context->commFifo, context->server)) {
			// We got a SHUTDOWN packet
			break;
		}
	}
	mLOG(NP, INFO, "Client thread exited");

	return 0;
}

void mNPContextSend(struct mNPContext* context, const struct mNPPacketHeader* header, const void* body) {
	mNPCommFIFOWrite(&context->commFifo, header, sizeof(*header));
	size_t size = header->size;
	if (size && body) {
		const uint8_t* ptr = body;
		while (size) {
			size_t chunkSize = size;
			if (chunkSize > PKT_CHUNK_SIZE) {
				chunkSize = PKT_CHUNK_SIZE;
			}
			mNPCommFIFOWrite(&context->commFifo, ptr, chunkSize);
			size -= chunkSize;
			ptr += chunkSize;
		}
	}
}

static bool _parseConnect(struct mNPContext* context, const struct mNPPacketAck* reply, size_t size) {
	if (sizeof(*reply) != size || !reply) {
		return false;
	}
	if (reply->reply < 0) {
		return false;
	}
	context->connected = true;
	if (context->callbacks.serverConnected) {
		context->callbacks.serverConnected(context, context->userContext);
	}
	return true;
}

static void _parseSync(struct mNPContext* context, const struct mNPPacketSync* sync, size_t size) {
	uint32_t nEvents = sync->nEvents;
	size -= sizeof(nEvents);
	if (nEvents * sizeof(struct mNPEvent) > size) {
		mLOG(NP, WARN, "Received improperly sized Sync packet");
		nEvents = size / sizeof(struct mNPEvent);
	}
	uint32_t i;
	for (i = 0; i < nEvents; ++i) {
		const struct mNPEvent* event = &sync->events[i];
		struct mNPCore* core = TableLookup(&context->cores, event->coreId);
		if (!core) {
			continue;
		}
		MutexLock(&core->mutex);
		*mNPEventQueueAppend(&core->queue) = *event;
		MutexUnlock(&core->mutex);
	}
}

static void _parseJoin(struct mNPContext* context, const struct mNPPacketJoin* join, size_t size) {
	if (sizeof(*join) != size || !join) {
		return;
	}
	struct mNPCore* core = TableLookup(&context->cores, join->coreId);
	if (core) {
		core->roomId = join->roomId;
		core->framesRemaining = join->syncPeriod;
	}
	if (context->callbacks.roomJoined) {
		context->callbacks.roomJoined(context, join->roomId, join->coreId, context->userContext);
	}
}

static void _parseListCores(struct mNPContext* context, const struct mNPPacketListCores* list, size_t size) {
	if (size != sizeof(struct mNPPacketListCores) + list->nCores * sizeof(struct mNPCoreInfo)) {
		return;
	}
	if (context->callbacks.listCores) {
		context->callbacks.listCores(context, list->cores, list->nCores, list->parent, context->userContext);
	}
}

static void _parseListRooms(struct mNPContext* context, const struct mNPPacketListRooms* list, size_t size) {
	if (size != sizeof(struct mNPPacketListRooms) + list->nRooms * sizeof(struct mNPRoomInfo)) {
		return;
	}
	if (context->callbacks.listRooms) {
		context->callbacks.listRooms(context, list->rooms, list->nRooms, context->userContext);
	}
}

static void _parseList(struct mNPContext* context, const struct mNPPacketList* list, size_t size) {
	if (sizeof(*list) > size || !list) {
		return;
	}

	switch (list->type) {
	case mNP_LIST_CORES:
		_parseListCores(context, (const struct mNPPacketListCores*) list, size);
	case mNP_LIST_ROOMS:
		_parseListRooms(context, (const struct mNPPacketListRooms*) list, size);
	}
}

static void _parseRegisterCore(struct mNPContext* context, const struct mNPPacketRegisterCore* reg, size_t size) {
	if (sizeof(*reg) != size || !reg) {
		return;
	}
	struct mNPPacketRegisterCore* pending = TableLookup(&context->pending, reg->nonce);
	pending->info.coreId = reg->info.coreId;
	pending->info.flags |= mNP_CORE_ALLOW_CONTROL | mNP_CORE_ALLOW_OBSERVE;
	context->callbacks.coreRegistered(context, &reg->info, reg->nonce, context->userContext);
}

bool mNPContextRecv(struct mNPContext* context, const struct mNPPacketHeader* header, const void* body) {
	switch (header->packetType) {
	case mNP_PKT_ACK:
		if (!context->connected) {
			return _parseConnect(context, body, header->size);
		}
		// TODO
		break;
	case mNP_PKT_SHUTDOWN:
		mLOG(NP, INFO, "Server shut down");
		if (context->callbacks.serverShutdown) {
			context->callbacks.serverShutdown(context, context->userContext);
		}
		return false;
	case mNP_PKT_SYNC:
		_parseSync(context, body, header->size);
		break;
	case mNP_PKT_JOIN:
		_parseJoin(context, body, header->size);
		break;
	case mNP_PKT_LIST:
		_parseList(context, body, header->size);
		break;
	case mNP_PKT_REGISTER_CORE:
		_parseRegisterCore(context, body, header->size);
		break;
	default:
		break;
	}
	// TODO
	return true;
}

void mNPAck(Socket sock, enum mNPReplyType reply) {
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_ACK,
		.size = sizeof(struct mNPPacketAck),
		.flags = 0
	};
	struct mNPPacketAck ack = {
		.reply = reply
	};
	SocketSend(sock, &header, sizeof(header));
	SocketSend(sock, &ack, sizeof(ack));
}

void mNPCommFIFOInit(struct mNPCommFIFO* fifo) {
	RingFIFOInit(&fifo->fifo, COMM_FIFO_SIZE);
	MutexInit(&fifo->mutex);
	ConditionInit(&fifo->fifoFull);
	ConditionInit(&fifo->fifoEmpty);
	fifo->buffer = malloc(PKT_CHUNK_SIZE);
}

void mNPCommFIFODeinit(struct mNPCommFIFO* fifo) {
	RingFIFODeinit(&fifo->fifo);
	MutexDeinit(&fifo->mutex);
	ConditionDeinit(&fifo->fifoFull);
	ConditionDeinit(&fifo->fifoEmpty);
	free(fifo->buffer);
}

void mNPCommFIFOWrite(struct mNPCommFIFO* fifo, const void* data, size_t size) {
	MutexLock(&fifo->mutex);
	while (!RingFIFOWrite(&fifo->fifo, data, size)) {
		ConditionWake(&fifo->fifoEmpty);
		ConditionWait(&fifo->fifoFull, &fifo->mutex);
	}
	ConditionWake(&fifo->fifoEmpty);
	MutexUnlock(&fifo->mutex);
}

void mNPCommFIFORead(struct mNPCommFIFO* fifo, void* data, size_t size) {
	MutexLock(&fifo->mutex);
	while (!RingFIFORead(&fifo->fifo, data, size)) {
		ConditionWake(&fifo->fifoFull);
		ConditionWait(&fifo->fifoEmpty, &fifo->mutex);
	}
	ConditionWake(&fifo->fifoFull);
	MutexUnlock(&fifo->mutex);
}

bool mNPCommFIFOTryRead(struct mNPCommFIFO* fifo, void* data, size_t size) {
	MutexLock(&fifo->mutex);
	bool success = RingFIFORead(&fifo->fifo, data, size);
	if (success) {
		ConditionWake(&fifo->fifoFull);
	}
	MutexUnlock(&fifo->mutex);
	return success;
}

bool mNPCommFIFOFlush(struct mNPCommFIFO* commFifo, Socket sock) {
	struct mNPPacketHeader header;
	uint8_t* chunkedData = commFifo->buffer;
	while (mNPCommFIFOTryRead(commFifo, &header, sizeof(header))) {
		SocketSend(sock, &header, sizeof(header));
		while (header.size) {
			size_t chunkSize = header.size;
			if (chunkSize > PKT_CHUNK_SIZE) {
				chunkSize = PKT_CHUNK_SIZE;
			}
			mNPCommFIFORead(commFifo, chunkedData, chunkSize);
			header.size -= chunkSize;
			SocketSend(sock, chunkedData, chunkSize);
		}
		if (header.packetType == mNP_PKT_SHUTDOWN) {
			return false;
		}
	}
	return true;
}

bool mNPCommFIFOPoll(struct mNPCommFIFO* fifo, int32_t timeoutMs) {
	bool empty = false;
	MutexLock(&fifo->mutex);
	empty = RingFIFOEmpty(&fifo->fifo);
	if (empty && timeoutMs) {
		empty = ConditionWaitTimed(&fifo->fifoEmpty, &fifo->mutex, timeoutMs);
	}
	MutexUnlock(&fifo->mutex);
	return !empty;
}
