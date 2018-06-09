/* Copyright (c) 2013-2018 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Debugger.h"
#include "DisassemblyModel.h"

#include <QDockWidget>

#include <memory>

#ifdef USE_DEBUGGERS

class QMainWindow;
class QTableView;

namespace QGBA {

class CoreController;
class RegisterView;

class DebugModeContext : public QObject {
Q_OBJECT

public:
	void attach(QMainWindow*, QWidget* screen, std::shared_ptr<CoreController>);

	Debugger* debugger() { return m_debugger.get(); }

public slots:
	void release();

protected:
	bool eventFilter(QObject* obj, QEvent* event) override;

private:
	std::unique_ptr<QDockWidget> m_screen;
	std::unique_ptr<QDockWidget> m_memory;
	std::unique_ptr<QDockWidget> m_registers;
	std::unique_ptr<DisassemblyModel> m_disassembly;
	std::unique_ptr<Debugger> m_debugger;

	std::shared_ptr<CoreController> m_controller;
};

}

#endif
