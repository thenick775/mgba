/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "NetplayView.h"

#include "NetplayController.h"

using namespace QGBA;

NetplayView::NetplayView(NetplayController* controller, QWidget* parent)
	: QWidget(parent)
	, m_netplay(controller)
	, m_coreModel(controller)
{
	m_ui.setupUi(this);

	m_ui.coreView->setModel(&m_coreModel);

	updateStatus();

	connect(m_ui.refresh, SIGNAL(clicked()), &m_coreModel, SLOT(refresh()));

	const QList<mNPRoomInfo>& rooms = controller->rooms();
	if (rooms.count()) {
		m_coreModel.setRoom(rooms[0].roomId);
	}

	connect(&m_coreModel, SIGNAL(modelReset()), this, SLOT(updateItems()));
}

void NetplayView::setActiveController(GameController* controller) {
	m_activeController = controller;
}

void NetplayView::updateStatus() {
	if (m_netplay->connectedToServer()) {
		m_ui.hostname->setEnabled(false);
		m_ui.hostname->setText(m_netplay->connectedHost());
		m_ui.connectButton->setText(tr("Disconnect"));
		m_ui.refresh->setEnabled(true);
	} else {
		m_ui.hostname->setEnabled(true);
		m_ui.connectButton->setText(tr("Connect"));
		m_ui.refresh->setEnabled(false);
	}
}

void NetplayView::updateItems() {
	for (int row = 0; row < m_coreModel.rowCount(); ++row) {
		QPushButton* observe = new QPushButton(tr("Observe"));
		QPushButton* control = new QPushButton(tr("Control"));
		QVariant o = m_coreModel.data(m_coreModel.index(row, 1));
		QVariant c = m_coreModel.data(m_coreModel.index(row, 2));
		observe->setEnabled(o.toBool());
		control->setEnabled(c.toBool());
		m_ui.coreView->setIndexWidget(m_coreModel.index(row, 1), observe);
		m_ui.coreView->setIndexWidget(m_coreModel.index(row, 2), control);
	}
}
