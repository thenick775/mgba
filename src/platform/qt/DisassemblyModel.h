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

#include <QAbstractScrollArea>
#include <QMap>
#include <QVector>

#include <memory>

namespace QGBA {

class DisassemblyModel : public QAbstractScrollArea {
Q_OBJECT

public:
	DisassemblyModel(QWidget* parent = nullptr);

	void setController(std::shared_ptr<CoreController>);

public slots:
	void jumpToAddress(const QString& hex);
	void jumpToAddress(uint32_t);
	void jumpToPc(uint32_t);

protected:
	void resizeEvent(QResizeEvent*) override;
	void paintEvent(QPaintEvent*) override;
	void keyPressEvent(QKeyEvent*) override;

private slots:
	void jumpToEstimate(int);

private:
	struct Instruction {
		QString disassembly;
		QString hexcode;
		uint32_t address;
		int segment;
		int bytesize;
	};

	struct InstructionBlock {
		bool isMirror = false;
		uint32_t mirror;
		size_t memsize;
	};

	Instruction disassemble(uint32_t address);

#ifdef M_CORE_GBA
	uint32_t lastInstructionARM(uint32_t);
#endif
#ifdef M_CORE_GB
	uint32_t lastInstructionLR35902(uint32_t);
#endif

	void adjustCursor(int adjust, bool shift);

	QMap<uint32_t, InstructionBlock> m_instructionBlocks;

	std::shared_ptr<CoreController> m_controller;
	int m_currentMapping = 0;
	int m_address;
	int m_pc;
	int m_indexEstimate;
};

}
