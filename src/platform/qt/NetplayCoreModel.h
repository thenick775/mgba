/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef QGBA_NETPLAY_CORE_MODEL
#define QGBA_NETPLAY_CORE_MODEL

#include <QAbstractItemModel>
#include <QMultiMap>

#include <mgba/internal/netplay/netplay.h>

namespace QGBA {

class NetplayController;

class NetplayCoreModel : public QAbstractItemModel {
Q_OBJECT

public:
	NetplayCoreModel(NetplayController* controller, QObject* parent = nullptr);

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex parent(const QModelIndex& index) const override;

	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

public slots:
	void refresh();
	void setRoom(quint32 = 0);

private slots:
	void setCores(const QList<mNPCoreInfo>& cores);

private:
	struct NetplayCoreColumn {
		QString name;
		std::function<QVariant(const mNPCoreInfo&)> value;
	};

	NetplayController* m_controller;
	uint32_t m_room;

	QMultiMap<uint32_t, mNPCoreInfo> m_coreInfo;
	QList<NetplayCoreColumn> m_columns;
};

}

#endif
