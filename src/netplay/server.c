/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <mgba/internal/netplay/server.h>

#include "netplay-private.h"

#include <mgba-util/table.h>
#include <mgba-util/threading.h>

mLOG_DEFINE_CATEGORY(NP_SERVER, "Netplay Server")

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
	Mutex mutex;
	struct mNPCommFIFO fifo;
	Thread thread;
};

struct mNPRoom {
	struct mNPServer* p;
	struct mNPRoomInfo info;
	struct Table cores;
	Mutex mutex;
	struct mNPCommFIFO fifo;
	Thread thread;
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

static bool _registerCore(struct mNPServer* serv, struct mNPCoreInfo* info) {
	MutexLock(&serv->mutex);
	if (TableSize(&serv->cores) >= serv->maxCores) {
		MutexUnlock(&serv->mutex);
		return false;
	}
	info->coreId = serv->coreCounter;
	TableInsert(&serv->cores, info->coreId, info);
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
	struct mNPCoreInfo* core = TableLookup(&client->p->cores, coreId);
	if (!core || !core->roomId) {
		MutexUnlock(&client->p->mutex);
		return false;
	}
	struct mNPRoom* room = TableLookup(&client->p->rooms, core->roomId);
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
	struct mNPCoreInfo* core = TableLookup(&client->p->cores, join->coreId);
	if (!core) {
		MutexUnlock(&client->p->mutex);
		mNPAck(client->sock, mNP_REPLY_NO_SUCH_CORE);
		return true;
	}
	if (core->roomId) {
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
	TableInsert(&room->cores, core->coreId, core);
	room->info.roomId = client->p->roomCounter;
	core->roomId = room->info.roomId;
	room->info.nCores = 1;
	memcpy(room->info.requiredCommitHash, client->clientInfo.commitHash, sizeof(room->info.requiredCommitHash));
	room->info.syncPeriod = join->syncPeriod;
	room->info.capacity = join->capacity;
	room->info.flags = join->flags;
	MutexInit(&room->mutex);
	mNPCommFIFOInit(&room->fifo);
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

static bool _processJoin(struct mNPClient* client, const struct mNPPacketJoin* join, size_t size) {
	if (sizeof(*join) != size || !join) {
		return false;
	}
	if (!join->coreId) {
		return false;
	}
	if (!join->roomId) {
		mLOG(NP_SERVER, INFO, "Client %i core %i creating room", client->clientId, join->coreId);
		return _createRoom(client, join);
	}
	mLOG(NP_SERVER, INFO, "Client %i core %i joining room %i", client->clientId, join->coreId, join->roomId);
	// TODO
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
	struct mNPPacketListCores* list = user;
	memcpy(&list->cores[list->nCores], value, sizeof(struct mNPCoreInfo));
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
	if (roomId) {
		struct mNPRoom* room = TableLookup(&client->p->rooms, roomId);
		if (room) {
			set = &room->cores;
			mutex = &room->mutex;
		} else {
			set = NULL;
		}
	}
	if (set) {
		MutexLock(mutex);
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
		list->nCores = 0;
	}
	SocketSend(client->sock, &header, sizeof(header));
	SocketSend(client->sock, list, header.size);
}

static void _listRooms(uint32_t id, void* value, void* user) {
	UNUSED(id);
	struct mNPPacketListRooms* list = user;
	memcpy(&list->rooms[list->nRooms], value, sizeof(struct mNPRoomInfo));
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
		TableEnumerate(&client->p->rooms, _listRooms, list);
	}
	MutexUnlock(&client->p->mutex);
	SocketSend(client->sock, &header, sizeof(header));
	SocketSend(client->sock, list, header.size);
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

static bool _processRegisterCore(struct mNPClient* client, const struct mNPPacketRegisterCore* reg, size_t size) {
	if (sizeof(*reg) != size || !reg) {
		return false;
	}
	struct mNPCoreInfo* info = malloc(sizeof(*info));
	memcpy(info, &reg->info, sizeof(*info));
	if (!_registerCore(client->p, info)) {
		mNPAck(client->sock, mNP_REPLY_FULL);
		return true;
	}
	TableInsert(&client->cores, info->coreId, info);
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_REGISTER_CORE,
		.size = sizeof(struct mNPPacketRegisterCore),
		.flags = 0
	};
	struct mNPPacketRegisterCore data = {
		.info = *info,
		.nonce = reg->nonce
	};
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
		result = _processJoin(client, data, header.size);
		break;
	case mNP_PKT_LIST:
		result = _processList(client, data, header.size);
		break;
	case mNP_PKT_EVENT:
		result = _forwardEvent(client, &header, data);
		forwarded = result;
		break;
	case mNP_PKT_REGISTER_CORE:
		result = _processRegisterCore(client, data, header.size);
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
	struct mNPCoreInfo* info = value;
	if (info->roomId) {
		MutexLock(&server->mutex);
		struct mNPRoom* room = TableLookup(&server->rooms, info->roomId);
		MutexUnlock(&server->mutex);
		if (room) {
			MutexLock(&room->mutex);
			TableRemove(&room->cores, info->coreId);
			MutexUnlock(&room->mutex);
		}
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

static void _processEvent(struct mNPRoom* room, const struct mNPForwardedPacket* packet) {

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
