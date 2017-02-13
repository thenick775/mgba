/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <mgba/internal/netplay/server.h>

#include "netplay-private.h"

#include <mgba-util/table.h>
#include <mgba-util/threading.h>
#include <mgba-util/vector.h>

mLOG_DEFINE_CATEGORY(NP_SERVER, "Netplay Server")

DECLARE_VECTOR(mNPRequestList, struct mNPPacketRequest);
DEFINE_VECTOR(mNPRequestList, struct mNPPacketRequest);

static THREAD_ENTRY _listenThread(void* context);
static THREAD_ENTRY _clientThread(void* context);
static THREAD_ENTRY _roomThread(void* context);

#define CLIENT_POLL_TIMEOUT 500

struct mNPServer {
	Socket serverSocket;

	uint32_t capacity;
	struct Table clients;
	uint32_t clientCounter;

	uint32_t maxRooms;
	struct Table rooms;
	uint32_t roomCounter;

	uint32_t maxCores;
	struct Table cores;
	uint32_t coreCounter;

	Mutex mutex;
	Thread listenThread;
	bool running;
};

struct mNPClient {
	struct mNPServer* p;
	uint32_t clientId;
	Socket sock;
	struct mNPPacketConnect clientInfo;
	struct Table cores;
	struct mNPRequestList outstandingRequests;
	Mutex mutex;
	struct mNPCommFIFO fifo;
	Thread thread;
};

struct mNPServerCoreInfo {
	struct mNPCoreInfo info;
	struct Table clients;
	struct Table controllingClients;
	uint32_t currentFrame;
};

struct mNPRoom {
	struct mNPServer* p;
	struct mNPRoomInfo info;
	struct Table cores;
	struct Table clients;
	Mutex mutex;
	struct mNPCommFIFO fifo;
	Thread thread;
	struct mNPEventQueue queue;
	struct mNPEventQueue auxQueue;
	uint32_t currentFrame;
	uint32_t nextSync;
	uint32_t coresUntilSync;
};

struct mNPForwardedPacket {
	struct mNPPacketHeader header;
	void* data;
	struct mNPClient* client;
};

struct mNPServer* mNPServerStart(const struct mNPServerOptions* opts) {
	Socket serverSocket = SocketOpenTCP(opts->port, &opts->address);
	if (SOCKET_FAILED(serverSocket)) {
		mLOG(NP_SERVER, ERROR, "Failed to start server on port %u", opts->port);
		return NULL;
	}
	if (SOCKET_FAILED(SocketListen(serverSocket, 10))) {
		mLOG(NP_SERVER, ERROR, "Failed to listen on port %u", opts->port);
		return NULL;
	}
	struct mNPServer* serv = malloc(sizeof(*serv));
	if (!serv) {
		mLOG(NP_SERVER, ERROR, "Out of memory");
		return NULL;
	}
	serv->serverSocket = serverSocket;
	serv->clientCounter = 1;
	serv->capacity = 100;
	TableInit(&serv->clients, 32, free);

	serv->roomCounter = 1;
	serv->maxRooms = 100;
	TableInit(&serv->rooms, 32, free);

	serv->coreCounter = 1;
	serv->maxCores = 100;
	TableInit(&serv->cores, 32, free);

	serv->running = true;

	MutexInit(&serv->mutex);
	ThreadCreate(&serv->listenThread, _listenThread, serv);
	return serv;
}

void mNPServerStop(struct mNPServer* serv) {
	MutexLock(&serv->mutex);
	serv->running = false;
	MutexUnlock(&serv->mutex);
	ThreadJoin(serv->listenThread);
	MutexDeinit(&serv->mutex);
	TableDeinit(&serv->clients);
	SocketClose(serv->serverSocket);
	free(serv);
}

static bool _registerCore(struct mNPClient* client, struct mNPServerCoreInfo* core) {
	struct mNPServer* serv = client->p;
	MutexLock(&serv->mutex);
	if (TableSize(&serv->cores) >= serv->maxCores) {
		MutexUnlock(&serv->mutex);
		return false;
	}
	// TODO: cleanup
	TableInit(&core->clients, 0, NULL);
	TableInit(&core->controllingClients, 0, NULL);
	TableInsert(&core->clients, client->clientId, client);
	TableInsert(&core->controllingClients, client->clientId, client);
	core->info.coreId = serv->coreCounter;
	core->currentFrame = 0;
	TableInsert(&serv->cores, core->info.coreId, core);
	while (TableLookup(&serv->cores, serv->coreCounter)) {
		++serv->coreCounter;
	}
	MutexUnlock(&serv->mutex);
	return true;
}

