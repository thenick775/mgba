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
#include "netplay-private.h"

mLOG_DEFINE_CATEGORY(NP, "Netplay")

const uint32_t mNP_PROTOCOL_VERSION = 1;

static void _coreReset(void* context);
static void _coreFrame(void* context);

static void _deleteCore(void* core);

struct mNPCore;

static THREAD_ENTRY _commThread(void* context);

DEFINE_VECTOR(mNPEventQueue, struct mNPEvent);

DECLARE_VECTOR(mNPCoreList, struct mNPCore*);
DEFINE_VECTOR(mNPCoreList, struct mNPCore*);

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
	struct mCoreCallbacks callbacks;
	Mutex mutex;
	uint32_t coreId;
	uint32_t roomId;
	uint32_t frameOffset;
	uint32_t flags;
	struct mNPEventQueue queue;
	struct mNPEventQueue sentQueue;
	uint32_t currentFrame;
	uint32_t nextSync;
	uint32_t syncPeriod;
	bool waitingForEvent;
	uint32_t lastInput;
	bool doingRollback;
	uint32_t rollbackStart;
	uint32_t rollbackEnd;
};

struct mNPRollback {
	uint32_t startFrame;
	uint32_t endFrame;
	uint32_t roomId;
};

struct mNPCoreFilter {
	struct mNPCoreList results;
	uint32_t roomId;
};

static void _filterCores(uint32_t id, void* value, void* user) {
	UNUSED(id);
	struct mNPCoreFilter* filter = user;
	struct mNPCore* core = value;
	if (filter->roomId && core->roomId == filter->roomId) {
		*mNPCoreListAppend(&filter->results) = core;
	}
}

static void _deleteCore(void* corep) {
	struct mNPCore* core = corep;
	mCoreThreadInterrupt(core->thread);
	core->thread->core->removeCoreCallbacks(core->thread->core, &core->callbacks);
	mCoreThreadContinue(core->thread);
	MutexDeinit(&core->mutex);
	mNPEventQueueDeinit(&core->queue);
	mNPEventQueueDeinit(&core->sentQueue);
	free(core);
}

struct mNPContext* mNPContextCreate(void) {
	struct mNPContext* context = malloc(sizeof(*context));
	if (!context) {
		return NULL;
	}
	memset(context, 0, sizeof(*context));
	TableInit(&context->cores, 8, _deleteCore);
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
			.flags = mNP_CORE_ALLOW_OBSERVE | mNP_CORE_ALLOW_CONTROL
		},
		.nonce = nonce
	};
	thread->core->getGameTitle(thread->core, data.info.gameTitle);
	thread->core->getGameCode(thread->core, data.info.gameCode);
	thread->core->checksum(thread->core, &data.info.crc32, CHECKSUM_CRC32);
	mCoreThreadContinue(thread);
	struct mNPCoreInfo* pending = malloc(sizeof(*pending));
	memcpy(pending, &data.info, sizeof(*pending));
	TableInsert(&context->pending, nonce, pending);
	mNPContextSend(context, &header, &data);
}

void mNPContextDeleteCore(struct mNPContext* context, uint32_t coreId) {
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_DELETE_CORE,
		.size = sizeof(struct mNPPacketDeleteCore),
		.flags = 0
	};
	struct mNPPacketDeleteCore data = {
		.coreId = coreId
	};
	mNPContextSend(context, &header, &data);
	TableRemove(&context->cores, coreId);
}

void mNPContextCloneCore(struct mNPContext* context, uint32_t coreId, uint32_t flags, uint32_t nonce) {
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_CLONE_CORE,
		.size = sizeof(struct mNPPacketCloneCore),
		.flags = 0
	};
	struct mNPPacketCloneCore data = {
		.coreId = coreId,
		.flags = flags,
		.nonce = nonce
	};
	struct mNPCoreInfo* pending = malloc(sizeof(*pending));
	memset(pending, 0, sizeof(*pending));
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

	memset(core, 0, sizeof(*core));
	core->p = context;
	core->thread = thread;
	MutexInit(&core->mutex);
	mCoreThreadInterrupt(thread);
	mNPEventQueueInit(&core->queue, 0);
	mNPEventQueueInit(&core->sentQueue, 0);
	core->flags = info->flags;
	core->roomId = info->roomId;
	core->coreId = info->coreId;
	core->frameOffset = thread->core->frameCounter(thread->core),
	core->waitingForEvent = false;
	core->callbacks.context = core;
	core->callbacks.videoFrameStarted = _coreFrame;
	core->callbacks.coreReset = _coreReset;
	thread->core->addCoreCallbacks(thread->core, &core->callbacks);
	mCoreThreadContinue(thread);
	TableInsert(&context->cores, info->coreId, core);
	TableRemove(&context->pending, nonce);
	if (info->roomId) {
		mNPContextJoinRoom(context, info->roomId, info->coreId);
	}
}

