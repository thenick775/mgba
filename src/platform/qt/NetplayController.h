/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef QGBA_NETPLAY_CONTROLLER
#define QGBA_NETPLAY_CONTROLLER

#include <QObject>

struct mNPContext;
struct mNPServer;
struct mNPServerOptions;

namespace QGBA {

class MultiplayerController;

class NetplayController : public QObject {
Q_OBJECT

public:
	NetplayController(MultiplayerController* mp, QObject* parent = NULL);
	~NetplayController();

	bool startServer(const mNPServerOptions& opts);
	void stopServer();

	bool connectToServer(const mNPServerOptions& opts);
	void disconnectFromServer();

private:
	MultiplayerController* m_multiplayer;
	mNPContext* m_np;
	mNPServer* m_server;
	mNPContext* m_client;
};

}

#endif
