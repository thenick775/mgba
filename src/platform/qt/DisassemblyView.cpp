/* Copyright (c) 2013-2018 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "DisassemblyView.h"

#include "CoreController.h"
#include "LogController.h"

#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QSlider>

#include <mgba/core/core.h>

using namespace QGBA;

DisassemblyView::DisassemblyView(QWidget* parent)
	: QAbstractScrollArea(parent)
{
	m_font.setFamily("Source Code Pro");
	m_font.setStyleHint(QFont::Monospace);
#ifdef Q_OS_MAC
	m_font.setPointSize(12);
#else
	m_font.setPointSize(10);
#endif
	QFontMetrics metrics(m_font);
	m_fontHeight = metrics.height();
	setFocusPolicy(Qt::StrongFocus);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	connect(verticalScrollBar(), &QSlider::sliderMoved, [this](int position) {
		m_top = position;
		update();
	});

	setRegion(0, 0x10000000, tr("All"));
}

void DisassemblyView::setController(std::shared_ptr<CoreController> controller) {
	m_core = controller->thread()->core;
}

void DisassemblyView::setRegion(uint32_t base, uint32_t size, const QString& name, int segment) {
	m_top = 0;
	m_base = base;
	m_size = size;
	m_currentBank = segment;
	verticalScrollBar()->setRange(0, (size >> 4) + 1 - viewport()->size().height() / m_fontHeight);
	verticalScrollBar()->setValue(0);
	viewport()->update();
}

void DisassemblyView::setSegment(int segment) {
	m_currentBank = segment;
	viewport()->update();
}

void DisassemblyView::jumpToAddress(const QString& hex) {
	bool ok = false;
	uint32_t i = hex.toInt(&ok, 16);
	if (ok) {
		jumpToAddress(i);
	}
}

void DisassemblyView::jumpToAddress(uint32_t address) {
	if (address >= 0x10000000) {
		return;
	}
	if (address < m_base || address >= m_base + m_size) {
		setRegion(0, 0x10000000, tr("All"));
	}
	m_top = (address - m_base) / 16;
	boundsCheck();
	verticalScrollBar()->setValue(m_top);
}

void DisassemblyView::resizeEvent(QResizeEvent*) {
	verticalScrollBar()->setRange(0, (m_size >> 4) + 1 - viewport()->size().height() / m_fontHeight);
	boundsCheck();
}

void DisassemblyView::paintEvent(QPaintEvent* event) {
	QPainter painter(viewport());
	QPalette palette;
	painter.setFont(m_font);
	painter.setPen(palette.color(QPalette::WindowText));
}

void DisassemblyView::wheelEvent(QWheelEvent* event) {
	m_top -= event->angleDelta().y() / 8;
	boundsCheck();
	event->accept();
	verticalScrollBar()->setValue(m_top);
	update();
}

void DisassemblyView::mousePressEvent(QMouseEvent* event) {
	if (event->x() < m_margins.left() || event->y() < m_margins.top() ||
	    event->x() > size().width() - m_margins.right()) {
		m_selection = qMakePair(0, 0);
		return;
	}

	QPoint position(event->pos() - QPoint(m_margins.left(), m_margins.top()));
	uint32_t address = int(position.y() / m_fontHeight) + m_top + m_base;
	if (event->button() == Qt::RightButton && isInSelection(address)) {
		return;
	}
	if (event->modifiers() & Qt::ShiftModifier) {
		if ((address & ~(m_align - 1)) < m_selectionAnchor) {
			m_selection = qMakePair(address & ~(m_align - 1), m_selectionAnchor + m_align);
		} else {
			m_selection = qMakePair(m_selectionAnchor, (address & ~(m_align - 1)) + m_align);
		}
	} else {
		m_selectionAnchor = address & ~(m_align - 1);
		m_selection = qMakePair(m_selectionAnchor, m_selectionAnchor + m_align);
	}
	emit selectionChanged(m_selection.first, m_selection.second);
	viewport()->update();
}

void DisassemblyView::mouseMoveEvent(QMouseEvent* event) {
	if (event->x() < m_margins.left() || event->y() < m_margins.top() ||
	    event->x() > size().width() - m_margins.right()) {
		return;
	}

	QPoint position(event->pos() - QPoint(m_margins.left(), m_margins.top()));
	uint32_t address = int(position.y() / m_fontHeight) + m_top + m_base;
	if ((address & ~(m_align - 1)) < m_selectionAnchor) {
		m_selection = qMakePair(address & ~(m_align - 1), m_selectionAnchor + m_align);
	} else {
		m_selection = qMakePair(m_selectionAnchor, (address & ~(m_align - 1)) + m_align);
	}
	emit selectionChanged(m_selection.first, m_selection.second);
	viewport()->update();
}

void DisassemblyView::keyPressEvent(QKeyEvent* event) {
	if (m_selection.first >= m_selection.second) {
		return;
	}
	int key = event->key();
	switch (key) {
	case Qt::Key_Up:
		adjustCursor(-1, event->modifiers() & Qt::ShiftModifier);
		break;
	case Qt::Key_Down:
		adjustCursor(1, event->modifiers() & Qt::ShiftModifier);
		break;
	default:
		break;
	}
}

void DisassemblyView::boundsCheck() {
	if (m_top < 0) {
		m_top = 0;
	} else if (m_top > (m_size >> 4) + 1 - viewport()->size().height() / m_fontHeight) {
		m_top = (m_size >> 4) + 1 - viewport()->size().height() / m_fontHeight;
	}
}

bool DisassemblyView::isInSelection(uint32_t address) {
	if (m_selection.first == m_selection.second) {
		return false;
	}
	if (m_selection.second <= (address | (m_align - 1))) {
		return false;
	}
	if (m_selection.first <= (address & ~(m_align - 1))) {
		return true;
	}
	return false;
}

void DisassemblyView::adjustCursor(int adjust, bool shift) {
	if (m_selection.first >= m_selection.second) {
		return;
	}
	int cursorPosition = m_top;
	if (shift) {
		if (m_selectionAnchor == m_selection.first) {
			if (adjust < 0 && m_base - adjust > m_selection.second) {
				adjust = m_base - m_selection.second + m_align;
			} else if (adjust > 0 && m_selection.second + adjust >= m_base + m_size) {
				adjust = m_base + m_size - m_selection.second;
			}
			adjust += m_selection.second;
			if (adjust <= m_selection.first) {
				m_selection.second = m_selection.first + m_align;
				m_selection.first = adjust - m_align;
				cursorPosition = m_selection.first;
			} else {
				m_selection.second = adjust;
				cursorPosition = m_selection.second - m_align;
			}
		} else {
			if (adjust < 0 && m_base - adjust > m_selection.first) {
				adjust = m_base - m_selection.first;
			} else if (adjust > 0 && m_selection.first + adjust >= m_base + m_size) {
				adjust = m_base + m_size - m_selection.first - m_align;
			}
			adjust += m_selection.first;
			if (adjust >= m_selection.second) {
				m_selection.first = m_selection.second - m_align;
				m_selection.second = adjust + m_align;
				cursorPosition = adjust;
			} else {
				m_selection.first = adjust;
				cursorPosition = m_selection.first;
			}
		}
		cursorPosition -= m_base;
	} else {
		if (m_selectionAnchor == m_selection.first) {
			m_selectionAnchor = m_selection.second - m_align;
		} else {
			m_selectionAnchor = m_selection.first;
		}
		if (adjust < 0 && m_base - adjust > m_selectionAnchor) {
			m_selectionAnchor = m_base;
		} else if (adjust > 0 && m_selectionAnchor + adjust >= m_base + m_size) {
			m_selectionAnchor = m_base + m_size - m_align;
		} else {
			m_selectionAnchor += adjust;
		}
		m_selection.first = m_selectionAnchor;
		m_selection.second = m_selection.first + m_align;
		cursorPosition = m_selectionAnchor - m_base;
	}
	if (cursorPosition < m_top) {
		m_top = cursorPosition;
	} else if (cursorPosition >= m_top + viewport()->size().height() / m_fontHeight - 1) {
		m_top = cursorPosition - viewport()->size().height() / m_fontHeight + 2;
	}
	emit selectionChanged(m_selection.first, m_selection.second);
	viewport()->update();
}