static void _shutdownClients(uint32_t id, void* value, void* user) {
	UNUSED(id);
	UNUSED(user);
	struct mNPClient* client = value;
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_SHUTDOWN,
		.size = 0,
		.flags = 0
	};
	mNPCommFIFOWrite(&client->fifo, &header, sizeof(header));
	ThreadJoin(client->thread);
}

THREAD_ENTRY _listenThread(void* context) {
	mLOG(NP_SERVER, INFO, "Server started");
	struct mNPServer* serv = context;
	while (true) {
		MutexLock(&serv->mutex);
		if (!serv->running) {
			MutexUnlock(&serv->mutex);
			break;
		}
		MutexUnlock(&serv->mutex);

		Socket reads[1] = { serv->serverSocket };
		SocketPoll(1, reads, NULL, NULL, 200);
		if (SOCKET_FAILED(reads[0])) {
			continue;
		}
		Socket newClient = SocketAccept(serv->serverSocket, NULL);
		SocketSetTCPPush(newClient, 1);
		MutexLock(&serv->mutex);
		if (TableSize(&serv->clients) >= serv->capacity) {
			MutexUnlock(&serv->mutex);
			mNPAck(newClient, mNP_REPLY_FULL);
			SocketClose(newClient);
			continue;
		}
		MutexUnlock(&serv->mutex);

		mLOG(NP_SERVER, INFO, "Client connected");
		struct mNPClient* client = malloc(sizeof(*client));
		client->p = serv;
		SocketSetBlocking(newClient, false);
		client->sock = newClient;
		TableInit(&client->cores, 8, NULL);
		MutexInit(&client->mutex);
		mNPCommFIFOInit(&client->fifo);
		mNPRequestListInit(&client->outstandingRequests, 0);
		memset(&client->clientInfo, 0, sizeof(client->clientInfo));

		MutexLock(&serv->mutex);
		while (!serv->clientCounter || TableLookup(&serv->clients, serv->clientCounter)) {
			++serv->clientCounter;
		}
		client->clientId = serv->clientCounter;
		TableInsert(&serv->clients, serv->clientCounter, client);
		MutexUnlock(&serv->mutex);
		ThreadCreate(&client->thread, _clientThread, client);
	}
	// TODO shutdown rooms
	TableEnumerate(&serv->clients, _shutdownClients, NULL);
	mLOG(NP_SERVER, INFO, "Server shut down");
	return 0;
}

static bool _forwardPacket(struct mNPClient* client, uint32_t coreId, const struct mNPPacketHeader* header, void* data) {
	MutexLock(&client->p->mutex);
	struct mNPServerCoreInfo* core = TableLookup(&client->p->cores, coreId);
	if (!core || !core->info.roomId) {
		MutexUnlock(&client->p->mutex);
		return false;
	}
	struct mNPRoom* room = TableLookup(&client->p->rooms, core->info.roomId);
	MutexUnlock(&client->p->mutex);
	if (!room) {
		return false;
	}
	struct mNPForwardedPacket packet = {
		.header = *header,
		.data = data,
		.client = client
	};
	mNPCommFIFOWrite(&room->fifo, &packet, sizeof(packet));
	return true;
}

