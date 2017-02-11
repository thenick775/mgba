/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "NetplayCoreModel.h"

#include "NetplayController.h"

using namespace QGBA;

NetplayCoreModel::NetplayCoreModel(NetplayController* controller, QObject* parent)
	: QAbstractItemModel(parent)
	, m_controller(controller)
{
	m_columns.append({
		tr("ID"),
		[](const mNPCoreInfo& info) -> QVariant {
			return info.coreId;
		}
	});

	refresh();
}

QVariant NetplayCoreModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid()) {
		return QVariant();
	}
	if (index.row() >= m_coreInfo.size()) {
		return QVariant();
	}
	if (index.column() >= m_columns.count()) {
		return QVariant();
	}
	const QList<mNPCoreInfo>& cores = m_coreInfo.values(index.parent().row());

	switch (role) {
	case Qt::DisplayRole:
		return m_columns[index.column()].value(cores[index.row()]);
	default:
		return QVariant();
	}
}

QVariant NetplayCoreModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

QModelIndex NetplayCoreModel::index(int row, int column, const QModelIndex& parent) const {
	if (parent.isValid()) {
		return createIndex(row, column, parent.row());
	} else {
		return createIndex(row, column, 0ULL);
	}
}

QModelIndex NetplayCoreModel::parent(const QModelIndex& index) const {
	if (!index.isValid() || !index.internalId()) {
		return QModelIndex();
	}
	return createIndex(index.internalId(), 0, 0ULL);
}

int NetplayCoreModel::columnCount(const QModelIndex& parent) const {
	if (!parent.isValid()) {
		return 1;
	}
	return m_columns.count();
}

int NetplayCoreModel::rowCount(const QModelIndex& parent) const {
	if (!parent.isValid()) {
		return 0;
 	}
 	return m_coreInfo.count(parent.row());
}

void NetplayCoreModel::refresh() {
	m_controller->listCores([this](const QList<mNPCoreInfo>& cores) {
		QMetaObject::invokeMethod(this, "setCores", Q_ARG(const QList<mNPCoreInfo>&, cores));
	});
}

void NetplayCoreModel::setCores(const QList<mNPCoreInfo>& cores) {
	beginResetModel();
	m_coreInfo.clear();
	for (const auto& info : cores) {
		if (info.roomId) {
			m_coreInfo.insert(info.roomId, info);
		}
	}
	endResetModel();
}
