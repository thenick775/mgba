/* Copyright (c) 2013-2018 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Debugger.h"

#include <QDockWidget>

#include <memory>

#ifdef USE_DEBUGGERS

class QMainWindow;
class QTableView;

namespace QGBA {

class CoreController;
class Debugger;
class DisassemblyModel;
class RegisterView;

class DebugModeContext : public QObject {
Q_OBJECT

public:
	void attach(QMainWindow*, QWidget* screen, std::shared_ptr<CoreController>);

	Debugger* debugger() { return m_debugger; }

public slots:
	void release();

private:
	QDockWidget* m_screen = nullptr;
	QDockWidget* m_memory = nullptr;
	QDockWidget* m_registers = nullptr;
	DisassemblyModel* m_disassembly = nullptr;
	Debugger* m_debugger;

	std::shared_ptr<CoreController> m_controller;
};

}

#endif