static bool _createRoom(struct mNPClient* client, const struct mNPPacketJoin* join) {
	if (!join->capacity) {
		return false;
	}
	MutexLock(&client->p->mutex);
	struct mNPServerCoreInfo* core = TableLookup(&client->p->cores, join->coreId);
	if (!core) {
		MutexUnlock(&client->p->mutex);
		mNPAck(client->sock, mNP_REPLY_NO_SUCH_CORE);
		return true;
	}
	if (core->info.roomId) {
		MutexUnlock(&client->p->mutex);
		mNPAck(client->sock, mNP_REPLY_BUSY);
		return true;
	}
	if (TableSize(&client->p->rooms) >= client->p->maxRooms) {
		MutexUnlock(&client->p->mutex);
		mNPAck(client->sock, mNP_REPLY_FULL);
		return true;
	}
	struct mNPRoom* room = malloc(sizeof(*room));
	room->p = client->p;
	TableInit(&room->cores, 8, NULL);
	TableInit(&room->clients, 8, NULL);
	room->currentFrame = join->currentFrame;
	room->nextSync = join->syncPeriod;
	TableInsert(&room->cores, core->info.coreId, core);
	TableInsert(&room->clients, client->clientId, client);
	room->info.roomId = client->p->roomCounter;
	core->info.roomId = room->info.roomId;
	room->info.nCores = 1;
	memcpy(room->info.requiredCommitHash, client->clientInfo.commitHash, sizeof(room->info.requiredCommitHash));
	room->info.syncPeriod = join->syncPeriod;
	room->info.capacity = join->capacity;
	room->info.flags = join->flags;
	MutexInit(&room->mutex);
	mNPCommFIFOInit(&room->fifo);
	mNPEventQueueInit(&room->queue, 0);
	mNPEventQueueInit(&room->auxQueue, 0);
	room->coresUntilSync = 1;
	TableInsert(&client->p->rooms, room->info.roomId, room);
	while (TableLookup(&client->p->rooms, client->p->roomCounter)) {
		++client->p->roomCounter;
	}
	ThreadCreate(&room->thread, _roomThread, room);
	MutexUnlock(&client->p->mutex);

	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_JOIN,
		.size = sizeof(struct mNPPacketJoin),
		.flags = 0
	};
	struct mNPPacketJoin data = *join;
	data.roomId = room->info.roomId;
	SocketSend(client->sock, &header, sizeof(header));
	SocketSend(client->sock, &data, sizeof(data));
	return true;
}

static bool _processJoin(struct mNPClient* client, const struct mNPPacketHeader* header, struct mNPPacketJoin* join, bool* forwarded) {
	if (sizeof(*join) != header->size || !join) {
		return false;
	}
	if (!join->coreId) {
		return false;
	}
	if (!join->roomId) {
		mLOG(NP_SERVER, INFO, "Client %i core %i creating room", client->clientId, join->coreId);
		return _createRoom(client, join);
	}
	MutexLock(&client->p->mutex);
	struct mNPRoom* room = TableLookup(&client->p->rooms, join->roomId);
	if (!room) {
		MutexUnlock(&client->p->mutex);
		mNPAck(client->sock, mNP_REPLY_DOES_NOT_EXIST);
		return true;
	}
	struct mNPServerCoreInfo* core = TableLookup(&client->p->cores, join->coreId);
	if (!core) {
		MutexUnlock(&client->p->mutex);
		mNPAck(client->sock, mNP_REPLY_NO_SUCH_CORE);
		return true;
	}
	if (core->info.roomId) {
		if (core->info.roomId == join->coreId) {
			MutexUnlock(&client->p->mutex);
			join->currentFrame = room->currentFrame;
			SocketSend(client->sock, header, sizeof(*header));
			SocketSend(client->sock, join, sizeof(*join));
			return true;
		} else {
			MutexUnlock(&client->p->mutex);
			mNPAck(client->sock, mNP_REPLY_BUSY);
			return true;
		}
	}
	MutexUnlock(&client->p->mutex);
	mLOG(NP_SERVER, INFO, "Client %i core %i joining room %i", client->clientId, join->coreId, join->roomId);
	_forwardPacket(client, join->coreId, header, join);
	*forwarded = true;
	return true;
}

static bool _forwardEvent(struct mNPClient* client, const struct mNPPacketHeader* header, struct mNPPacketEvent* event) {
	if (sizeof(*event) != header->size || !event) {
		return false;
	}
	if (!event->event.coreId) {
		return false;
	}
	return _forwardPacket(client, event->event.coreId, header, event);
}

static void _listCores(uint32_t id, void* value, void* user) {
	UNUSED(id);
	struct mNPServerCoreInfo* core = value;
	struct mNPPacketListCores* list = user;
	memcpy(&list->cores[list->nCores], &core->info, sizeof(struct mNPCoreInfo));
	++list->nCores;
}

