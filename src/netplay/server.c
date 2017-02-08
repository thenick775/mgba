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
	struct RingFIFO fifo;
	Condition fifoFull;
	Condition fifoEmpty;
	Thread thread;
	bool running;
};

struct mNPRoom {
};

struct mNPServer* mNPServerStart(const struct mNPServerOptions* opts) {
	Socket serverSocket = SocketOpenTCP(opts->port, &opts->address);
	if (SOCKET_FAILED(serverSocket)) {
		mLOG(NP_SERVER, ERROR, "Failed to start server on port %u", opts->port);
		return NULL;
	}
	if (SOCKET_FAILED(SocketListen(serverSocket, 0))) {
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

static void _registerCore(struct mNPServer* serv, struct mNPCoreInfo* info) {
	MutexLock(&serv->mutex);
	info->coreId = serv->coreCounter;
	TableInsert(&serv->cores, info->coreId, info);
	while (TableLookup(&serv->cores, serv->coreCounter)) {
		++serv->coreCounter;
	}
	MutexUnlock(&serv->mutex);
}

static void _shutdownClients(uint32_t id, void* value, void* user) {
	UNUSED(id);
	UNUSED(user);
	struct mNPClient* client = value;
	MutexLock(&client->mutex);
	client->running = false;
	MutexUnlock(&client->mutex);
	ThreadJoin(client->thread);
}

THREAD_ENTRY _listenThread(void* context) {
	mLOG(NP_SERVER, INFO, "Server started");
	struct mNPServer* serv = context;
	SocketSetBlocking(serv->serverSocket, false);
	while (true) {
		MutexLock(&serv->mutex);
		if (!serv->running) {
			MutexUnlock(&serv->mutex);
			break;
		}
		MutexUnlock(&serv->mutex);

		Socket reads[1] = { serv->serverSocket };
		SocketPoll(1, reads, NULL, NULL, 50);
		Socket newClient = SocketAccept(serv->serverSocket, NULL);
		if (SOCKET_FAILED(newClient)) {
			continue;
		}
		SocketSetTCPPush(newClient, 1);
		MutexLock(&serv->mutex);
		if (TableSize(&serv->clients) >= serv->capacity) {
			MutexUnlock(&serv->mutex);
			struct mNPPacketHeader header = {
				.packetType = mNP_PKT_ACK,
				.size = sizeof(struct mNPPacketAck),
				.flags = 0
			};
			struct mNPPacketAck ack = {
				.reply = mNP_REPLY_FULL
			};
			SocketSend(newClient, &header, sizeof(header));
			SocketSend(newClient, &ack, sizeof(ack));
			SocketClose(newClient);
			continue;
		}
		MutexUnlock(&serv->mutex);

		mLOG(NP_SERVER, INFO, "Client connected");
		struct mNPClient* client = malloc(sizeof(*client));
		client->p = serv;
		client->sock = newClient;
		TableInit(&client->cores, 8, NULL);
		MutexInit(&client->mutex);
		RingFIFOInit(&client->fifo, COMM_FIFO_SIZE);
		ConditionInit(&client->fifoFull);
		ConditionInit(&client->fifoEmpty);
		memset(&client->clientInfo, 0, sizeof(client->clientInfo));
		client->running = true;

		MutexLock(&serv->mutex);
		while (!serv->clientCounter || TableLookup(&serv->clients, serv->clientCounter)) {
			++serv->clientCounter;
		}
		client->clientId = serv->clientCounter;
		TableInsert(&serv->clients, serv->clientCounter, client);
		MutexUnlock(&serv->mutex);
		ThreadCreate(&client->thread, _clientThread, client);
	}
	TableEnumerate(&serv->clients, _shutdownClients, NULL);
	mLOG(NP_SERVER, INFO, "Server shut down");
	return 0;
}

static bool _processJoin(struct mNPClient* client, const struct mNPPacketJoin* join, size_t size) {
	if (sizeof(*join) != size || !join) {
		return false;
	}
	// TODO
	return true;
}

static bool _processRegisterCore(struct mNPClient* client, const struct mNPPacketRegisterCore* reg, size_t size) {
	if (sizeof(*reg) != size || !reg) {
		return false;
	}
	struct mNPCoreInfo* info = malloc(sizeof(*info));
	memcpy(info, &reg->info, sizeof(*info));
	_registerCore(client->p, info);
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
	return true;
}

static int _clientRecv(struct mNPClient* client) {
	SocketSetBlocking(client->sock, false);
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
	switch (header.packetType) {
	case mNP_PKT_SHUTDOWN:
		result = -1;
		break;
	case mNP_PKT_JOIN:
		result = _processJoin(client, data, header.size);
		break;
	case mNP_PKT_REGISTER_CORE:
		result = _processRegisterCore(client, data, header.size);
		break;
	default:
		mLOG(NP_SERVER, DEBUG, "Unknown packet type %" PRIi32, header.packetType);
		result = 0;
		break;
	}
	if (data) {
		free(data);
	} else if (header.size) {
		// TODO: skip data
	}
	return result;
}

static bool _clientSend(struct mNPClient* client) {
	// TODO
	return true;
}

static THREAD_ENTRY _clientThread(void* context) {
	mLOG(NP_SERVER, DEBUG, "Client thread started");
	struct mNPClient* client = context;
	while (true) {
		MutexLock(&client->mutex);
		if (!client->running) {
			MutexUnlock(&client->mutex);
			break;
		}
		MutexUnlock(&client->mutex);

		int status = _clientRecv(client);
		if (!status) {
			struct mNPPacketHeader header = {
				.packetType = mNP_PKT_ACK,
				.size = sizeof(struct mNPPacketAck),
				.flags = 0
			};
			struct mNPPacketAck ack = {
				.reply = mNP_REPLY_MALFORMED
			};
			SocketSend(client->sock, &header, sizeof(header));
			SocketSend(client->sock, &ack, sizeof(ack));
			break;
		}
		if (status < 0) {
			// Received shutdown packet or a critical error
			break;
		}

		if (!_clientSend(client)) {
			break;
		}
	}
	mLOG(NP_SERVER, DEBUG, "Client thread exited");
	struct mNPPacketHeader header = {
		.packetType = mNP_PKT_SHUTDOWN,
		.size = 0,
		.flags = 0
	};
	SocketSend(client->sock, &header, sizeof(header));
	SocketClose(client->sock);

	TableDeinit(&client->cores);
	MutexDeinit(&client->mutex);
	RingFIFODeinit(&client->fifo);
	ConditionDeinit(&client->fifoFull);
	ConditionDeinit(&client->fifoEmpty);

	// The client is deleted here so we have to grab the server pointer first
	struct mNPServer* server = client->p;
	MutexLock(&server->mutex);
	TableRemove(&server->clients, client->clientId);
	MutexUnlock(&server->mutex);

	return 0;
}
