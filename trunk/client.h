#ifndef _CLIENT_H_
#define _CLIENT_H_
#include "socket.h"
#include "udpsocket.h"
#include "messagerecv.h"
//#include <iostream>
#include <QTimer>
#include <QTime>
#include <QObject>
#include <QThread>
//using namespace std;

class LocalClient : public QThread {
	Q_OBJECT
private:
	QTimer localtimer;
private:
	QTime lastpingtime; // for remote and local client
	TCPSocket* socket;
	MessageReceiver* receiver;
	ClientInfo info;
	void stop() {socket->close();localtimer.stop();}
public:
	~LocalClient() { delete receiver; socket->deleteLater(); }
	LocalClient();
	void start(QString hostname, quint16 port) {socket->connectToHost(hostname, port);localtimer.start();QThread::start();}
	void setNickname(QString name) {info.name=name;}
	void setColor(QColor color) {info.color=color;}
	QString getNickname() {return info.name;}
	QColor getColor() {return info.color;}
	bool isConnected() {return socket->isConnected();}
protected:
	void run() {exec();stop();}
private slots:
	void localChatMessageReceive(ChatMessage);
	void localPlayersListMessageReceive(PlayersListMessage);
	void localServerReadyMessageReceive(ServerReadyMessage);
	void localClientConnectMessageReceive(ClientConnectMessage);
	void localClientDisconnectMessageReceive(ClientDisconnectMessage);
	void localConnectionAcceptedMessageReceive(ConnectionAcceptedMessage);
	void localPingMessageReceive(PingMessage);
	void localStartGameMessageReceive(StartGameMessage);
	void localTurnMessageReceive(TurnMessage);
	void localSurrenderMessageReceive(SurrenderMessage);
	void localConnected();
	void localDisconnected();
	void localError();
	// from timer
	void localTimerCheck();
	// from form
	void sendMessage(QString text);
	void turnDone(QString name,QColor color,QString tile,int id,int x,int y);
	void playerSurrendered(QString name,QColor color);
signals:
	void lcChatMessageReceive(ChatMessage);
	void lcPlayersListMessageReceive(PlayersListMessage);
	//void lcServerReadyMessageReceive(ServerReadyMessage);
	void lcClientConnectMessageReceive(ClientConnectMessage);
	void lcClientDisconnectMessageReceive(ClientDisconnectMessage);
	void lcConnectionAcceptedMessageReceive(ConnectionAcceptedMessage);
	//void lcPingMessageReceive(PingMessage);
	void lcStartGameMessageReceive(StartGameMessage);
	void lcTurnMessageReceive(TurnMessage);
	void lcSurrenderMessageReceive(SurrenderMessage);
	void lcConnected();
	void lcDisconnected();
	void lcError();
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