static void _processListCores(struct mNPClient* client, uint32_t roomId) {
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_LIST,
		.flags = 0
	};
	size_t nCores;
	struct mNPPacketListCores* list;
	struct Table* set = &client->p->cores;
	Mutex* mutex = &client->p->mutex;
	MutexLock(mutex);
	if (roomId) {
		struct mNPRoom* room = TableLookup(&client->p->rooms, roomId);
		if (room) {
			set = &room->cores;
			MutexUnlock(mutex);
			mutex = &room->mutex;
			MutexLock(mutex);
		} else {
			MutexUnlock(mutex);
			set = NULL;
		}
	}
	if (set) {
		nCores = TableSize(set);
		header.size = sizeof(struct mNPPacketListCores) + nCores * sizeof(struct mNPCoreInfo);
		list = malloc(header.size);
		if (list) {
			list->nCores = 0;
			TableEnumerate(set, _listCores, list);
		}
		MutexUnlock(mutex);
	} else {
		header.size = sizeof(struct mNPPacketListCores);
		list = malloc(header.size);
		list->type = mNP_LIST_CORES;
		list->nCores = 0;
	}
	SocketSend(client->sock, &header, sizeof(header));
	SocketSend(client->sock, list, header.size);
}

static void _listRooms(uint32_t id, void* value, void* user) {
	UNUSED(id);
	struct mNPRoom* room = value;
	struct mNPPacketListRooms* list = user;
	memcpy(&list->rooms[list->nRooms], &room->info, sizeof(struct mNPRoomInfo));
	++list->nRooms;
}

static void _processListRooms(struct mNPClient* client) {
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_LIST,
		.flags = 0
	};
	size_t nRooms;
	struct mNPPacketListRooms* list;
	MutexLock(&client->p->mutex);
	nRooms = TableSize(&client->p->rooms);
	header.size = sizeof(struct mNPPacketListRooms) + nRooms * sizeof(struct mNPRoomInfo);
	list = malloc(header.size);
	if (list) {
		list->nRooms = 0;
		list->type = mNP_LIST_ROOMS;
		TableEnumerate(&client->p->rooms, _listRooms, list);
	}
	MutexUnlock(&client->p->mutex);
	SocketSend(client->sock, &header, sizeof(header));
	SocketSend(client->sock, list, header.size);
}

static void _sendPacket(uint32_t id, void* value, void* user) {
	UNUSED(id);
	struct mNPClient* client = value;
	struct mNPForwardedPacket* packet = user;
	if (client == packet->client) {
		// This is the client that initiated the request
		return;
	}
	SocketSend(client->sock, &packet->header, sizeof(packet->header));
	SocketSend(client->sock, packet->data, packet->header.size);
}

static bool _processList(struct mNPClient* client, const struct mNPPacketList* list, size_t size) {
	if (sizeof(*list) != size || !list) {
		return false;
	}
	switch (list->type) {
	case mNP_LIST_CORES:
		_processListCores(client, list->parent);
		break;
	case mNP_LIST_ROOMS:
		_processListRooms(client);
		break;
	default:
		return false;
	}
	return true;
}

static void _sendData(uint32_t id, void* value, void* user) {
	UNUSED(id);
	struct mNPClient* client = value;
	struct mNPForwardedPacket* packet = user;
	struct mNPPacketData* data = packet->data;
	size_t i;
	for (i = 0; i < mNPRequestListSize(&client->outstandingRequests); ++i) {
		struct mNPPacketRequest* req = mNPRequestListGetPointer(&client->outstandingRequests, i);
		if (req->type == data->type && req->coreId == data->coreId) {
			mNPRequestListShift(&client->outstandingRequests, i, 1);
			SocketSend(client->sock, &packet->header, sizeof(packet->header));
			SocketSend(client->sock, data, packet->header.size);
			return;
		}
	}
}

static bool _processData(struct mNPClient* client, struct mNPPacketData* data, size_t size) {
	if (sizeof(*data) > size || !data) {
		return false;
	}
	if (!data->coreId) {
		return false;
	}
	MutexLock(&client->p->mutex);
	struct mNPServerCoreInfo* core = TableLookup(&client->p->cores, data->coreId);
	if (!core) {
		MutexUnlock(&client->p->mutex);
		mNPAck(client->sock, mNP_REPLY_NO_SUCH_CORE);
		return true;
	}
	struct mNPForwardedPacket packet = {
		.header = {
			.packetType = mNP_PKT_DATA,
			.size = size,
			.flags = 0
		},
		.data = data
	};
	TableEnumerate(&core->clients, _sendData, &packet);
	MutexUnlock(&client->p->mutex);
	return true;
}

