/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef QGBA_NETPLAY_CONTROLLER
#define QGBA_NETPLAY_CONTROLLER

#include <QList>
#include <QMap>
#include <QObject>

#include <mgba/internal/netplay/netplay.h>

struct mNPServer;
struct mNPServerOptions;

Q_DECLARE_METATYPE(mNPRoomInfo);
Q_DECLARE_METATYPE(mNPCoreInfo);

namespace QGBA {

class GameController;
class MultiplayerController;

class NetplayController : public QObject {
Q_OBJECT

public:
	static const uint16_t DEFAULT_PORT;

	NetplayController(MultiplayerController* mp, QObject* parent = NULL);
	~NetplayController();

	bool startServer(const QString& address, uint16_t port = DEFAULT_PORT);
	bool connectToServer(const QString& address, uint16_t port = DEFAULT_PORT);

	void listRooms(std::function<void (const QList<mNPRoomInfo>&)> callback);
	void listCores(std::function<void (const QList<mNPCoreInfo>&)> callback, uint32_t roomId = 0);

	const QList<mNPCoreInfo>& cores() const { return m_coreInfo; }
	const QList<mNPRoomInfo>& rooms() const { return m_roomInfo; }

	bool serverRunning() const { return m_server; }
	bool connectedToServer() const { return m_connected; }

	QString connectedHost() const;

	GameController* controllerForId(uint32_t coreId) { return m_cores[coreId]; }

public slots:
	void stopServer();
	void disconnectFromServer();
	void addGameController(GameController*);
	void clone(GameController*, quint32 coreId, quint32 flags);

	void joinRoom(GameController*, quint32 roomId = 0);
	void joinFirstRoom(GameController*);

private slots:
	void addGameController(quint32 nonce, quint32 id);
	void cbListRooms(QList<mNPRoomInfo> list);
	void cbListCores(QList<mNPCoreInfo> list, quint32 roomId);
	void cbRollbackStart(QList<mNPCoreInfo> list);
	void cbRollbackEnd(QList<mNPCoreInfo> list);
	void updateRooms();
	void updateCores();

signals:
	void connected();
	void disconnected();
	void roomJoined(quint32 roomId, quint32 coreId);
	void coreRegistered(quint32 coreId);

private:
	MultiplayerController* m_multiplayer;
	QMap<uint32_t, GameController*> m_cores;
	QMap<uint32_t, GameController*> m_pendingCores;
	mNPContext* m_np;
	mNPServer* m_server;
	bool m_connected;
	QString m_connectedHost;
	uint16_t m_connectedPort;

	QList<mNPRoomInfo> m_roomInfo;
	QList<mNPCoreInfo> m_coreInfo;

	QList<std::function<void (const QList<mNPRoomInfo>&)>> m_listRoomsCallbacks;
	QMap<quint32, QList<std::function<void (const QList<mNPCoreInfo>&)>>> m_listCoresCallbacks;

	static const mNPCallbacks s_callbacks;
	static void cbServerConnected(mNPContext*, void* user);
	static void cbServerShutdown(mNPContext*, void* user);
	static void cbCoreRegistered(mNPContext*, const mNPCoreInfo*, uint32_t nonce, void* user);
	static void cbRoomJoined(mNPContext*, uint32_t roomId, uint32_t coreId, void* user);
	static void cbListRooms(mNPContext*, const struct mNPRoomInfo* rooms, uint32_t nRooms, void* user);
	static void cbListCores(mNPContext*, const struct mNPCoreInfo* cores, uint32_t nCores, uint32_t roomId, void* user);
	static void cbRollbackStart(mNPContext*, const struct mNPCoreInfo* cores, uint32_t nCores, void* user);
	static void cbRollbackEnd(mNPContext*, const struct mNPCoreInfo* cores, uint32_t nCores, void* user);
};

}

#endif
