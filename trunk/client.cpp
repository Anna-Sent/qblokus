#include "client.h"
#define PING_INTERVAL	5000
#define PING_TIME		1500000

RemoteClient::RemoteClient(TCPSocket *s) : state(1), lastpingtime(QTime::currentTime()) {
	MessageReceiver *rr = new MessageReceiver(s);
	connect(rr, SIGNAL(chatMessageReceive(ChatMessage)), this, SLOT(remoteChatMessageReceive(ChatMessage)));
	connect(rr, SIGNAL(tryToConnectMessageReceive(TryToConnectMessage)), this, SLOT(remoteTryToConnectMessageReceive(TryToConnectMessage)));
	connect(rr, SIGNAL(pingMessageReceive(PingMessage)), this, SLOT(remotePingMessageReceive(PingMessage)));
	connect(rr, SIGNAL(turnMessageReceive(TurnMessage)), this, SLOT(remoteTurnMessageReceive(TurnMessage)));
	connect(rr, SIGNAL(surrenderMessageReceive(SurrenderMessage)), this, SLOT(remoteSurrenderMessageReceive(SurrenderMessage)));
	connect(s, SIGNAL(disconnected()), this, SLOT(remoteDisconnected()));
	connect(s, SIGNAL(error()), this, SLOT(remoteError()));
	this->socket = s;
	this->receiver = rr;
	this->state = 1;
}

void RemoteClient::remoteChatMessageReceive(ChatMessage msg) {
	emit rcChatMessageReceive(msg,this);
}

void RemoteClient::remoteTryToConnectMessageReceive(TryToConnectMessage msg) {
	emit rcTryToConnectMessageReceive(msg,this);
}

void RemoteClient::remotePingMessageReceive(PingMessage msg) {
	emit rcPingMessageReceive(msg,this);
}

void RemoteClient::remoteTurnMessageReceive(TurnMessage msg) {
	emit rcTurnMessageReceive(msg,this);
}

void RemoteClient::remoteSurrenderMessageReceive(SurrenderMessage msg) {
	emit rcSurrenderMessageReceive(msg,this);
}

void RemoteClient::remoteDisconnected() {
	emit rcDisconnected(this);
}

void RemoteClient::remoteError() {
	emit rcError(this);
}