static bool _processRequest(struct mNPClient* client, struct mNPPacketRequest* req, size_t size) {
	if (sizeof(*req) != size || !req) {
		return false;
	}
	if (!req->coreId) {
		return false;
	}
	MutexLock(&client->p->mutex);
	struct mNPServerCoreInfo* core = NULL;
	switch (req->type) {
	case mNP_DATA_SAVESTATE:
		core = TableLookup(&client->cores, req->coreId);
		break;
	case mNP_DATA_SCREENSHOT:
		core = TableLookup(&client->p->cores, req->coreId);
		if (core && !core->info.flags & mNP_CORE_ALLOW_OBSERVE) {
			MutexUnlock(&client->p->mutex);
			mNPAck(client->sock, mNP_REPLY_DISALLOWED);
			return true;
		}
		break;
	}
	if (!core) {
		MutexUnlock(&client->p->mutex);
		mNPAck(client->sock, mNP_REPLY_NO_SUCH_CORE);
		return true;
	}
	MutexUnlock(&client->p->mutex);

	mLOG(NP_SERVER, INFO, "Client %" PRIi32 " requesting data %" PRIi32 " from core %" PRIi32, client->clientId, req->type, req->coreId);
	struct mNPForwardedPacket packet = {
		.header = {
			.packetType = mNP_PKT_REQUEST,
			.size = size,
			.flags = 0
		},
		.data = req,
		.client = client
	};

	*mNPRequestListAppend(&client->outstandingRequests) = *req;
	TableEnumerate(&core->clients, _sendPacket, &packet);

	return true;
}

static bool _processRegisterCore(struct mNPClient* client, const struct mNPPacketRegisterCore* reg, size_t size) {
	if (sizeof(*reg) != size || !reg) {
		return false;
	}
	struct mNPServerCoreInfo* core = malloc(sizeof(*core));
	memcpy(&core->info, &reg->info, sizeof(core->info));
	if (!_registerCore(client, core)) {
		mNPAck(client->sock, mNP_REPLY_FULL);
		return true;
	}
	TableInsert(&client->cores, core->info.coreId, core);
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_REGISTER_CORE,
		.size = sizeof(struct mNPPacketRegisterCore),
		.flags = 0
	};
	struct mNPPacketRegisterCore data = {
		.info = core->info,
		.nonce = reg->nonce
	};
	mLOG(NP_SERVER, INFO, "Client %" PRIi32 " registering core %" PRIi32, client->clientId, core->info.coreId);
	SocketSend(client->sock, &header, sizeof(header));
	SocketSend(client->sock, &data, sizeof(data));
	return true;
}

static bool _processCloneCore(struct mNPClient* client, const struct mNPPacketCloneCore* clone, size_t size) {
	if (sizeof(*clone) != size || !clone) {
		return false;
	}

	mLOG(NP_SERVER, INFO, "Client %" PRIi32 " cloning core %" PRIi32, client->clientId, clone->coreId);
	MutexLock(&client->p->mutex);
	struct mNPServerCoreInfo* core = TableLookup(&client->p->cores, clone->coreId);
	if (!core) {
		MutexUnlock(&client->p->mutex);
		mNPAck(client->sock, mNP_REPLY_NO_SUCH_CORE);
		return true;
	}
	if ((core->info.flags & clone->flags) != clone->flags) {
		MutexUnlock(&client->p->mutex);
		mNPAck(client->sock, mNP_REPLY_DISALLOWED);
		return true;
	}
	uint32_t roomId = core->info.roomId;
	struct mNPRoom* room = TableLookup(&client->p->rooms, roomId);
	if (!roomId || !room) {
		MutexUnlock(&client->p->mutex);
		mNPAck(client->sock, mNP_REPLY_DOES_NOT_EXIST);
		return true;
	}
	MutexLock(&room->mutex);
	if (TableLookup(&room->clients, client->clientId)) {
		MutexUnlock(&room->mutex);
		MutexUnlock(&client->p->mutex);
		mNPAck(client->sock, mNP_REPLY_BUSY);
		return true;
	}
	TableInsert(&room->clients, client->clientId, client);
	TableInsert(&core->clients, client->clientId, client);
	TableInsert(&client->cores, core->info.coreId, core);
	if (clone->flags & mNP_CORE_ALLOW_CONTROL) {
		TableInsert(&core->controllingClients, client->clientId, client);
	}
	MutexUnlock(&room->mutex);
	MutexUnlock(&client->p->mutex);

	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_REGISTER_CORE,
		.size = sizeof(struct mNPPacketRegisterCore),
		.flags = 0
	};
	struct mNPPacketRegisterCore data = {
		.info = core->info,
		.nonce = clone->nonce
	};
	data.info.flags &= clone->flags;

	SocketSend(client->sock, &header, sizeof(header));
	SocketSend(client->sock, &data, sizeof(data));
	return true;
}

