/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#pragma once

#include <QMap>
#include <QWidget>

#include <memory>

class QLabel;

namespace QGBA {

class CoreController;

class RegisterView : public QWidget {
Q_OBJECT

public:
	RegisterView(std::shared_ptr<CoreController> controller, QWidget* parent = nullptr);

	uint32_t pc() const { return m_pc; }
	int execMode() const { return m_mode; }

public slots:
	void updateRegisters();

private:
	void addRegisters(const QStringList& names, int column = 0);
#ifdef M_CORE_GBA
	void updateRegistersARM();
#endif
#ifdef M_CORE_GB
	void updateRegistersLR35902();
#endif

	QMap<QString, QLabel*> m_registers;
	uint32_t m_pc = 0;
	int m_mode = 0;

	std::shared_ptr<CoreController> m_controller;
};

}
