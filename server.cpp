#include "server.h"
#define MAGIC_NUMBER	110807
#define PING_INTERVAL	5000
#define PING_TIME		15000		

void Server::readyReadUDP() {
	if (listener.hasPendingDatagrams()) {
		qint64 datagramSize = listener.pendingDatagramSize();
		if (datagramSize == sizeof(int)) {
			int data;
			QString address;
			quint16 port;
			listener.readDatagram((char*)&data, datagramSize, &address, &port);
			if (data == MAGIC_NUMBER) {
				QList<ClientInfo> list;
				for (int i=0;i<clients.size();++i)
					if (clients[i]->state==2&&clients[i]->socket->isConnected())
						list.append(clients[i]->info);
				PlayersListMessage msg(list);
				QByteArray data = msg.serialize();
				int res = listener.writeDatagram(data.data(), data.size(), address, port);
			}
		}
	}
}

void Server::remotePingMessageReceive(PingMessage, RemoteClient* client) {
	client->lastpingtime.start();
}

void Server::removeClient(RemoteClient* client) {
	clients.removeAt(clients.indexOf(client));
	client->deleteLater();
}

Server::Server() {
	connect(&serverConnection, SIGNAL(newConnection()), this, SLOT(newConnection()));
	connect(&listener, SIGNAL(readyRead()), this, SLOT(readyReadUDP()));
	timer.setInterval(PING_INTERVAL);
	connect(&timer, SIGNAL(timeout()), this, SLOT(ping()));
}

bool Server::start(int maxClientsCount, quint16 port) {
	this->maxClientsCount = maxClientsCount;
	bool listening = serverConnection.listen(port);
	if (listening) {
		timer.start();
		listener.bind(INADDR_ANY, port);
		QThread::start();
		cerr<<"server started"<<endl;
	}
	return listening;
}

void Server::run() {
	exec();
	cerr<<"server exec"<<endl;
	stop();
	cerr<<"server stopped"<<endl;
}

void Server::stop() {
	if (serverConnection.isListening()) {
		serverConnection.close();
		listener.close();
		timer.stop();
		while (clients.size()>0) {
			removeClient(0);
		}
		clients.clear();
	}
}

void Server::ping() {
	for (int i=0; i < clients.size(); ++i) {
		if (clients[i]->socket->isConnected()) {
			PingMessage msg;
			msg.send(clients[i]->socket);
			QTime last = clients[i]->lastpingtime;
			int elapsed = last.elapsed();
			cerr<<"server: elapsed "<<elapsed<<endl;
			if (elapsed > PING_TIME) {
				clients[i]->socket->close();
			}
		}
	}
}

void Server::remoteTurnMessageReceive(TurnMessage msg, RemoteClient*) {
	sendToAll(&msg);
}

void Server::remoteSurrenderMessageReceive(SurrenderMessage msg, RemoteClient*) {
	sendToAll(&msg);
}

void Server::remoteTryToConnectMessageReceive(TryToConnectMessage msg, RemoteClient* client) {
	int i, error=0;
	for (i=0; i<clients.size() && (msg.getColor()!=clients[i]->info.color || clients[i]->state!=2); ++i) {}
	if (i!=clients.size())
		error=1;
	for (i=0; i<clients.size() && (msg.getName()!=clients[i]->info.name || clients[i]->state!=2); ++i) {}
	if (i!=clients.size())
		error=2;
	if (getPlayersCount()==maxClientsCount)
		error=4;
	ConnectionAcceptedMessage msg1(error);
	msg1.send(client->socket);
	if (!error) {
		client->info.name = msg.getName();
		client->info.color = msg.getColor();
		client->state = 2;
		ClientConnectMessage msg1(msg.getName(), msg.getColor());
		sendToAll(&msg1);
		sendPlayersList();
	} else
		removeClient(client);
}

void Server::remoteError(RemoteClient *client) {
	client->socket->close();
}

void Server::newConnection() {
	if (serverConnection.hasPendingConnections()) {
		TCPSocket* s = serverConnection.nextPendingConnection();
		RemoteClient *client = new RemoteClient(s);
		connect(client, SIGNAL(rcChatMessageReceive(ChatMessage,RemoteClient*)), this, SLOT(remoteChatMessageReceive(ChatMessage,RemoteClient*)));
		connect(client, SIGNAL(rcTryToConnectMessageReceive(TryToConnectMessage,RemoteClient*)), this, SLOT(remoteTryToConnectMessageReceive(TryToConnectMessage,RemoteClient*)));
		connect(client, SIGNAL(rcPingMessageReceive(PingMessage,RemoteClient*)), this, SLOT(remotePingMessageReceive(PingMessage,RemoteClient*)));
		connect(client, SIGNAL(rcTurnMessageReceive(TurnMessage,RemoteClient*)), this, SLOT(remoteTurnMessageReceive(TurnMessage,RemoteClient*)));
		connect(client, SIGNAL(rcSurrenderMessageReceive(SurrenderMessage,RemoteClient*)), this, SLOT(remoteSurrenderMessageReceive(SurrenderMessage,RemoteClient*)));
		connect(client, SIGNAL(rcDisconnected(RemoteClient*)), this, SLOT(remoteDisconnected(RemoteClient*)));
		connect(client, SIGNAL(rcError(RemoteClient*)), this, SLOT(remoteError(RemoteClient*)));
		clients.append(client);
		ServerReadyMessage msg;
		msg.send(s);
	}
}

void Server::remoteDisconnected(RemoteClient *client) {
	ClientDisconnectMessage msg(client->info.name, client->info.color);
	removeClient(client);
	sendToAll(&msg);
	sendPlayersList();
}

void Server::remoteChatMessageReceive(ChatMessage msg, RemoteClient*) {
	sendToAll(&msg);
}

void Server::sendToAll(Message *msg) {
	QList<RemoteClient*>::iterator i;
	for (i = clients.begin(); i != clients.end(); ++i)
		if ((*i)->socket->isConnected()&&(*i)->state==2) {
			msg->send((*i)->socket);
		}
}


void Server::sendPlayersList() {
	QList<ClientInfo> list;
	for (int i=0;i<clients.size();++i)
		if (clients[i]->state==2&&clients[i]->socket->isConnected())
			list.append(clients[i]->info);
	PlayersListMessage msg(list);
	sendToAll(&msg);
}

void Server::startGame() {
	StartGameMessage msg;
	sendToAll(&msg);
}