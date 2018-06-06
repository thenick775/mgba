/* Copyright (c) 2013-2018 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#pragma once

#include <QObject>

#ifdef USE_DEBUGGERS

#include "DebuggerController.h"

#include <mgba/debugger/debugger.h>

namespace QGBA {

class Debugger : public DebuggerController {
Q_OBJECT

public:
	Debugger(QObject* parent = nullptr);

public slots:
	void doContinue();
	void doBreak();
	void next();

signals:
	void entered(mDebuggerEntryReason);
	void stepped();
	void continued();

protected:
	virtual void init() {}
	virtual void deinit() {}
	virtual void custom() {}
	virtual void paused() {}

private:
	static void init(mDebugger*);
	static void deinit(mDebugger*);
	static void custom(mDebugger*);
	static void paused(mDebugger*);
	static void entered(mDebugger*, enum mDebuggerEntryReason, struct mDebuggerEntryInfo*);

	struct DebuggerImpl : public mDebugger {
		Debugger* p;
	} m_debugger{};
};

}

#endif
