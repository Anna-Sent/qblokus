#ifndef _SERVER_H_
#define _SERVER_H_
#include <QObject>
#include <QList>
#include <QTimer>
#include "messagerecv.h"
#include "client.h"

class Server : public QObject {
	Q_OBJECT
public:
	TCPServer serverConnection;
	UDPSocket listener;
	QList<RemoteClient*> clients;
	QTimer timer;
	int maxClientsCount;
	void stop();
	void sendToAll(Message*);
private:
	void sendPlayersList();
	void removeClient(RemoteClient*);
private slots:
	//from timer
	void ping();
	//from udpsocket
	void readyReadUDP();
	//from tcpserver
	void newConnection();
	//from remote client
	void remoteChatMessageReceive(ChatMessage,RemoteClient*);
	void remoteTryToConnectMessageReceive(TryToConnectMessage,RemoteClient*);
	void remotePingMessageReceive(PingMessage,RemoteClient*);
	void remoteTurnMessageReceive(TurnMessage,RemoteClient*);
	void remoteSurrenderMessageReceive(SurrenderMessage,RemoteClient*);
	void remoteDisconnected(RemoteClient*);
	void remoteError(RemoteClient*);
signals:
	void error(QString);
};
#endif