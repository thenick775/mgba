
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
		m_screen = new QDockWidget;
		m_screen->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	}
	m_screen->setWidget(screen);
	window->addDockWidget(Qt::RightDockWidgetArea, m_screen);

	if (!m_memory) {
		m_memory = new QDockWidget;
	}
	MemoryModel* memModel = new MemoryModel;
	memModel->setController(controller);
	m_memory->setWidget(memModel);
	window->addDockWidget(Qt::BottomDockWidgetArea, m_memory);

	if (!m_registers) {
		m_registers = new QDockWidget;
		m_registers->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	}
	RegisterView* regView = new RegisterView(controller);
	m_registers->setWidget(regView);
	window->addDockWidget(Qt::RightDockWidgetArea, m_registers);

	if (!m_disassembly) {
		m_disassembly = new DisassemblyModel;
	}
	m_disassembly->setController(controller);
	window->setCentralWidget(m_disassembly);

	if (!m_debugger) {
		m_debugger = new Debugger(this);
	}
	connect(m_debugger, static_cast<void (Debugger::*)(mDebuggerEntryReason)>(&Debugger::entered), regView, &RegisterView::updateRegisters);
	connect(m_debugger, &Debugger::stepped, regView, &RegisterView::updateRegisters);
	connect(m_debugger, &Debugger::stepped, m_disassembly, [this, regView]() {
		m_disassembly->jumpToPc(regView->pc());
	});

	m_debugger->setController(controller);
	m_debugger->attach();

	connect(controller.get(), &CoreController::frameAvailable, regView, &RegisterView::updateRegisters);
	connect(controller.get(), &CoreController::stopping, this, &DebugModeContext::release);
}

void DebugModeContext::release() {
	if (m_debugger) {
		m_debugger->detach();
	}

	if (m_screen) {
		delete m_screen;
		m_screen = nullptr;
	}

	if (m_memory) {
		delete m_memory;
		m_memory = nullptr;
	}

	if (m_registers) {
		delete m_registers;
		m_registers = nullptr;
	}

	if (m_disassembly) {
		delete m_disassembly;
		m_disassembly = nullptr;
	}

	if (m_debugger) {
		delete m_debugger;
		m_debugger = nullptr;
	}

	m_controller.reset();
}
