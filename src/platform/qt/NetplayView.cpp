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
	, m_roomModel(controller)
{
	m_ui.setupUi(this);

	m_ui.coreView->setModel(&m_coreModel);
	m_ui.roomView->setModel(&m_roomModel);

	updateStatus();

	connect(m_ui.refresh, SIGNAL(clicked()), &m_coreModel, SLOT(refresh()));
	connect(m_ui.refresh, SIGNAL(clicked()), &m_roomModel, SLOT(refresh()));

	connect(m_ui.create, &QAbstractButton::clicked, [this]() {
		if (!m_activeController) {
			return;
		}
		m_netplay->joinRoom(m_activeController);
	});

	connect(m_ui.join, &QAbstractButton::clicked, [this]() {
		if (!m_activeController) {
			return;
		}
		QModelIndex index = m_ui.roomView->selectionModel()->currentIndex();
		if (!index.isValid()) {
			return;
		}
		uint32_t id = m_roomModel.data(index, Qt::DisplayRole).toUInt();
		m_netplay->joinRoom(m_activeController, id);
	});

	connect(m_ui.roomView, &QAbstractItemView::clicked, [this](const QModelIndex& index) {
		m_ui.join->setEnabled(true);
		m_ui.leave->setEnabled(true);
		uint32_t id = m_roomModel.data(index, Qt::DisplayRole).toUInt();
		QModelIndex newRoot = m_coreModel.index(id, 0, QModelIndex());
		m_ui.coreView->setRootIndex(newRoot);
	});

	connect(m_ui.coreView, &QAbstractItemView::clicked, [this](const QModelIndex& index) {
		uint32_t id = m_coreModel.data(index, Qt::DisplayRole).toUInt();
		// TODO
	});
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
		m_ui.create->setEnabled(true);
	} else {
		m_ui.hostname->setEnabled(true);
		m_ui.connectButton->setText(tr("Connect"));
		m_ui.refresh->setEnabled(false);
		m_ui.create->setEnabled(false);
		m_ui.join->setEnabled(false);
		m_ui.leave->setEnabled(false);
		m_ui.observe->setEnabled(false);
		m_ui.control->setEnabled(false);
	}
}
