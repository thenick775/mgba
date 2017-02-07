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

static void _coreStart(struct mCoreThread* threadContext);
static void _coreReset(struct mCoreThread* threadContext);
static void _coreClean(struct mCoreThread* threadContext);
static void _coreFrame(struct mCoreThread* threadContext);

struct mNPCore;
static void _pollInput(struct mNPCore* core);
static void _updateInput(struct mNPCore* core);

static THREAD_ENTRY _commThread(void* context);

struct mNPCore {
	struct mNPContext* p;
	struct mCoreThread* thread;
	ThreadCallback startCallback;
	ThreadCallback resetCallback;
	ThreadCallback cleanCallback;
	ThreadCallback frameCallback;
	Mutex mutex;
	void* threadData;
	bool hasControl;
	uint32_t coreId;
	unsigned nClients;
	uint32_t nPendingInputs;
	uint32_t pendingInput;
};

struct mNPContext* mNPContextCreate(void) {
	struct mNPContext* context = malloc(sizeof(*context));
	memset(context, 0, sizeof(*context));
	TableInit(&context->cores, 8, free); // TODO: Properly free
	TableInit(&context->coresWaiting, 8, NULL);
	return context;
}

void mNPContextDestroy(struct mNPContext* context) {
	TableDeinit(&context->cores);
	TableDeinit(&context->coresWaiting);
	free(context);
}

void mNPContextAttachCore(struct mNPContext* context, struct mCoreThread* thread, uint32_t coreId, bool hasControl) {
	struct mNPCore* core = malloc(sizeof(*core));
	core->p = context;
	core->thread = thread;
	MutexInit(&core->mutex);
	mCoreThreadInterrupt(thread);
	core->startCallback = thread->startCallback;
	core->resetCallback = thread->resetCallback;
	core->cleanCallback = thread->cleanCallback;
	core->frameCallback = thread->frameCallback;
	core->threadData = thread->userData;
	thread->startCallback = _coreStart;
	thread->resetCallback = _coreReset;
	thread->cleanCallback = _coreClean;
	thread->frameCallback = _coreFrame;
	thread->userData = core;
	mCoreThreadContinue(thread);
	core->hasControl = hasControl;
	core->coreId = coreId;
	TableInsert(&context->cores, coreId, core);
}

void mNPContextPushInput(struct mNPContext* context, uint32_t coreId, uint32_t input) {
	struct mNPCore* core = TableLookup(&context->cores, coreId);
	mLOG(NP, DEBUG, "Recieved input for coreId %" PRIi32 ": %" PRIx32, coreId, input);
	if (!core || core->nPendingInputs) {
		return;
	}
	MutexLock(&core->mutex);
	if (core->nClients == core->nPendingInputs) {
		core->pendingInput = 0;
	}
	core->pendingInput |= input;
	input = core->pendingInput;
	--core->nPendingInputs;
	if (!core->nPendingInputs) {
		TableRemove(&core->p->coresWaiting, coreId);
		_updateInput(core);
	}
	MutexUnlock(&core->mutex);
	if (core->hasControl) {
		struct mNPPacketHeader header = {
			.packetType = mNP_PKT_DATA,
			.size = sizeof(struct mNPPacketSmallData),
			.flags = 0,
			.coreId = coreId
		};
		struct mNPPacketSmallData data = {
			.type = mNP_DATA_KEY_INPUT,
			.datum = input
		};
		mNPContextSend(context, &header, &data);
	}
}

bool mNPContextConnect(struct mNPContext* context, const struct mNPServerOptions* opts) {
	Socket serverSocket = SocketConnectTCP(opts->port, &opts->address);
	if (SOCKET_FAILED(serverSocket)) {
		mLOG(NP, ERROR, "Failed to connect to server");
		return false;
	}

	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_CONNECT,
		.size = sizeof(struct mNPPacketConnect),
		.flags = 0,
		.coreId = 0
	};
	struct mNPPacketConnect data = { .protocolVersion = mNP_PROTOCOL_VERSION };
	const char* ptr = gitCommit;
	size_t i;
	for (i = 0; i < 20; ++i) {
		ptr = hex8(ptr, &data.commitHash[i]);
		if (!ptr) {
			break;
		}
	}

	context->server = serverSocket;
	RingFIFOInit(&context->commFifo, COMM_FIFO_SIZE);
	MutexInit(&context->commMutex);
	ConditionInit(&context->commFifoFull);
	ConditionInit(&context->commFifoEmpty);

	mNPContextSend(context, &header, &data);
	ThreadCreate(&context->commThread, _commThread, context);
	return true;
}

void mNPContextDisconnect(struct mNPContext* context) {
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_SHUTDOWN,
		.size = 0,
		.flags = 0,
		.coreId = 0
	};
	mNPContextSend(context, &header, NULL);
	ThreadJoin(context->commThread);
	mLOG(NP, INFO, "Disconnected from server");
}

static void _pollInput(struct mNPCore* core) {
	// TODO: Implement rollback
	MutexLock(&core->mutex);
	if (core->nPendingInputs) {
		mCoreThreadWaitFromThread(core->thread);
	}
	MutexUnlock(&core->mutex);
}

