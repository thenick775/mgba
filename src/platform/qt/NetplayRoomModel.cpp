/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "NetplayRoomModel.h"

#include "NetplayController.h"

using namespace QGBA;

NetplayRoomModel::NetplayRoomModel(NetplayController* controller, QObject* parent)
	: QAbstractItemModel(parent)
	, m_controller(controller)
{
	m_columns.append({
		tr("ID"),
		[](const mNPRoomInfo& info) -> QVariant {
			return info.roomId;
		}
	});

	refresh();
}

QVariant NetplayRoomModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid()) {
		return QVariant();
	}
	if (index.row() >= m_roomInfo.size()) {
		return QVariant();
	}
	if (index.column() >= m_columns.count()) {
		return QVariant();
	}
	const mNPRoomInfo& room = m_roomInfo[index.row()];

	switch (role) {
	case Qt::DisplayRole:
		return m_columns[index.column()].value(room);
	default:
		return QVariant();
	}
}

QVariant NetplayRoomModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (role != Qt::DisplayRole) {
		return QAbstractItemModel::headerData(section, orientation, role);
	}
	if (orientation == Qt::Horizontal) {
		if (section >= m_columns.count()) {
			return QVariant();
		}
		return m_columns[section].name;
	}
	return section;
}

QModelIndex NetplayRoomModel::index(int row, int column, const QModelIndex& parent) const {
	if (parent.isValid()) {
		return QModelIndex();
	}
	return createIndex(row, column, nullptr);
}

QModelIndex NetplayRoomModel::parent(const QModelIndex& index) const {
	return QModelIndex();
}

int NetplayRoomModel::columnCount(const QModelIndex& parent) const {
	return m_columns.count();
}

int NetplayRoomModel::rowCount(const QModelIndex& parent) const {
 	return m_roomInfo.count();
}

void NetplayRoomModel::refresh() {
	m_controller->listRooms([this](const QList<mNPRoomInfo>& rooms) {
		QMetaObject::invokeMethod(this, "setRooms", Q_ARG(const QList<mNPRoomInfo>&, rooms));
	});
}

void NetplayRoomModel::setRooms(const QList<mNPRoomInfo>& rooms) {
	beginResetModel();
	m_roomInfo = rooms;
	endResetModel();
}
