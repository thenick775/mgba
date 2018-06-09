
/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "DebugModeContext.h"

#include "CoreController.h"
#include "Debugger.h"
#include "DisassemblyModel.h"
#include "MemoryModel.h"
#include "RegisterView.h"

#include <QDockWidget>
#include <QEvent>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QHeaderView>
#include <QMainWindow>
#include <QTableView>

using namespace QGBA;
using namespace std;

void DebugModeContext::attach(QMainWindow* window, QWidget* screen, shared_ptr<CoreController> controller) {
	if (m_controller != controller) {
		release();
		m_controller = controller;
	}

	if (!m_screen) {
		m_screen = make_unique<QDockWidget>();
		m_screen->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	}
	m_screen->setWidget(screen);
	screen->installEventFilter(this);
	window->addDockWidget(Qt::RightDockWidgetArea, m_screen.get());

	if (!m_memory) {
		m_memory = make_unique<QDockWidget>();
	}
	MemoryModel* memModel = new MemoryModel;
	memModel->setController(controller);
	m_memory->setWidget(memModel);
	window->addDockWidget(Qt::BottomDockWidgetArea, m_memory.get());

	if (!m_registers) {
		m_registers = make_unique<QDockWidget>();
		m_registers->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	}
	RegisterView* regView = new RegisterView(controller);
	m_registers->setWidget(regView);
	window->addDockWidget(Qt::RightDockWidgetArea, m_registers.get());

	if (!m_disassembly) {
		m_disassembly = make_unique<DisassemblyModel>();
	}
	m_disassembly->setController(controller);
	window->setCentralWidget(m_disassembly.get());

	if (!m_debugger) {
		m_debugger = make_unique<Debugger>(this);
	}
	connect(m_debugger.get(), static_cast<void (Debugger::*)(mDebuggerEntryReason)>(&Debugger::entered), regView, &RegisterView::updateRegisters);
	connect(m_debugger.get(), &Debugger::stepped, regView, &RegisterView::updateRegisters);
	connect(m_debugger.get(), &Debugger::stepped, m_disassembly.get(), [this, regView]() {
		m_disassembly->setMapping(regView->execMode());
		m_disassembly->jumpToPc(regView->pc());
	});
	connect(m_debugger.get(), static_cast<void (Debugger::*)(mDebuggerEntryReason)>(&Debugger::entered), m_disassembly.get(), [this, regView]() {
		m_disassembly->setMapping(regView->execMode());
		m_disassembly->jumpToPc(regView->pc());
	});

	m_debugger->setController(controller);
	m_debugger->attach();

	connect(controller.get(), &CoreController::stopping, this, &DebugModeContext::release);
}

void DebugModeContext::release() {
	if (m_debugger) {
		m_debugger->detach();
	}

	if (m_screen) {
		// Prevent screen from being deleted
		QWidget* w = m_screen->widget();
		if (w) {
			w->removeEventFilter(this);
			w->setParent(nullptr);
		}
		m_screen.reset();
	}

	m_memory.reset();
	m_registers.reset();
	m_disassembly.reset();
	m_debugger.reset();

	disconnect();
	m_controller.reset();
}

bool DebugModeContext::eventFilter(QObject* obj, QEvent* event) {
	if (m_screen && obj == m_screen->widget()) {
		switch (event->type()) {
		case QEvent::MouseButtonPress:
			m_debugger->doContinue();
			return true;
		case QEvent::MouseButtonDblClick:
			return true;
		default:
			break;
		}
	}
	return false;
}