static bool _clientWelcome(struct mNPClient* client) {
	Socket reads[1] = { client->sock };
	Socket errors[1] = { client->sock };
	SocketPoll(1, reads, NULL, errors, CLIENT_POLL_TIMEOUT);
	if (!SOCKET_FAILED(*errors)) {
		mLOG(NP_SERVER, WARN, "Socket error on client %" PRIi32, client->clientId);
		return false;
	}
	if (SOCKET_FAILED(*reads)) {
		return true;
	}

	struct mNPPacketHeader header;
	ssize_t len = SocketRecv(client->sock, &header, sizeof(header));
	if (len < 0 && SocketWouldBlock()) {
		return true;
	}
	if (len != sizeof(header)) {
		mLOG(NP_SERVER, WARN, "Malformed header on client %" PRIi32, client->clientId);
		return false;
	}
	if (header.packetType != mNP_PKT_CONNECT || header.size != sizeof(struct mNPPacketConnect)) {
		mLOG(NP_SERVER, WARN, "Received invalid Connect packet on client %" PRIi32, client->clientId);
		return false;
	}

	reads[0] = client->sock;
	errors[0] = client->sock;
	SocketPoll(1, reads, NULL, errors, CLIENT_POLL_TIMEOUT);
	if (!SOCKET_FAILED(*errors)) {
		mLOG(NP_SERVER, WARN, "Socket error on client %" PRIi32, client->clientId);
		return false;
	}
	if (SOCKET_FAILED(*reads)) {
		mLOG(NP_SERVER, WARN, "Timeout reading Connect packet on client %" PRIi32, client->clientId);
		return false;
	}
	len = SocketRecv(client->sock, &client->clientInfo, sizeof(client->clientInfo));
	if (len < 0) {
		mLOG(NP_SERVER, WARN, "Failed to read Connect packet on client %" PRIi32, client->clientId);
		return false;
	}
	if (len != sizeof(client->clientInfo)) {
		mLOG(NP_SERVER, WARN, "Malformed Connect packet on client %" PRIi32, client->clientId);
		return false;
	}

	mNPAck(client->sock, mNP_REPLY_OK);
	return true;
}

