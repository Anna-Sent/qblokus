#ifndef _CLIENT_H_
#define _CLIENT_H_
#include "socket.h"
#include "udpsocket.h"
#include "messagerecv.h"
#include <QTimer>
#include <QTime>
#include <QObject>
#include <QThread>
#include <iostream>
using namespace std;

class LCWrapper : public QThread {
	protected:
		void run()
		{
			exec();
		}
};

class LocalClient: public QObject {
	Q_OBJECT
private:
	QTimer localtimer;
private:
	QTime lastpingtime;
	TCPSocket* socket;
	MessageReceiver* receiver;
	ClientInfo info;
	void stop() {socket->close();localtimer.stop();}
public:
	~LocalClient() { delete receiver; socket->deleteLater(); }
	void quit() {stop();}
	LocalClient();
	void start(QString hostname, quint16 port) {socket->connectToHost(hostname, port);localtimer.start();cerr<<"client started"<<endl;}
	void setNickname(QString name) {info.name=name;}
	void setColor(QColor color) {info.color=color;}
	QString getNickname() {return info.name;}
	QColor getColor() {return info.color;}
	bool isConnected() {return socket->isConnected();}
/*protected:
	void run() {exec();cerr<<"client exec"<<endl;stop();cerr<<"client stopped"<<endl;}*/
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
	void lcClientConnectMessageReceive(ClientConnectMessage);
	void lcClientDisconnectMessageReceive(ClientDisconnectMessage);
	void lcConnectionAcceptedMessageReceive(ConnectionAcceptedMessage);
	void lcStartGameMessageReceive(StartGameMessage);
	void lcTurnMessageReceive(TurnMessage);
	void lcSurrenderMessageReceive(SurrenderMessage);
	void lcConnected();
	void lcDisconnected();
	void lcError(QString);
};

class RemoteClient : public QObject {
	Q_OBJECT
public:
	int state;
	QTime lastpingtime;
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