LocalClient::LocalClient():lastpingtime(QTime::currentTime()) {
	localtimer.setInterval(PING_INTERVAL);
	connect(&localtimer, SIGNAL(timeout()), this, SLOT(localTimerCheck()));
	socket = new TCPSocket;
	receiver = new MessageReceiver(socket);
	qRegisterMetaType<PlayersListMessage>("PlayersListMessage");
	qRegisterMetaType<ClientConnectMessage>("ClientConnectMessage");
	qRegisterMetaType<ServerReadyMessage>("ServerReadyMessage");
	qRegisterMetaType<ConnectionAcceptedMessage>("ConnectionAcceptedMessage");
	qRegisterMetaType<PingMessage>("PingMessage");
	qRegisterMetaType<StartGameMessage>("StartGameMessage");
	qRegisterMetaType<StartGameMessage>("RestartGameMessage");
	qRegisterMetaType<TurnMessage>("TurnMessage");
	qRegisterMetaType<ClientDisconnectMessage>("ClientDisconnectMessage");
	qRegisterMetaType<SurrenderMessage>("SurrenderMessage");
	qRegisterMetaType<ChatMessage>("ChatMessage");
	connect(receiver, SIGNAL(chatMessageReceive(ChatMessage)), this, SLOT(localChatMessageReceive(ChatMessage)));
	connect(receiver, SIGNAL(playersListMessageReceive(PlayersListMessage)), this, SLOT(localPlayersListMessageReceive(PlayersListMessage)));
	connect(receiver, SIGNAL(serverReadyMessageReceive(ServerReadyMessage)), this, SLOT(localServerReadyMessageReceive(ServerReadyMessage)));
	connect(receiver, SIGNAL(clientConnectMessageReceive(ClientConnectMessage)), this, SLOT(localClientConnectMessageReceive(ClientConnectMessage)));
	connect(receiver, SIGNAL(clientDisconnectMessageReceive(ClientDisconnectMessage)), this, SLOT(localClientDisconnectMessageReceive(ClientDisconnectMessage)));
	connect(receiver, SIGNAL(connectionAcceptedMessageReceive(ConnectionAcceptedMessage)), this, SLOT(localConnectionAcceptedMessageReceive(ConnectionAcceptedMessage)));
	connect(receiver, SIGNAL(pingMessageReceive(PingMessage)), this, SLOT(localPingMessageReceive(PingMessage)));
	connect(receiver, SIGNAL(startGameMessageReceive(StartGameMessage)), this, SLOT(localStartGameMessageReceive(StartGameMessage)));
	connect(receiver, SIGNAL(restartGameMessageReceive(RestartGameMessage)), this, SLOT(localRestartGameMessageReceive(RestartGameMessage)));
	connect(receiver, SIGNAL(turnMessageReceive(TurnMessage)), this, SLOT(localTurnMessageReceive(TurnMessage)));
	connect(receiver, SIGNAL(surrenderMessageReceive(SurrenderMessage)), this, SLOT(localSurrenderMessageReceive(SurrenderMessage)));
	connect(socket, SIGNAL(connected()), this, SLOT(localConnected()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(localDisconnected()));
	connect(socket, SIGNAL(error()), this, SLOT(localError()));
}

void LocalClient::localChatMessageReceive(ChatMessage msg) {
	emit lcChatMessageReceive(msg);
}

void LocalClient::localPlayersListMessageReceive(PlayersListMessage msg) {
	emit lcPlayersListMessageReceive(msg);
}

void LocalClient::localServerReadyMessageReceive(ServerReadyMessage msg) {
	TryToConnectMessage msg1(info);
	msg1.send(socket);
}

void LocalClient::localClientConnectMessageReceive(ClientConnectMessage msg) {
	emit lcClientConnectMessageReceive(msg);
}

void LocalClient::localClientDisconnectMessageReceive(ClientDisconnectMessage msg) {
	emit lcClientDisconnectMessageReceive(msg);
}

void LocalClient::localConnectionAcceptedMessageReceive(ConnectionAcceptedMessage msg) {
	emit lcConnectionAcceptedMessageReceive(msg);
}

void LocalClient::localStartGameMessageReceive(StartGameMessage msg) {
	emit lcStartGameMessageReceive(msg);
}

void LocalClient::localRestartGameMessageReceive(RestartGameMessage msg) {
	emit lcRestartGameMessageReceive(msg);
}

void LocalClient::localTurnMessageReceive(TurnMessage msg) {
	emit lcTurnMessageReceive(msg);
}

void LocalClient::localSurrenderMessageReceive(SurrenderMessage msg) {
	emit lcSurrenderMessageReceive(msg);
}

void LocalClient::localConnected() {
	emit lcConnected();
}

void LocalClient::localDisconnected() {
	emit lcDisconnected();
}

void LocalClient::localError() {
	emit lcError(socket->errorString());
}

void LocalClient::localPingMessageReceive(PingMessage msg) {
	msg.send(socket);
	lastpingtime.start();
}

void LocalClient::localTimerCheck() {
	int elapsed = lastpingtime.elapsed();
	if (elapsed > PING_TIME)
		emit lcError(QString::fromUtf8("Проверьте кабель"));
}

void LocalClient::sendMessage(QString text) {
	ChatMessage msg(info.name,text,info.color);
	msg.send(socket);
}

void LocalClient::turnDone(QString name,QColor color,QString tile,int id,int x,int y) {
	TurnMessage msg(name,color,tile,id,x,y);
	msg.send(socket);
}

void LocalClient::playerSurrendered(QString name,QColor color) {
	SurrenderMessage msg(name,color);
	msg.send(socket);
}