static void _sendEvent(struct mNPCore* core, enum mNPEventType type, uint32_t datum) {
	if (core->doingRollback) {
		return;
	}
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_EVENT,
		.size = sizeof(struct mNPPacketEvent),
		.flags = 0
	};
	struct mNPPacketEvent data = {
		.event = {
			.eventType = type,
			.coreId = core->coreId,
			.eventDatum = datum,
			.frameId = core->currentFrame
		}
	};
	*mNPEventQueueAppend(&core->sentQueue) = data.event;
	if (core->flags & mNP_CORE_ALLOW_CONTROL) {
		mNPContextSend(core->p, &header, &data);
	}
}

void mNPContextPushInput(struct mNPContext* context, uint32_t coreId, uint32_t input) {
	struct mNPCore* core = TableLookup(&context->cores, coreId);
	if (!core || !(core->flags & mNP_CORE_ALLOW_CONTROL) || !core->roomId || core->doingRollback) {
		return;
	}
	mLOG(NP, DEBUG, "Recieved input for coreId %" PRIi32 ": %" PRIx32, coreId, input);
	if (core->lastInput == input) {
		return;
	}
	core->lastInput = input;
	_sendEvent(core, mNP_EVENT_KEY_INPUT, input);
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
		return;
	case mNP_EVENT_FRAME:
		MutexLock(&core->mutex);
		while (mNPEventQueueSize(&core->sentQueue)) {
			struct mNPEvent* sentEvent = mNPEventQueueGetPointer(&core->sentQueue, 0);
			// TODO: Account for overflow
			if (sentEvent->frameId > event->frameId) {
				break;
			}
			mNPEventQueueShift(&core->sentQueue, 0, 1);
		}
		MutexUnlock(&core->mutex);
		break;
	case mNP_EVENT_RESET:
		mCoreThreadReset(core->thread);
		break;
	case mNP_EVENT_KEY_INPUT:
		core->thread->core->setKeys(core->thread->core, event->eventDatum);
		break;
	}
}

static void _pollEvent(struct mNPCore* core) {
	core->currentFrame = core->thread->core->frameCounter(core->thread->core) - core->frameOffset;
	if (!core->roomId) {
		return;
	}
	MutexLock(&core->mutex);
	if (core->doingRollback) {
		while (mNPEventQueueSize(&core->queue)) {
			// Copy event so we don't have to hold onto the mutex
			struct mNPEvent event = *mNPEventQueueGetPointer(&core->queue, 0);
			MutexUnlock(&core->mutex);
			if (event.frameId > core->currentFrame) {
				MutexLock(&core->mutex);
				break;
			} else {
				_handleEvent(core, &event);
				MutexLock(&core->mutex);
				mNPEventQueueShift(&core->queue, 0, 1);
			}
		}
		if (core->rollbackEnd == core->currentFrame || !mNPEventQueueSize(&core->queue)) {
			if (core->p->callbacks.rollbackEnd) {
				core->p->callbacks.rollbackEnd(core->p, &core->coreId, 1, core->p->userContext);
			}
			core->doingRollback = false;
		}
	}
	if (core->nextSync == core->currentFrame && !core->doingRollback) {
		core->waitingForEvent = true;
		mCoreThreadWaitFromThread(core->thread);
	}
	/*if (core->currentFrame > core->nextSync) {
		abort();
	}*/
	MutexUnlock(&core->mutex);
}

static void _coreReset(void* context) {
	struct mNPCore* core = context;
	core->currentFrame = 0;
	_pollEvent(core);
}

