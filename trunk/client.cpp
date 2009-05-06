#include "client.h"

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
