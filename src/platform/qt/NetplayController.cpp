/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "NetplayController.h"

#include <mgba/internal/netplay/server.h>

#include "GameController.h"

using namespace QGBA;

const mNPCallbacks NetplayController::s_callbacks = {
	NetplayController::cbServerShutdown,
	NetplayController::cbCoreRegistered,
	NetplayController::cbRoomJoined
};

NetplayController::NetplayController(MultiplayerController* mp, QObject* parent)
	: QObject(parent)
	, m_multiplayer(mp)
	, m_np(nullptr)
	, m_server(nullptr)
{
}

NetplayController::~NetplayController() {
	disconnectFromServer();
	stopServer();
}

bool NetplayController::startServer(const mNPServerOptions& opts) {
	if (m_server) {
		return false;
	}
	m_server = mNPServerStart(&opts);
	if (!m_server) {
		return false;
	}
	if (!connectToServer(opts)) {
		stopServer();
		return false;
	}
	return true;
}

void NetplayController::stopServer() {
	if (!m_server) {
		return;
	}
	mNPServerStop(m_server);
	m_server = nullptr;
}

bool NetplayController::connectToServer(const mNPServerOptions& opts) {
	if (m_np) {
		return false;
	}
	m_np = mNPContextCreate();
	mNPContextAttachCallbacks(m_np, &s_callbacks, this);
	if (!mNPContextConnect(m_np, &opts)) {
		mNPContextDestroy(m_np);
		m_np = nullptr;
		return false;
	}
	return true;
}

void NetplayController::disconnectFromServer() {
	if (!m_np) {
		return;
	}
	mNPContextDisconnect(m_np);
	mNPContextDestroy(m_np);
	m_cores.clear();
	m_pendingCores.clear();
	m_np = nullptr;
}

void NetplayController::addGameController(GameController* controller, uint32_t id) {
	if (!m_np || !controller->isLoaded()) {
		return;
	}
	if (!id) {
		uint32_t nonce = qrand();
		while (m_pendingCores.contains(nonce)) {
			nonce = qrand();
		}
		m_pendingCores[nonce] = controller;
		mNPContextRegisterCore(m_np, controller->thread(), nonce);
		return;
	}
	mNPContextAttachCore(m_np, controller->thread(), id);
}

void NetplayController::addGameController(uint32_t nonce, uint32_t id) {
	if (!m_np) {
		return;
	}
	GameController* controller = m_pendingCores.take(nonce);
	addGameController(controller, id);
}

void NetplayController::cbServerShutdown(mNPContext* context, void* user) {
	QMetaObject::invokeMethod(static_cast<NetplayController*>(user), "stopServer");
}

void NetplayController::cbCoreRegistered(mNPContext* context, const mNPCoreInfo* info, uint32_t nonce, void* user) {
	QMetaObject::invokeMethod(static_cast<NetplayController*>(user), "addGameController", Q_ARG(quint32, nonce), Q_ARG(quint32, info->coreId));
}

void NetplayController::cbRoomJoined(mNPContext* context, uint32_t roomId, uint32_t coreId, void* user) {

}
