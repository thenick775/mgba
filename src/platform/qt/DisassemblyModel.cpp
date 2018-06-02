/* Copyright (c) 2013-2018 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "DisassemblyModel.h"

#include <QFontDatabase>
#include <QFontMetrics>

using namespace QGBA;

static QFont s_font;
static QSize s_hexMetrics;

void DisassemblyModel::setController(std::shared_ptr<CoreController> controller) {
	m_controller = controller;
	m_instructionBlocks.clear();
	s_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	s_hexMetrics = QFontMetrics(s_font).size(0, "FFFFFFFF") * 1.5;
	refresh(true);
}

QVariant DisassemblyModel::headerData(int section, Qt::Orientation o, int role) const {
	if (o != Qt::Vertical) {
		if (role == Qt::SizeHintRole) {
			switch (section) {
			case 0:
				return QSize(s_hexMetrics.height(), s_hexMetrics.height());
			case 1:
				return s_hexMetrics;
			case 2:
				return QSize(s_hexMetrics.width() * 4, s_hexMetrics.height());
			}
		}
		return QVariant();
	}

	switch (role) {
	case Qt::FontRole:
		return s_font;
	case Qt::DisplayRole:
		return QString::number(indexToInstruction(section, nullptr).address, 16);
	case Qt::SizeHintRole:
		return s_hexMetrics;
	case Qt::TextAlignmentRole:
		return Qt::AlignRight;
	default:
		return QAbstractItemModel::headerData(section, o, role);
	}
}

QVariant DisassemblyModel::data(const QModelIndex& index, int role) const {
	switch (role) {
	case Qt::FontRole:
		return s_font;
	case Qt::DisplayRole:
		break;
	default:
		return QVariant();
	}

	const uint8_t* mem;
	Instruction insn = indexToInstruction(index.row(), &mem);
	QString builder;

	switch (index.column()) {
	case 0:
		// TODO: breakpoint icon
		break;
	case 1:
		switch (m_controller->platform()) {
#ifdef M_CORE_GBA
		case PLATFORM_GBA:
			for (uint32_t i = insn.nRaw; i > 0; --i) {
				builder += QString("%1").arg(mem[i - 1], 2, 16, QChar('0'));
			}
			break;
#endif
#ifdef M_CORE_GB
		case PLATFORM_GB:
			for (uint32_t i = 0; i < insn.nRaw; ++i) {
				builder += QString("%1").arg(mem[i], 2, 16, QChar('0'));
			}
			break;
#endif
		}
		return builder;
	case 2:
		return disassemble(insn, mem);
	}
	return QVariant();
}

QModelIndex DisassemblyModel::index(int row, int column, const QModelIndex& parent) const {
	return createIndex(row, column, nullptr);
}

QModelIndex DisassemblyModel::parent(const QModelIndex& index) const {
	return QModelIndex();
}

int DisassemblyModel::columnCount(const QModelIndex& parent) const {
	return 3;
}

int DisassemblyModel::rowCount(const QModelIndex& parent) const {
	int sum = 0;
	for (const auto& block : m_instructionBlocks) {
		if (block.isMirror) {
			sum += m_instructionBlocks[block.mirror].mapping[m_currentMapping].size();
		} else {
			sum += block.mapping[m_currentMapping].size();
		}
	}
	return sum;
}

void DisassemblyModel::refresh(bool ro) {
	CoreController::Interrupter interrupter(m_controller);
	const mCoreMemoryBlock* memoryBlocks = nullptr;
	mCore* core = m_controller->thread()->core;
	size_t nMemoryBlocks = core->listMemoryBlocks(core, &memoryBlocks);
	for (size_t i = 0; i < nMemoryBlocks; ++i) {
		if (!(memoryBlocks[i].flags & mCORE_MEMORY_MAPPED)) {
			continue;
		}
		if (memoryBlocks[i].mirror >= 0) {
			auto& block = m_instructionBlocks[memoryBlocks[i].start];
			block.mapping.clear();
			block.isMirror = true;
			block.mirror = memoryBlocks[memoryBlocks[i].mirror].start;
			block.memory = core->getMemoryBlock(core, memoryBlocks[i].id, &block.memsize);
			continue;
		}
		if (ro || memoryBlocks[i].flags & mCORE_MEMORY_WRITE) {
			refreshBlock(memoryBlocks[i]);
		}
	}
}

DisassemblyModel::Instruction DisassemblyModel::indexToInstruction(int index, const uint8_t** mem) const {
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
		if (index >= block.mapping[m_currentMapping].size()) {
			index -= block.mapping[m_currentMapping].size();
			continue;
		}
		blockMapping = bmap;
		break;
	}
	if (blockMapping < 0) {
		return {};
	}

	const auto& block = m_instructionBlocks[blockMapping].mapping[m_currentMapping];
	auto insn = block[index];
	if (mem) {
		*mem = &static_cast<const uint8_t*>(m_instructionBlocks[blockMapping].memory)[insn.address - blockMapping];
	}
	insn.address += offset;
	return insn;
}

QModelIndex DisassemblyModel::addressToIndex(uint32_t address) const {
	int i = 0;
	for (const auto& blockId : m_instructionBlocks.keys()) {
		InstructionBlock block = m_instructionBlocks[blockId];
		uint32_t offset = 0;
		if (block.isMirror) {
			offset = blockId - block.mirror;
			block = m_instructionBlocks[block.mirror];
		}
		const auto& mapping = block.mapping[m_currentMapping];
		if (!mapping.size()) {
			continue;
		}
		uint32_t first = mapping[0].address + offset;
		if (address < first) {
			return QModelIndex();
		}
		uint32_t last = mapping[mapping.size() - 1].address + offset;
		if (address > last) {
			i += mapping.size();
			continue;
		}
		auto iter = std::lower_bound(mapping.cbegin(), mapping.cend(), address, [offset](const auto& i, uint32_t j) {
			return i.address + offset < j;
		});
		if (iter->address + offset > address) {
			iter = --iter;
		}
		return index(i + (iter - mapping.cbegin()), 1, QModelIndex());
	}
	return QModelIndex();
}

QString DisassemblyModel::disassemble(const Instruction& insn, const uint8_t* mem) const {
	char buffer[64]{};
	switch (m_controller->platform()) {
#ifdef M_CORE_GBA
	case PLATFORM_GBA: {
		ARMInstructionInfo info;
		uint32_t address = insn.address;
		if (m_currentMapping == MODE_ARM) {
			uint32_t opcode;
			LOAD_32LE(opcode, 0, mem);
			ARMDecodeARM(opcode, &info);
			address += WORD_SIZE_ARM * 2;
		} else {
			uint16_t opcode;
			LOAD_16LE(opcode, 0, mem);
			ARMDecodeThumb(opcode, &info);
			address += WORD_SIZE_THUMB * 2;
			if (insn.nRaw == 4) {
				ARMInstructionInfo info2;
				LOAD_16LE(opcode, 2, mem);
				ARMDecodeThumb(opcode, &info2);
				ARMDecodeThumbCombine(&info, &info2, &info);
			}
		}
		ARMDisassemble(&info, address, buffer, sizeof(buffer));
		break;
	}
#endif
#ifdef M_CORE_GB
	case PLATFORM_GB: {
		LR35902InstructionInfo info{};
		for (size_t i = 0; i < insn.nRaw; ++i) {
			LR35902Decode(mem[i], &info);
		}
		LR35902Disassemble(&info, address, buffer, sizeof(buffer));
	}
#endif
	}
	return QString(buffer);
}

void DisassemblyModel::refreshBlock(const mCoreMemoryBlock& block) {
	m_instructionBlocks[block.start].mapping.clear();
	m_instructionBlocks[block.start].isMirror = false;
	switch (m_controller->platform()) {
#ifdef M_CORE_GBA
	case PLATFORM_GBA:
		refreshARM(block);
		break;
#endif
#ifdef M_CORE_GB
	case PLATFORM_GB:
		refreshLR35902(block);
		break;
#endif
	}
}

#ifdef M_CORE_GB
void DisassemblyModel::refreshLR35902(const mCoreMemoryBlock& block) {
	mCore* core = m_controller->thread()->core;
	InstructionBlock& iblock = m_instructionBlocks[block.start];
	iblock.memory = core->getMemoryBlock(core, block.id, &iblock.memsize);
	for (int segment = 0; segment < block.maxSegment; ++segment) {
		for (uint32_t address = block.start; address < block.end && address - block.start < block.size;) {
			LR35902InstructionInfo info{};
			Instruction insn{
				address,
				0,
			};
			uint8_t byte;
			size_t bytesRemaining = 1;
			for (bytesRemaining = 1; bytesRemaining; --bytesRemaining) {
				byte = core->rawRead8(core, address, segment);
				++address;
				++insn.nRaw;
				bytesRemaining += LR35902Decode(byte, &info);
			}
			iblock.mapping[segment].append(insn);
		}
	}
}
#endif

#ifdef M_CORE_GBA
void DisassemblyModel::refreshARM(const mCoreMemoryBlock& block) {
	mCore* core = m_controller->thread()->core;
	InstructionBlock& iblock = m_instructionBlocks[block.start];
	iblock.mapping[MODE_ARM].reserve(block.size / 2);
	iblock.mapping[MODE_THUMB].reserve(block.size);
	iblock.memory = core->getMemoryBlock(core, block.id, &iblock.memsize);
	ARMInstructionInfo thumbLast{};
	uint16_t thumbLastOpcode = 0;
	for (uint32_t address = block.start; address < block.end && address - block.start + 4 <= iblock.memsize; address += 4) {
		ARMInstructionInfo info;
		uint32_t opcode;
		LOAD_32LE(opcode, address - block.start, iblock.memory);
		Instruction arm = {
			address,
			4,
		};
		iblock.mapping[MODE_ARM].append(arm);

		for (int i = 0; i < 2; ++i) {
			Instruction thumb = {
				address + i * WORD_SIZE_THUMB,
				2,
			};

			ARMDecodeThumb(opcode, &info);
			if (ARMDecodeThumbCombine(&thumbLast, &info, &info)) {
				thumb.nRaw = 4;
				thumb.address -= WORD_SIZE_THUMB;
				iblock.mapping[MODE_THUMB].back() = thumb;
				thumbLast = ARMInstructionInfo{};
			} else {
				iblock.mapping[MODE_THUMB].append(thumb);
				thumbLast = info;
				thumbLastOpcode = opcode;
			}
			opcode >>= 16;
		}
	}
}
#endif
