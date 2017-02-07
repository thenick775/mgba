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

struct mNPServer* mNPServerStart(const struct mNPServerOptions* opts) {
	Socket serverSocket = SocketOpenTCP(opts->port, &opts->address);
	if (SOCKET_FAILED(serverSocket)) {
		return NULL;
	}
	if (SOCKET_FAILED(SocketListen(serverSocket, 0))) {
		return NULL;
	}
	struct mNPServer* serv = malloc(sizeof(*serv));
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
		MutexLock(&serv->mutex);
		if (TableSize(&serv->clients) >= serv->capacity) {
			MutexUnlock(&serv->mutex);
			struct mNPPacketHeader header = {
				.packetType = mNP_PKT_ACK,
				.size = sizeof(struct mNPPacketAck),
				.flags = 0,
				.coreId = 0
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
		TableInit(&client->cores, 8, free);
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

static int _clientRecv(struct mNPClient* client) {
	Socket reads[1] = { client->sock };
	Socket errors[1] = { client->sock };
	SocketSetBlocking(client->sock, false);
	if (!client->clientInfo.protocolVersion) {
		SocketPoll(1, reads, NULL, errors, 250);
		if (!SOCKET_FAILED(*errors)) {
			return 0;
		}
		if (SOCKET_FAILED(*reads)) {
			return 0;
		}
		struct mNPPacketHeader header;
		ssize_t len = SocketRecv(client->sock, &header, sizeof(header));
		if (len < 0 && !SocketWouldBlock()) {
			return 0;
		}
		if (len != sizeof(header)) {
			return 0;
		}
		if (header.packetType != mNP_PKT_CONNECT || header.size != sizeof(struct mNPPacketConnect)) {
			return 0;
		}
		
		len = SocketRecv(client->sock, &client->clientInfo, sizeof(client->clientInfo));
		if (len < 0 && !SocketWouldBlock()) {
			return 0;
		}
		if (len != sizeof(client->clientInfo)) {
			return 0;
		}
		return 1;
	}
	SocketPoll(1, reads, NULL, errors, 4);
	if (!SOCKET_FAILED(*errors)) {
		return 0;
	}
	if (SOCKET_FAILED(*reads)) {
		return 1;
	}
	struct mNPPacketHeader header;
	ssize_t len = SocketRecv(client->sock, &header, sizeof(header));
	if (len < 0 && !SocketWouldBlock()) {
		return 0;
	}
	if (len != sizeof(header)) {
		return 0;
	}
	switch (header.packetType) {
	case mNP_PKT_SHUTDOWN:
		return -1;
	default:
		mLOG(NP_SERVER, DEBUG, "Unknown packet type %" PRIi32, header.packetType);
		// TODO: skip data
		return 0;
	}
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
				.flags = 0,
				.coreId = 0
			};
			struct mNPPacketAck ack = {
				.reply = mNP_REPLY_MALFORMED
			};
			SocketSend(client->sock, &header, sizeof(header));
			SocketSend(client->sock, &ack, sizeof(ack));
			break;
		}
		if (status < 0) {
			// Received shutdown packet
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
		.flags = 0,
		.coreId = 0
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