static void _wakeupCores(uint32_t coreId, void* value, void* user) {
	UNUSED(user);
	struct mNPCore* core = value;
	TableInsert(&core->p->cores, coreId, core);
	core->nClients = core->nPendingInputs;
	mCoreThreadStopWaiting(core->thread);
}

static void _updateInput(struct mNPCore* core) {
	core->thread->core->setKeys(core->thread->core, core->pendingInput);
	if (!TableSize(&core->p->coresWaiting)) {
		TableEnumerate(&core->p->coresWaiting, _wakeupCores, NULL);
	}
}

static void _coreStart(struct mCoreThread* threadContext) {
	struct mNPCore* core = threadContext->userData;
	threadContext->userData = core->threadData;
	core->startCallback(threadContext);
	threadContext->userData = core;
	_pollInput(core);
}

static void _coreReset(struct mCoreThread* threadContext) {
	struct mNPCore* core = threadContext->userData;
	threadContext->userData = core->threadData;
	core->resetCallback(threadContext);
	threadContext->userData = core;
	_pollInput(core);
}

static void _coreClean(struct mCoreThread* threadContext) {
	struct mNPCore* core = threadContext->userData;
	threadContext->userData = core->threadData;
	core->cleanCallback(threadContext);
	threadContext->userData = core;
}

static void _coreFrame(struct mCoreThread* threadContext) {
	struct mNPCore* core = threadContext->userData;
	threadContext->userData = core->threadData;
	core->frameCallback(threadContext);
	threadContext->userData = core;
	_pollInput(core);
}

static bool _commRecv(struct mNPContext* context) {
	struct mNPPacketHeader header;
	SocketSetBlocking(context->server, false);
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
	if (header.size && header.size < 0x4000000) {
		SocketSetBlocking(context->server, true);
		data = malloc(header.size);
		uint8_t* ptr = data;
		size_t size = header.size;
		while (size) {
			size_t chunkSize = size;
			if (chunkSize > PKT_CHUNK_SIZE) {
				chunkSize = PKT_CHUNK_SIZE;
			}
			ssize_t received = SocketRecv(context->server, ptr, chunkSize);
			if (received < 0 || (size_t) received != chunkSize) {
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
	mNPContextRecv(context, &header, data);
	if (data) {
		free(data);
	}
	return true;
}

static bool _commSend(struct mNPContext* context) {
	struct mNPPacketHeader header;
	uint8_t chunkedData[PKT_CHUNK_SIZE];
	MutexLock(&context->commMutex);
	while (RingFIFORead(&context->commFifo, &header, sizeof(header))) {
		SocketSetBlocking(context->server, true);
		ConditionWake(&context->commFifoFull);
		MutexUnlock(&context->commMutex);
		SocketSend(context->server, &header, sizeof(header));
		while (header.size) {
			size_t chunkSize = header.size;
			if (chunkSize > PKT_CHUNK_SIZE) {
				chunkSize = PKT_CHUNK_SIZE;
			}
			MutexLock(&context->commMutex);
			while (!RingFIFORead(&context->commFifo, chunkedData, chunkSize)) {
				ConditionWake(&context->commFifoFull);
				ConditionWait(&context->commFifoEmpty, &context->commMutex);
			}
			ConditionWake(&context->commFifoFull);
			MutexUnlock(&context->commMutex);
			header.size -= chunkSize;
			SocketSend(context->server, chunkedData, chunkSize);
		}
		if (header.packetType == mNP_PKT_SHUTDOWN) {
			return false;
		}
		MutexLock(&context->commMutex);
	}
	MutexUnlock(&context->commMutex);
	return true;
}

static THREAD_ENTRY _commThread(void* user) {
	ThreadSetName("Netplay Client Thread");
	struct mNPContext* context = user;
	bool running = true;
	while (running) {
		// Receive
		if (!_commRecv(user)) {
			break;
		}

		// Send
		running = _commSend(user);
	}
	SocketClose(context->server);
	ConditionDeinit(&context->commFifoFull);
	ConditionDeinit(&context->commFifoEmpty);
	MutexDeinit(&context->commMutex);

	return 0;
}

void mNPContextSend(struct mNPContext* context, const struct mNPPacketHeader* header, const void* body) {
	MutexLock(&context->commMutex);
	while (!RingFIFOWrite(&context->commFifo, header, sizeof(*header))) {
		ConditionWake(&context->commFifoEmpty);
		ConditionWait(&context->commFifoFull, &context->commMutex);
	}
	MutexUnlock(&context->commMutex);
	size_t size = header->size;
	if (size && body) {
		const uint8_t* ptr = body;
		while (size) {
			size_t chunkSize = size;
			if (chunkSize > PKT_CHUNK_SIZE) {
				chunkSize = PKT_CHUNK_SIZE;
			}
			MutexLock(&context->commMutex);
			while (!RingFIFOWrite(&context->commFifo, ptr, chunkSize)) {
				ConditionWake(&context->commFifoEmpty);
				ConditionWait(&context->commFifoFull, &context->commMutex);
			}
			MutexUnlock(&context->commMutex);
			size -= chunkSize;
			ptr += chunkSize;
		}
	}
}

void mNPContextRecv(struct mNPContext* context, const struct mNPPacketHeader* header, const void* body) {
	// TODO
}