static int _clientRecv(struct mNPClient* client) {
	if (!client->clientInfo.protocolVersion) {
		return _clientWelcome(client) ? 1 : -1;
	}
	Socket reads[1] = { client->sock };
	Socket errors[1] = { client->sock };
	SocketPoll(1, reads, NULL, errors, 4);
	if (!SOCKET_FAILED(*errors)) {
		mLOG(NP_SERVER, WARN, "Socket error on client %" PRIi32, client->clientId);
		return 0;
	}
	if (SOCKET_FAILED(*reads)) {
		return 1;
	}
	struct mNPPacketHeader header;
	ssize_t len = SocketRecv(client->sock, &header, sizeof(header));
	if ((len < 0 && !SocketWouldBlock()) || len != sizeof(header)) {
		mLOG(NP_SERVER, WARN, "Malformed header on client %" PRIi32, client->clientId);
		return 0;
	}
	void* data = NULL;
	if (header.size && header.size <= PKT_CHUNK_SIZE) {
		data = malloc(header.size);
		if (!data) {
			return -1;
		}
		uint8_t* ptr = data;
		size_t size = header.size;
		while (size) {
			reads[0] = client->sock;
			errors[0] = client->sock;
			SocketPoll(1, reads, NULL, errors, CLIENT_POLL_TIMEOUT);
			if (!SOCKET_FAILED(*errors)) {
				free(data);
				return -1;
			}
			if (SOCKET_FAILED(*reads)) {
				break;
			}
			ssize_t len = SocketRecv(client->sock, ptr, size);
			if (len < 0) {
				break;
			}
			ptr += len;
			size -= len;
		};
		if (size) {
			free(data);
			return 0;
		}
	}
	int result = 1;
	bool forwarded = false;
	switch (header.packetType) {
	case mNP_PKT_SHUTDOWN:
		result = -1;
		break;
	case mNP_PKT_JOIN:
		result = _processJoin(client, &header, data, &forwarded);
		break;
	case mNP_PKT_LIST:
		result = _processList(client, data, header.size);
		break;
	case mNP_PKT_DATA:
		result = _processData(client, data, header.size);
		break;
	case mNP_PKT_REQUEST:
		result = _processRequest(client, data, header.size);
		break;
	case mNP_PKT_EVENT:
		result = _forwardEvent(client, &header, data);
		forwarded = result;
		break;
	case mNP_PKT_REGISTER_CORE:
		result = _processRegisterCore(client, data, header.size);
		break;
	case mNP_PKT_CLONE_CORE:
		result = _processCloneCore(client, data, header.size);
		break;
	default:
		result = 0;
		mLOG(NP_SERVER, DEBUG, "Unknown packet type %" PRIi32, header.packetType);
		break;
	}
	if (data) {
		if (!forwarded) {
			free(data);
		}
	} else if (header.size) {
		size_t size = header.size;
		uint8_t buffer[4096];
		while (size) {
			size_t toRead = size;
			if (toRead > sizeof(buffer)) {
				toRead = sizeof(buffer);
			}
			ssize_t len = SocketRecv(client->sock, buffer, toRead);
			if (len <= 0) {
				return -1;
			}
			size -= len;
		}
	}
	return result;
}

static void _cleanCores(uint32_t coreId, void* value, void* user) {
	struct mNPServer* server = user;
	struct mNPServerCoreInfo* core = value;
	if (core->info.roomId) {
		MutexLock(&server->mutex);
		struct mNPRoom* room = TableLookup(&server->rooms, core->info.roomId);
		if (room) {
			MutexLock(&room->mutex);
			TableRemove(&room->cores, core->info.coreId);
			MutexUnlock(&room->mutex);
		}
		MutexUnlock(&server->mutex);
	}
	TableRemove(&server->cores, coreId);
}

static THREAD_ENTRY _clientThread(void* context) {
	struct mNPClient* client = context;
	mLOG(NP_SERVER, DEBUG, "Client %i thread started", client->clientId);
	while (true) {
		int status = _clientRecv(client);
		if (!status) {
			mNPAck(client->sock, mNP_REPLY_MALFORMED);
		} else if (status < 0) {
			// Received shutdown packet or a critical error
			break;
		}

		if (!mNPCommFIFOFlush(&client->fifo, client->sock)) {
			// We got a SHUTDOWN packet
			break;
		}
	}
	mLOG(NP_SERVER, DEBUG, "Client %i thread exited", client->clientId);
	SocketClose(client->sock);

	MutexDeinit(&client->mutex);
	mNPCommFIFODeinit(&client->fifo);

	// The client is deleted here so we have to grab the server pointer first
	struct mNPServer* server = client->p;
	TableEnumerate(&client->cores, _cleanCores, server);
	MutexLock(&server->mutex);
	TableDeinit(&client->cores);
	TableRemove(&server->clients, client->clientId);
	MutexUnlock(&server->mutex);

	return 0;
}

struct mNPServerEnumerateData {
	void* data;
	size_t size;
};

static void _sendToClients(uint32_t clientId, void* value, void* user) {
	UNUSED(clientId);
	struct mNPServerEnumerateData* packet = user;
	struct mNPClient* client = value;
	mNPCommFIFOWrite(&client->fifo, packet->data, packet->size);
}

static void _processAuxQueue(struct mNPRoom* room) {
	size_t i = 0;
	while (mNPEventQueueSize(&room->queue) > i) {
		struct mNPEvent* event = mNPEventQueueGetPointer(&room->queue, i);
		int32_t cond = room->nextSync - event->frameId;
		if (cond >= 0) {
			*mNPEventQueueAppend(&room->queue) = *event;
			if (event->eventType == mNP_EVENT_FRAME) {
				if (room->currentFrame + 1 == event->frameId) {
					room->currentFrame = event->frameId;
				}
				if (room->nextSync == event->frameId) {
					--room->coresUntilSync;
				}
			}
			mNPEventQueueShift(&room->queue, i, 1);
		} else {
			++i;
		}
	}
}