static void _coreFrame(void* context) {
	struct mNPCore* core = context;
	core->currentFrame = core->thread->core->frameCounter(core->thread->core) - core->frameOffset;

	if (core->doingRollback && core->rollbackStart < core->rollbackEnd) {
		uint32_t frames = core->currentFrame - core->rollbackStart;
		for (; frames; --frames) {
			mCoreRewindRestore(&core->thread->rewind, core->thread->core);
		}
		core->rollbackStart = core->rollbackEnd;
	}

	_pollEvent(core);

	if ((core->flags & mNP_CORE_ALLOW_OBSERVE) && core->roomId) {
		_sendEvent(core, mNP_EVENT_FRAME, core->currentFrame);
	}
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
		SocketSetBlocking(context->server, true);
		while (size) {
			size_t chunkSize = size;
			if (chunkSize > PKT_CHUNK_SIZE) {
				chunkSize = PKT_CHUNK_SIZE;
			}
			ssize_t received = SocketRecv(context->server, ptr, chunkSize);
			if (received < 0 || (size_t) received != chunkSize) {
				free(data);
				SocketSetBlocking(context->server, false);
				return false;
			}
			size -= chunkSize;
			ptr += chunkSize;
		}
		SocketSetBlocking(context->server, false);
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

static void _dispatchRollback(struct mNPContext* context, struct mNPRollback* rollback) {
	struct mNPCoreFilter filter = {
		.roomId = rollback->roomId
	};
	mNPCoreListInit(&filter.results, 0);
	TableEnumerate(&context->cores, _filterCores, &filter);
	if (context->callbacks.rollbackStart) {
		uint32_t* list = malloc(sizeof(uint32_t) * mNPCoreListSize(&filter.results));
		size_t i;
		for (i = 0; i < mNPCoreListSize(&filter.results); ++i) {
			struct mNPCore* core = *mNPCoreListGetPointer(&filter.results, i);
			list[i] = core->coreId;
			core->rollbackEnd = rollback->endFrame;
			if (core->doingRollback) {
				if (core->rollbackStart < core->currentFrame) {
					core->rollbackStart = rollback->startFrame;
				} else {
					core->rollbackStart = rollback->endFrame;
				}
			}
			core->doingRollback = true;
			if (core->currentFrame < core->rollbackEnd && core->waitingForEvent) {
				core->waitingForEvent = false;
				mCoreThreadStopWaiting(core->thread);
			}
		}
		context->callbacks.rollbackStart(context, list, mNPCoreListSize(&filter.results), context->userContext);
		free(list);
	}
	mNPCoreListDeinit(&filter.results);
}

static void _parseSync(struct mNPContext* context, const struct mNPPacketSync* sync, size_t size) {
	uint32_t nEvents = sync->nEvents;
	size -= sizeof(*sync);
	if (nEvents * sizeof(struct mNPEvent) > size) {
		mLOG(NP, WARN, "Received improperly sized Sync packet");
		nEvents = size / sizeof(struct mNPEvent);
	}
	struct mNPRollback rollback = {
		.roomId = 0
	};
	uint32_t i;
	for (i = 0; i < nEvents; ++i) {
		const struct mNPEvent* event = &sync->events[i];
		struct mNPCore* core = TableLookup(&context->cores, event->coreId);
		if (!core) {
			continue;
		}
		MutexLock(&core->mutex);
		if (!rollback.roomId && mNPEventQueueSize(&core->sentQueue) && memcmp(event, mNPEventQueueGetPointer(&core->sentQueue, 0), sizeof(*event)) == 0) {
			mNPEventQueueShift(&core->sentQueue, 0, 1);
			if (!mNPEventQueueSize(&core->sentQueue) && core->waitingForEvent) {
				core->waitingForEvent = false;
				mCoreThreadStopWaiting(core->thread);
			}
		} else {
			mNPEventQueueClear(&core->sentQueue);
			if (!rollback.roomId) {
				rollback.roomId = core->roomId;
				rollback.startFrame = event->frameId;
				rollback.endFrame = event->frameId;
			}
			*mNPEventQueueAppend(&core->queue) = *event;
		}
		core->nextSync = core->currentFrame + core->syncPeriod * 2;
		if (rollback.roomId) {
			if (300 > event->frameId - rollback.endFrame) {
				rollback.endFrame = event->frameId;
			}
		}
		MutexUnlock(&core->mutex);
	}
	if (rollback.roomId) {
		_dispatchRollback(context, &rollback);
	}
}

static void _parseJoin(struct mNPContext* context, const struct mNPPacketJoin* join, size_t size) {
	if (sizeof(*join) != size || !join) {
		return;
	}
	struct mNPCore* core = TableLookup(&context->cores, join->coreId);
	if (core) {
		core->roomId = join->roomId;
		core->currentFrame = join->currentFrame;
		core->nextSync = core->currentFrame + join->syncPeriod * 2;
		core->frameOffset = core->thread->core->frameCounter(core->thread->core) - core->currentFrame;
		core->syncPeriod = join->syncPeriod;
		if (core->waitingForEvent) {
			core->waitingForEvent = false;
			mCoreThreadStopWaiting(core->thread);
		}
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
		break;
	case mNP_LIST_ROOMS:
		_parseListRooms(context, (const struct mNPPacketListRooms*) list, size);
		break;
	}
}

static void _parseRegisterCore(struct mNPContext* context, const struct mNPPacketRegisterCore* reg, size_t size) {
	if (sizeof(*reg) != size || !reg) {
		return;
	}
	struct mNPCoreInfo* pending = TableLookup(&context->pending, reg->nonce);
	memcpy(pending, &reg->info, sizeof(*pending));
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
