/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "NetplayController.h"

#include <mgba/internal/netplay/netplay.h>
#include <mgba/internal/netplay/server.h>

using namespace QGBA;

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
	m_np = nullptr;
}
