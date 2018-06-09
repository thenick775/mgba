/* Copyright (c) 2013-2018 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "DisassemblyModel.h"

#include <QFontDatabase>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QSlider>

using namespace QGBA;

static QFont s_font;
static QFontMetrics s_metrics(s_font);
static QSize s_hexMetrics;
static QMargins s_margins;

DisassemblyModel::DisassemblyModel(QWidget* parent)
	: QAbstractScrollArea(parent)
{
	s_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	s_metrics = QFontMetrics(s_font);
	s_hexMetrics = s_metrics.size(0, "FFFFFFFF") * 1.2;
	s_margins = QMargins(s_hexMetrics.height() * 1.5, s_hexMetrics.height() / 2, 0, 0);

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	connect(verticalScrollBar(), &QSlider::sliderMoved, this, &DisassemblyModel::jumpToEstimate);
}

void DisassemblyModel::setController(std::shared_ptr<CoreController> controller) {
	m_controller = controller;
	m_address = 0;

	CoreController::Interrupter interrupter(m_controller);
	const mCoreMemoryBlock* memoryBlocks = nullptr;
	mCore* core = m_controller->thread()->core;
	m_indexEstimate = 0;
	size_t nMemoryBlocks = core->listMemoryBlocks(core, &memoryBlocks);
	for (size_t i = 0; i < nMemoryBlocks; ++i) {
		if (!(memoryBlocks[i].flags & mCORE_MEMORY_MAPPED)) {
			continue;
		}
		auto& block = m_instructionBlocks[memoryBlocks[i].start];
		if (memoryBlocks[i].mirror >= 0) {
			block.isMirror = true;
			block.mirror = memoryBlocks[memoryBlocks[i].mirror].start;
		} else {
			block.isMirror = false;
		}
		block.memsize = memoryBlocks[i].end - memoryBlocks[i].start;
		m_indexEstimate += block.memsize;
	}
	verticalScrollBar()->setRange(0, m_indexEstimate + 1 - viewport()->size().height() / s_hexMetrics.height());
	verticalScrollBar()->setValue(0);
	viewport()->update();
}

void DisassemblyModel::jumpToAddress(const QString& hex) {
	bool ok = false;
	uint32_t i = hex.toInt(&ok, 16);
	if (ok) {
		jumpToAddress(i);
	}
}

void DisassemblyModel::jumpToAddress(uint32_t address) {
	CoreController::Interrupter interrupter(m_controller);
	switch (m_controller->platform()) {
#ifdef M_CORE_GBA
	case PLATFORM_GBA:
		m_address = lastInstructionARM(address);
		break;
#endif
#ifdef M_CORE_GB
	case PLATFORM_GB:
		m_address = lastInstructionLR35902(address);
		break;
#endif
	}
	viewport()->update();
}

void DisassemblyModel::jumpToPc(uint32_t pc) {
	m_pc = pc;
	jumpToAddress(m_pc);
	adjustCursor(-2, false);
}

void DisassemblyModel::setMapping(int mapping) {
	if (m_currentMapping == mapping) {
		return;
	}
	m_currentMapping = mapping;
	adjustCursor(0, false);
}

void DisassemblyModel::jumpToEstimate(int index) {
	int blockMapping = -1;
	uint32_t offset = 0;
	for (const auto& blockId : m_instructionBlocks.keys()) {
		InstructionBlock block = m_instructionBlocks[blockId];
		int bmap = blockId;
		offset = 0;
		if (block.isMirror) {
			bmap = block.mirror;
			offset = blockId - bmap;
			block = m_instructionBlocks[bmap];
		}
		if (index >= block.memsize) {
			index -= block.memsize;
			continue;
		}
		blockMapping = bmap;
		break;
	}
	if (blockMapping < 0) {
		return;
	}

	index += blockMapping + offset;
	jumpToAddress(index);
}

void DisassemblyModel::resizeEvent(QResizeEvent*) {
	verticalScrollBar()->setRange(0, m_indexEstimate + 1 - viewport()->size().height() / s_hexMetrics.height());
}

void DisassemblyModel::paintEvent(QPaintEvent* event) {
	QPainter painter(viewport());
	QPalette palette;
	painter.setFont(s_font);
	painter.setPen(palette.color(QPalette::WindowText));

	CoreController::Interrupter interrupter(m_controller);
	mDebuggerPlatform* debugger = m_controller->debugger()->platform;
	int offset = 0;
	int height = viewport()->size().height() / s_hexMetrics.height();
	for (int y = 0; y < height; ++y) {
		int yp = s_hexMetrics.height() * y + s_margins.top() + s_metrics.ascent();
		Instruction insn = disassemble(m_address + offset);

		if (m_selected.contains(insn.address)) {
			painter.fillRect(QRectF(QPointF(0, yp - QFontMetrics(s_font).height()), QSizeF(viewport()->width(), s_hexMetrics.height())), palette.highlight());
		}

		float yc = yp + QFontMetrics(s_font).height() / 2 - s_metrics.ascent();
		if (m_breakpointCache.contains(std::make_pair(insn.address, insn.segment)) || m_breakpointCache.contains(std::make_pair(insn.address, -1))) {
			painter.setBrush(Qt::red);
			painter.setPen(Qt::NoPen);
			painter.drawEllipse(QPointF(s_hexMetrics.height() / 2, yc), s_hexMetrics.height() * 0.3, s_hexMetrics.height() * 0.3);
		}
		if (m_pc >= insn.address && m_pc < insn.address + insn.bytesize) {
			painter.setBrush(Qt::black);
			painter.setPen(Qt::NoPen);
			painter.drawEllipse(QPointF(s_hexMetrics.height() / 2, yc), s_hexMetrics.height() * 0.2, s_hexMetrics.height() * 0.2);
		}

		if (m_selected.contains(insn.address)) {
			painter.setPen(palette.color(QPalette::HighlightedText));
		} else {
			painter.setPen(palette.color(QPalette::WindowText));
		}

		painter.drawText(QPointF(s_margins.left(), yp), QString("%0").arg(insn.address, 8, 16, QChar('0')).toUpper());
		painter.drawText(QPointF(s_margins.left() + s_hexMetrics.width(), yp), insn.hexcode);
		painter.drawText(QPointF(s_margins.left() + s_hexMetrics.width() * 2 + 3, yp), insn.disassembly);
		offset += insn.bytesize;
	}
}

void DisassemblyModel::wheelEvent(QWheelEvent* event) {
	adjustCursor(event->angleDelta().y() / 8, false);
}

void DisassemblyModel::mousePressEvent(QMouseEvent* event) {
	uint32_t address = addressFromTop((event->y() - s_margins.top()) / s_hexMetrics.height());
	if (event->modifiers() & Qt::ShiftModifier) {
		if (m_selected.contains(address)) {
			m_selected.remove(address);
		} else {
			m_selected.insert(address);
		}
	} else {
		m_selected.clear();
		m_selected.insert(address);
	}
	viewport()->update();
}

void DisassemblyModel::mouseDoubleClickEvent(QMouseEvent* event) {
	CoreController::Interrupter interrupter(m_controller);
	mDebuggerPlatform* debugger = m_controller->debugger()->platform;
	uint32_t address = addressFromTop((event->y() - s_margins.top()) / s_hexMetrics.height());
	if (m_breakpointCache.contains(std::make_pair(address, -1))) {
		for (size_t id : m_breakpointCache.values(std::make_pair(address, -1))) {
			debugger->clearBreakpoint(debugger, id);
		}
		m_breakpointCache.remove(std::make_pair(address, -1));
	} else {
		struct mBreakpoint breakpoint = {
			.address = address,
			.segment = -1,
			.type = BREAKPOINT_HARDWARE
		};
		ssize_t id = debugger->setBreakpoint(debugger, &breakpoint);
		if (id >= 0) {
			m_breakpointCache.insert(std::make_pair(address, -1), id);
		}
	}
	viewport()->update();
}

void DisassemblyModel::keyPressEvent(QKeyEvent* event) {
	int key = event->key();
	switch (key) {
	case Qt::Key_Up:
		adjustCursor(-1, event->modifiers() & Qt::ShiftModifier);
		return;
	case Qt::Key_Down:
		adjustCursor(1, event->modifiers() & Qt::ShiftModifier);
		return;
	}
	event->ignore();
}

DisassemblyModel::Instruction DisassemblyModel::disassemble(uint32_t address) {
	Instruction insn{};
	insn.segment = -1;
	char buffer[64]{};
	mCore* core = m_controller->thread()->core;

	insn.address = address;
	switch (m_controller->platform()) {
#ifdef M_CORE_GBA
	case PLATFORM_GBA: {
		ARMInstructionInfo info;
		if (m_currentMapping == MODE_ARM) {
			uint32_t opcode = core->rawRead32(core, address, -1);
			ARMDecodeARM(opcode, &info);
			address += WORD_SIZE_ARM * 2;
			insn.bytesize = WORD_SIZE_ARM;
			insn.hexcode = QString("%0").arg(opcode, 8, 16, QChar('0')).toUpper();
		} else {
			ARMInstructionInfo info2;
			uint16_t opcode = core->rawRead16(core, address, -1);
			ARMDecodeThumb(opcode, &info);
			insn.bytesize = WORD_SIZE_THUMB;
			insn.hexcode = QString("%0").arg(opcode, 4, 16, QChar('0')).toUpper();
			opcode = core->busRead16(core, address + WORD_SIZE_THUMB);
			ARMDecodeThumb(opcode, &info2);
			if (ARMDecodeThumbCombine(&info, &info2, &info)) {
				insn.bytesize += WORD_SIZE_THUMB;
				insn.hexcode += QString("%0").arg(opcode, 4, 16, QChar('0')).toUpper();
			}
			address += WORD_SIZE_THUMB * 2;
		}
		ARMDisassemble(&info, address, buffer, sizeof(buffer));
		break;
	}
#endif
#ifdef M_CORE_GB
	case PLATFORM_GB: {
		LR35902InstructionInfo info{};
		for (int bytesRemaining = 1; bytesRemaining; --bytesRemaining) {
			uint8_t opcode = core->busRead8(core, address);
			bytesRemaining += LR35902Decode(opcode, &info);
			++insn.bytesize;
			++address;
			insn.hexcode += QString("%0").arg(opcode, 2, 16, QChar('0')).toUpper();
		}
		LR35902Disassemble(&info, address, buffer, sizeof(buffer));
	}
#endif
	}
	insn.disassembly = buffer;
	return insn;
}

#ifdef M_CORE_GBA
uint32_t DisassemblyModel::lastInstructionARM(uint32_t address) {
	if (m_currentMapping == MODE_ARM) {
		return address & ~3;
	}
	address &= ~1;

	ARMInstructionInfo info{};
	ARMInstructionInfo info2{};
	mCore* core = m_controller->thread()->core;
	uint16_t opcode = core->busRead16(core, address - WORD_SIZE_THUMB);
	ARMDecodeThumb(opcode, &info);
	opcode = core->busRead16(core, address);
	ARMDecodeThumb(opcode, &info2);
	if (ARMDecodeThumbCombine(&info, &info2, &info)) {
		return address - WORD_SIZE_THUMB;
	}
	return address;
}
#endif

#ifdef M_CORE_GB
uint32_t DisassemblyModel::lastInstructionLR35902(uint32_t address) {
	uint32_t prefix = 0;
	uint32_t prefix2 = 0;
	mCore* core = m_controller->thread()->core;

	for (uint32_t mockAddress = address - 1; address - mockAddress < 8; --mockAddress) {
		LR35902InstructionInfo info{};
		int bytesRemaining = 0;
		for (uint32_t currentAddress = mockAddress; currentAddress <= address; ++currentAddress) {
			if (!bytesRemaining) {
				info = {};
				bytesRemaining = 1;
				prefix2 = currentAddress;
			}
			uint8_t opcode = core->busRead8(core, currentAddress);
			bytesRemaining += LR35902Decode(opcode, &info);
			--bytesRemaining;
		}
		if (prefix == prefix2) {
			return prefix;
		}
		prefix = prefix2;
	}
	return prefix;
}
#endif

void DisassemblyModel::adjustCursor(int adjust, bool shift) {
	if (shift) {
		// TODO
	}
	m_address = addressFromTop(adjust);
	viewport()->update();
}


uint32_t DisassemblyModel::addressFromTop(int adjust) {
	uint32_t cursorPosition = m_address;
	uint32_t address = m_address;
	CoreController::Interrupter interrupter(m_controller);
	while (adjust) {
		int offset = 0;
		while (cursorPosition == address) {
			offset += (adjust < 0 ? -1 : 1);
			if (offset < 0 && -offset > cursorPosition) {
				break;
			}
			switch (m_controller->platform()) {
		#ifdef M_CORE_GBA
			case PLATFORM_GBA:
				cursorPosition = lastInstructionARM(cursorPosition + offset);
				break;
		#endif
		#ifdef M_CORE_GB
			case PLATFORM_GB:
				cursorPosition = lastInstructionLR35902(cursorPosition + offset);
				break;
		#endif
			}
		}
		address = cursorPosition;
		adjust += (adjust < 0 ? 1 : -1);
	}
	return address;
}
