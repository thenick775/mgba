/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef QGBA_NETPLAY_VIEW
#define QGBA_NETPLAY_VIEW

#include "ui_NetplayView.h"

#include "NetplayCoreModel.h"

namespace QGBA {

class GameController;
class NetplayController;

class NetplayView : public QWidget {
Q_OBJECT

public:
	NetplayView(NetplayController*, QWidget* parent = nullptr);

	void setActiveController(GameController* activeController);

public slots:
	void updateStatus();

private slots:
	void updateItems();
	void connectToServer();

private:
	Ui::NetplayView m_ui;

	NetplayController* m_netplay;
	NetplayCoreModel m_coreModel;

	GameController* m_activeController;
};

}

#endif
