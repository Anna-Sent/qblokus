#ifndef _CLIENT_H_
#define _CLIENT_H_
#include "socket.h"
#include "udpsocket.h"
#include "messagerecv.h"
//#include <iostream>
//#include <QTimer>
#include <QTime>
#include <QObject>
//using namespace std;

class LocalClient : public QObject {
	Q_OBJECT
public:
	QTime lastpingtime; // for remote and local client
	TCPSocket* socket;
	MessageReceiver* receiver;
	ClientInfo info;
public:
	~LocalClient() { delete receiver; socket->deleteLater(); }
	LocalClient():lastpingtime(QTime::currentTime()),socket(NULL),receiver(NULL) {}
};

class RemoteClient : public QObject {
	Q_OBJECT
public:
	int state; // for remote client
	QTime lastpingtime; // for remote and local client
	TCPSocket* socket;
	MessageReceiver* receiver;
	ClientInfo info;
public:
	~RemoteClient() { delete receiver; socket->deleteLater(); }
	RemoteClient(TCPSocket*);
private slots:
	void remoteChatMessageReceive(ChatMessage);
	void remoteTryToConnectMessageReceive(TryToConnectMessage);
	void remotePingMessageReceive(PingMessage);
	void remoteTurnMessageReceive(TurnMessage);
	void remoteSurrenderMessageReceive(SurrenderMessage);
	void remoteDisconnected();
	void remoteError();
signals:
	void rcChatMessageReceive(ChatMessage,RemoteClient*);
	void rcTryToConnectMessageReceive(TryToConnectMessage,RemoteClient*);
	void rcPingMessageReceive(PingMessage,RemoteClient*);
	void rcTurnMessageReceive(TurnMessage,RemoteClient*);
	void rcSurrenderMessageReceive(SurrenderMessage,RemoteClient*);
	void rcDisconnected(RemoteClient*);
	void rcError(RemoteClient*);
};
#endif