/* Copyright (c) 2013-2018 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "Debugger.h"

#include "CoreController.h"

#include <mgba/core/thread.h>

using namespace QGBA;

Debugger::Debugger(QObject* parent)
	: DebuggerController(&m_debugger, parent)
{
	m_debugger.p = this;
	m_debugger.init = &init;
	m_debugger.deinit = &deinit;
	m_debugger.custom = &custom;
	m_debugger.paused = &paused;
	m_debugger.entered = &entered;
}

void Debugger::doContinue() {
	CoreController::Interrupter interrupter(m_gameController);
	m_debugger.state = DEBUGGER_RUNNING;
	emit continued();
	mCoreThreadStopWaiting(m_gameController->thread());
}

void Debugger::doBreak() {
	mCoreThreadRunFunction(m_gameController->thread(), [](mCoreThread* context) {
		mDebuggerEnter(context->core->debugger, DEBUGGER_ENTER_MANUAL, nullptr);
	});
}

void Debugger::next() {
	mCoreThreadRunFunction(m_gameController->thread(), [](mCoreThread* context) {
		if (context->core->debugger->state == DEBUGGER_RUNNING) {
			mDebuggerEnter(context->core->debugger, DEBUGGER_ENTER_MANUAL, nullptr);
		}
		context->core->step(context->core);
		auto p = static_cast<DebuggerImpl*>(context->core->debugger)->p;
		emit p->stepped();
	});
}

void Debugger::init(mDebugger* debugger) {
	static_cast<DebuggerImpl*>(debugger)->p->init();
}

void Debugger::deinit(mDebugger* debugger) {
	static_cast<DebuggerImpl*>(debugger)->p->deinit();
}

void Debugger::custom(mDebugger* debugger) {
	static_cast<DebuggerImpl*>(debugger)->p->custom();
}

void Debugger::paused(mDebugger* debugger) {
	static_cast<DebuggerImpl*>(debugger)->p->paused();
}

void Debugger::entered(mDebugger* debugger, mDebuggerEntryReason reason, mDebuggerEntryInfo* info) {
	auto p = static_cast<DebuggerImpl*>(debugger)->p;
	mCoreThreadWaitFromThread(p->m_gameController->thread());
	emit p->entered(reason);
}
