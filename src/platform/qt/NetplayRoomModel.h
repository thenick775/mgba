/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef QGBA_NETPLAY_ROOM_MODEL
#define QGBA_NETPLAY_ROOM_MODEL

#include <QAbstractItemModel>
#include <functional>

#include <mgba/internal/netplay/netplay.h>

namespace QGBA {

class NetplayController;

class NetplayRoomModel : public QAbstractItemModel {
Q_OBJECT

public:
	NetplayRoomModel(NetplayController* controller, QObject* parent = nullptr);

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
	virtual QModelIndex parent(const QModelIndex& index) const override;

	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

public slots:
	void refresh();

private slots:
	void setRooms(const QList<mNPRoomInfo>& cores);

private:
	struct NetplayRoomColumn {
		QString name;
		std::function<QVariant(const mNPRoomInfo&)> value;
	};

	NetplayController* m_controller;

	QList<mNPRoomInfo> m_roomInfo;
	QList<NetplayRoomColumn> m_columns;
};

}

#endif
