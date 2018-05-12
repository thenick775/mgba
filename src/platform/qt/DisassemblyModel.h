/* Copyright (c) 2013-2018 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#pragma once

#include "CoreController.h"

#ifdef M_CORE_GBA
#include <mgba/internal/arm/decoder.h>
#endif
#ifdef M_CORE_GB
#include <mgba/internal/lr35902/decoder.h>
#endif

#include <QAbstractItemModel>

#include <memory>

namespace QGBA {

class DisassemblyModel : public QAbstractItemModel {
Q_OBJECT

public:
	void setController(std::shared_ptr<CoreController>);

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
	virtual QModelIndex parent(const QModelIndex& index) const override;

	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

	QModelIndex addressToIndex(uint32_t) const;

public slots:
	void refresh(bool ro = false);

private:
	struct Instruction {
		uint32_t address;
		uint32_t nRaw;
	};

	struct InstructionBlock {
		QMap<int, QVector<Instruction>> mapping;
		bool isMirror = false;
		uint32_t mirror;
		const void* memory;
		size_t memsize;
	};

	Instruction indexToInstruction(int, const uint8_t** mem) const;

	QString disassemble(const Instruction&, const uint8_t* mem) const;

	void refreshBlock(const mCoreMemoryBlock&);

#ifdef M_CORE_GB
	void refreshLR35902(const mCoreMemoryBlock&);
#endif
#ifdef M_CORE_GBA
	void refreshARM(const mCoreMemoryBlock&);
#endif

	QMap<uint32_t, InstructionBlock> m_instructionBlocks;

	std::shared_ptr<CoreController> m_controller;
	int m_currentMapping = 0;
};

}