static void _sendSync(struct mNPRoom* room) {
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_SYNC,
		.flags = 0
	};
	size_t nEvents = mNPEventQueueSize(&room->queue);
	header.size = sizeof(struct mNPPacketSync) + nEvents * sizeof(struct mNPEvent);
	struct mNPServerEnumerateData packet = {
		.data = &header,
		.size = sizeof(header)
	};
	TableEnumerate(&room->clients, _sendToClients, &packet);

	struct mNPPacketSync sync = {
		.nEvents = nEvents,
	};
	packet.data = &sync;
	packet.size = sizeof(sync);
	TableEnumerate(&room->clients, _sendToClients, &packet);

	size_t i;
	for (i = 0; i < nEvents; ++i) {
		struct mNPEvent* event = mNPEventQueueGetPointer(&room->queue, i);
		struct mNPServerEnumerateData eventDatum = {
			.data = event,
			.size = sizeof(*event)
		};
		TableEnumerate(&room->clients, _sendToClients, &eventDatum);
	}
	mNPEventQueueClear(&room->queue);
	MutexLock(&room->mutex);
	room->coresUntilSync = TableSize(&room->cores);
	MutexUnlock(&room->mutex);
	room->nextSync += room->info.syncPeriod;
	_processAuxQueue(room);
}

static void _processEvent(struct mNPRoom* room, const struct mNPForwardedPacket* packet) {
	const struct mNPPacketEvent* eventPacket = packet->data;
	const struct mNPEvent* event = &eventPacket->event;
	// Check if we're past the sync point safely taking into account integer overflow
	// E.g, if nextSync == 1 and currentFrame = 2, this amounts to UINT32_MAX,
	// which gets coerced to -1. If nextSync == 2 and currentFrame == 1, this equals
	// 1, which is >= 0. This relies on the target architecture being two's-complement.
	int32_t cond = room->nextSync - room->currentFrame;
	if (cond >= 0) {
		*mNPEventQueueAppend(&room->queue) = *event;
		if (event->eventType == mNP_EVENT_FRAME) {
			if (room->currentFrame + 1 == event->frameId) {
				room->currentFrame = event->frameId;
			}
			if (room->nextSync == event->frameId) {
				--room->coresUntilSync;
			}
		}
	} else {
		*mNPEventQueueAppend(&room->auxQueue) = *event;
	}

	if (!room->coresUntilSync) {
		_sendSync(room);
	}
}

static void _processRoomJoin(struct mNPRoom* room, const struct mNPForwardedPacket* packet) {
	const struct mNPPacketJoin* join = packet->data;
	// TODO
}

static THREAD_ENTRY _roomThread(void* context) {
	struct mNPRoom* room = context;
	mLOG(NP_SERVER, DEBUG, "Room %i thread started", room->info.roomId);
	while (true) {
		struct mNPForwardedPacket packet;
		while (mNPCommFIFOPoll(&room->fifo, 10)) {
			mNPCommFIFORead(&room->fifo, &packet, sizeof(packet));
			mLOG(NP_SERVER, DEBUG, "Room %i received packet of type %i", room->info.roomId, packet.header.packetType);
			switch (packet.header.packetType) {
			case mNP_PKT_EVENT:
				_processEvent(room, &packet);
				break;
			case mNP_PKT_JOIN:
				_processRoomJoin(room, &packet);
				break;
			default:
				mLOG(NP_SERVER, WARN, "Room %i received unknown packet type %" PRIi32, room->info.roomId, packet.header.packetType);
			}
			if (packet.data) {
				free(packet.data);
			}
		}

		MutexLock(&room->mutex);
		if (!TableSize(&room->cores)) {
			MutexUnlock(&room->mutex);
			// All of the cores shut down
			break;
		}
		MutexUnlock(&room->mutex);
	}
	// TODO
	mLOG(NP_SERVER, DEBUG, "Room %i thread exited", room->info.roomId);

	return 0;
}
