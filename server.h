#ifndef _SERVER_H_
#define _SERVER_H_
#include <QObject>
#include <QList>
#include <QTimer>
#include "messagerecv.h"
#include "client.h"

class Server : public QThread {
	Q_OBJECT
private:
	TCPServer serverConnection;
	UDPSocket listener;
	QList<RemoteClient*> clients;
	QTimer timer;
	int maxClientsCount;
public:
	int getMaxClientsCount() {return maxClientsCount;}
	Server();
	QString getErrorString() {return serverConnection.errorString();}
	int getPlayersCount() {int count = 0;for(int i=0;i<clients.size();++i) if (clients[i]->state==2&&clients[i]->socket->isConnected()) ++count;return count;}
	bool start(int maxClientsCount, quint16 port);
private:
	void stop();
	void sendPlayersList();
	void sendToAll(Message*);
	void removeClient(RemoteClient*);
protected:
	void run();
private slots:
	//from app
	void startGame();
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
};
#endif