#include <QtGui>
#include "app.h"
#include <iostream>
#include <cstdlib>
#define MAGIC_NUMBER	110807
#define PING_INTERVAL	5000
#define PING_TIME		15000					
	
using namespace std;

App::App(QWidget *parent) {
	setupUi(this);
	timer.setInterval(PING_INTERVAL);
	localtimer.setInterval(PING_INTERVAL);
	connect(&timer, SIGNAL(timeout()), this, SLOT(ping()));
	connect(&localtimer, SIGNAL(timeout()), this, SLOT(localTimerCheck()));
	TCPSocket *clientConnection = new TCPSocket;
	MessageReceiver *local_receiver=new MessageReceiver(clientConnection);
	connect(local_receiver, SIGNAL(chatMessageReceive(ChatMessage)), this, SLOT(localChatMessageReceive(ChatMessage)));
	connect(local_receiver, SIGNAL(playersListMessageReceive(PlayersListMessage)), this, SLOT(localPlayersListMessageReceive(PlayersListMessage)));
	connect(local_receiver, SIGNAL(serverReadyMessageReceive(ServerReadyMessage)), this, SLOT(localServerReadyMessageReceive(ServerReadyMessage)));
	connect(local_receiver, SIGNAL(clientConnectMessageReceive(ClientConnectMessage)), this, SLOT(localClientConnectMessageReceive(ClientConnectMessage)));
	connect(local_receiver, SIGNAL(clientDisconnectMessageReceive(ClientDisconnectMessage)), this, SLOT(localClientDisconnectMessageReceive(ClientDisconnectMessage)));
	connect(local_receiver, SIGNAL(connectionAcceptedMessageReceive(ConnectionAcceptedMessage)), this, SLOT(localConnectionAcceptedMessageReceive(ConnectionAcceptedMessage)));
	connect(local_receiver, SIGNAL(pingMessageReceive(PingMessage)), this, SLOT(localPingMessageReceive(PingMessage)));

	connect(local_receiver, SIGNAL(startGameMessageReceive(StartGameMessage)), this, SLOT(localStartGameMessageReceive(StartGameMessage)));
	connect(local_receiver, SIGNAL(turnMessageReceive(TurnMessage)), this, SLOT(localTurnMessageReceive(TurnMessage)));
	connect(local_receiver, SIGNAL(surrenderMessageReceive(SurrenderMessage)), this, SLOT(localSurrenderMessageReceive(SurrenderMessage)));

	connect(clientConnection, SIGNAL(connected()), this, SLOT(connected()));
	connect(clientConnection, SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(clientConnection, SIGNAL(error()), this, SLOT(error()));
	connect(&serverConnection, SIGNAL(newConnection()), this, SLOT(newConnection()));
	localClient.socket = clientConnection;
	localClient.receiver = local_receiver;
	connect(a_exit, SIGNAL(activated()), this, SLOT(exit()));
	connect(actionStartGame, SIGNAL(activated()), this, SLOT(startGame()));
	connect(actionDisconnectFromServer, SIGNAL(activated()), this, SLOT(disconnectFromServer()));
	connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
	connect(lineEdit, SIGNAL(returnPressed()), lineEdit, SLOT(clear()));
	
	connect(&listener, SIGNAL(readyRead()), this, SLOT(readyReadUDP()));
	dialog = new OptDialog(this);
	dialog->show();
}

void App::readyReadUDP() {
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

OptDialog::OptDialog(App* app) {
	setupUi(this);
	this->app = app;
	connect(pbSearch, SIGNAL(clicked()), this, SLOT(searchBtnClicked()));
	connect(pbConnect, SIGNAL(clicked()), this, SLOT(connectBtnClicked()));
	connect(cbCreateServer, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
	connect(&socket, SIGNAL(readyRead()), this, SLOT(getServersList()));
	timer.setInterval(1000);
	connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
	connect(lwServersList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClicked(QListWidgetItem*)));
}

void OptDialog::toggled(bool checked) {
	if (checked)
		pbConnect->setText("Create server and connect to it");
	else
		pbConnect->setText("Connect to server");
}

void OptDialog::connectBtnClicked() {
	if (timer.isActive()) {
		timer.stop();
		socket.close();
		pbSearch->setText("Start search");
		sbPort->setDisabled(false);
		servers.clear();
		textEdit->clear();
	}
	switch (comboBox->currentIndex()) {
		case 0: app->localClient.info.color = Qt::red; break;
		case 1: app->localClient.info.color = Qt::darkYellow; break;
		case 2: app->localClient.info.color = Qt::green; break;
		case 3: app->localClient.info.color = Qt::blue; break;
		default: QMessageBox::warning(this, "Error", "Incorrect color"); return;
	}
	app->localClient.info.name = leNickname->text();
	if (app->localClient.info.name=="") {
		QMessageBox::warning(this, "Error", "Enter nickname");
		return;
	} else if (app->localClient.info.name.toUtf8().size()>100) {
		QMessageBox::warning(this, "Error", "Your nickname is too long");
		return;
	}
	app->textEdit->clear();
	app->listWidget->clear();
	app->lineEdit->clear();
	if (!cbCreateServer->checkState()) { // connect to some server
		QString hostname = leServerIP->text();
		if (hostname=="") {
			QMessageBox::warning(this, "Error", "Enter host name");
			return;
		}
		int port = sbPort->value();
		close();
		app->game = new Game(app);
		//game signals
		connect(app->game, SIGNAL(turnDone(QString,QColor,QString,int,int,int)), app, SLOT(turnDone(QString,QColor,QString,int,int,int)));
		connect(app->game, SIGNAL(playerRetired(QString, QColor)), app, SLOT(playerSurrendered(QString,QColor)));
		app->show();
		app->localClient.socket->connectToHost(hostname, port);
		app->localtimer.start();
	} else { // create server and connect to it
		QString hostname = "localhost";
		int port = sbPort->value();
		app->maxClientsCount = sbClientsCount->value();
		bool listening = app->serverConnection.listen(port);
		if (listening) {
			app->timer.start();
			app->listener.bind(INADDR_ANY, port);
			close();
			app->game = new Game(app);
			//game signals
			connect(app->game, SIGNAL(turnDone(QString,QColor,QString,int,int,int)), app, SLOT(turnDone(QString,QColor,QString,int,int,int)));
			app->show();
			app->localClient.socket->connectToHost(hostname, port);
			app->localtimer.start();
		} else {
			QMessageBox::critical(this, "Error", app->serverConnection.errorString());
			return;
		}
	}
}

void OptDialog::itemClicked ( QListWidgetItem * item ) {
	textEdit->clear();
	if (servers.contains(item->text())) {
		QList<ClientInfo> list = servers.value(item->text());
		for (int i=0;i<list.size();++i) {
			textEdit->setTextColor(list[i].color);
			textEdit->append(list[i].name);
		}
	}
}

void OptDialog::getServersList() {
	if (socket.hasPendingDatagrams()) {
		qint64 datagramSize = socket.pendingDatagramSize();
		char *data = (char*)::malloc(datagramSize);
		QString address;
		quint16 port;
		int res=socket.readDatagram(data, datagramSize, &address, &port);
		if (!servers.contains(address)) {
			MessageHeader header;
			header.len = datagramSize-header.getLength();
			header.type = mtPlayersList;
			PlayersListMessage msg(header);
			msg.fill(QByteArray::fromRawData(data+header.getLength(), header.len));
			servers.insert(address, msg.getList());
			lwServersList->addItem(address);
		}
		free(data);
	}
}

void OptDialog::searchBtnClicked() {
	if (timer.isActive()) {
		timer.stop();
		socket.close();
		pbSearch->setText("Start search");
		sbPort->setDisabled(false);
		textEdit->clear();
	} else {
		lwServersList->clear();
		servers.clear();
		socket.bind(INADDR_ANY, 0);
		sbPort->setDisabled(true);
		timer.start();
		pbSearch->setText("Stop search");
	}
}

void OptDialog::timeout() {
	int query = MAGIC_NUMBER;
	quint16 port = sbPort->value();
	socket.writeDatagram((char*)&query, sizeof(query), INADDR_BROADCAST, port);
}

void App::turnDone(QString name,QColor color,QString tile,int id,int x,int y) {
	TurnMessage msg(name,color,tile,id,x,y);
	msg.send(localClient.socket);
}
void App::localSurrenderMessageReceive(SurrenderMessage msg)
{
	game->remotePlayerRetired(msg.getName(),msg.getColor());
}

void App::startGame() {
	if (serverConnection.isListening()) {
		int count = 0;
		for (int i=0;i<clients.size();++i)
			if (clients[i]->state==2 && clients[i]->socket->isConnected())
				++count;
		if (count==maxClientsCount) {
			game->start();
			StartGameMessage msg;
			sendToAll(&msg);
		} else
			QMessageBox::warning(this, "Warning", "Wait for "+QString::number(maxClientsCount)+" players");
	} else
		QMessageBox::warning(this, "Warning", "Only server can start the game");
}

void App::localStartGameMessageReceive(StartGameMessage msg) {
	game->start();
}

void App::localTurnMessageReceive(TurnMessage msg) {
	game->turnDone(msg.getColor(),msg.getMask(),msg.getId(),msg.getX(),msg.getY());
}

void App::remoteTurnMessageReceive(TurnMessage msg, RemoteClient*) {
	sendToAll(&msg);
}

void App::remoteSurrenderMessageReceive(SurrenderMessage msg, RemoteClient*) {
	sendToAll(&msg);
}

void App::perror(QString text) {
	textEdit->setTextColor(Qt::darkRed);
	textEdit->append(text);
}

void App::pinfo(QString text) {
	textEdit->setTextColor(Qt::darkBlue);
	textEdit->append(text);
}

void App::ping() {
	for (int i=0; i < clients.size(); ++i) {
		if (clients[i]->socket->isConnected()) {
			PingMessage msg;
			msg.send(clients[i]->socket);
			QTime last = clients[i]->lastpingtime;
			int elapsed = last.elapsed();
			if (elapsed > PING_TIME) {
				perror(QString::fromUtf8("Удаленный клиент отвалился"));
				clients[i]->socket->close();// client shut down
			}
		}
	}
}
//===========================Timer ping=========================================
void App::localPingMessageReceive(PingMessage msg) {
	msg.send(localClient.socket);
	localClient.lastpingtime.start();
}

void App::localTimerCheck() {
	int elapsed = localClient.lastpingtime.elapsed();
	if (elapsed > PING_TIME) {
		perror(QString::fromUtf8("Проверьте кабель"));
		localClient.socket->close();
	}
}

void App::remotePingMessageReceive(PingMessage, RemoteClient* client) {
	client->lastpingtime.start();
}
//===========================Timer ping=========================================
App::~App() {
	delete dialog;
	stopServer();
}

void App::exit() {
	std::exit(0);
}

void App::localChatMessageReceive(ChatMessage msg) {
	textEdit->setTextColor(msg.getColor());
	textEdit->append("("+QTime::currentTime().toString("hh:mm:ss")+") "+msg.getName()+":");
	textEdit->setTextColor(Qt::black);
	textEdit->append(msg.getText());
}

void App::remoteChatMessageReceive(ChatMessage msg, RemoteClient*) {
	sendToAll(&msg);
}

void App::localPlayersListMessageReceive(PlayersListMessage msg) {
	listWidget->clear();
	QList<ClientInfo> list = msg.getList();
	QList<bool> isLocal;
	for (int i=0;i<list.size();++i) {
		listWidget->addItem(list[i].name);
		isLocal.append(list[i].name==localClient.info.name&&list[i].color==localClient.info.color);
	}
	listWidget->sortItems();
	game->updatePlayers(list,isLocal);
}

void App::sendMessage() {
	if (localClient.socket->isConnected()) {
		QString text = lineEdit->text();
		if (text != "") {
			ChatMessage msg(localClient.info.name,text,localClient.info.color);
			msg.send(localClient.socket);
		}
	} else
		perror(QString::fromUtf8("Подключение утеряно"));
}

void App::sendToAll(Message *msg) {
	QList<RemoteClient*>::iterator i;
	for (i = clients.begin(); i != clients.end(); ++i)
		if ((*i)->socket->isConnected()&&(*i)->state==2) {
			msg->send((*i)->socket);
		}
}

void App::localConnectionAcceptedMessageReceive(ConnectionAcceptedMessage msg) {
	int code = msg.getCode();
	if (code != 0) {
		switch (msg.getCode()) {
			case 1: perror(QString::fromUtf8("Этот цвет уже используется")); break;
			case 2: perror(QString::fromUtf8("Этот ник уже используется")); break;
			case 3: perror(QString::fromUtf8("Игра уже начата. Попробуйте подключиться позднее")); break;
			case 4: perror(QString::fromUtf8("К игре уже подключились 4 игрока")); break;
		}
		localClient.socket->close();
		localtimer.stop();
	}
}

void App::disconnectFromServer() {
	localClient.socket->disconnectFromHost();
	localtimer.stop();
	stopServer();
	close();
	dialog->show();
}

void App::playerSurrendered(QString name,QColor color)
{
	SurrenderMessage msg(name,color);
	msg.send(localClient.socket);
}


void App::connected() {
	pinfo("Connected");
}

void App::localServerReadyMessageReceive(ServerReadyMessage) {
	TryToConnectMessage msg(localClient.info);
	msg.send(localClient.socket);
}

void App::sendPlayersList() {
	QList<ClientInfo> list;
	for (int i=0;i<clients.size();++i)
		if (clients[i]->state==2&&clients[i]->socket->isConnected())
			list.append(clients[i]->info);
	PlayersListMessage msg(list);
	sendToAll(&msg);
}

void App::newConnection() {
	if (serverConnection.hasPendingConnections()) {
		TCPSocket* s = serverConnection.nextPendingConnection();
		RemoteClient *client = new RemoteClient(s);
		connect(client, SIGNAL(rcChatMessageReceive(ChatMessage,RemoteClient*)), this, SLOT(remoteChatMessageReceive(ChatMessage,RemoteClient*)));
		connect(client, SIGNAL(rcCryToConnectMessageReceive(TryToConnectMessage,RemoteClient*)), this, SLOT(remoteTryToConnectMessageReceive(TryToConnectMessage,RemoteClient*)));
		connect(client, SIGNAL(rcPingMessageReceive(PingMessage,RemoteClient*)), this, SLOT(remotePingMessageReceive(PingMessage,RemoteClient*)));
		connect(client, SIGNAL(rcTurnMessageReceive(TurnMessage,RemoteClient*)), this, SLOT(remoteTurnMessageReceive(TurnMessage,RemoteClient*)));
		connect(client, SIGNAL(rcSurrenderMessageReceive(SurrenderMessage,RemoteClient*)), this, SLOT(remoteSurrenderMessageReceive(SurrenderMessage,RemoteClient*)));
		connect(client, SIGNAL(rcDisconnected(RemoteClient*)), this, SLOT(remoteDisconnected(RemoteClient*)));
		connect(client, SIGNAL(rcError(RemoteClient*)), this, SLOT(remoteError(RemoteClient*)));

		/*client->socket = s;
		client->receiver = rr;
		client->state = 1;*/
		clients.append(client);
		ServerReadyMessage msg;
		msg.send(s);
	}
}

void App::remoteDisconnected(RemoteClient *client) {
	ClientDisconnectMessage msg(client->info.name, client->info.color);
	removeClient(client);
	sendToAll(&msg);
	sendPlayersList();
}

void App::disconnected() {
	pinfo("Disconnected");
	listWidget->clear();
	delete game;
}

void App::error() {
	perror("local error "+localClient.socket->errorString());
	localClient.socket->close();
	localtimer.stop();
	stopServer();
}

void App::remoteError(RemoteClient *client) {
	client->socket->close();
}

void App::remoteTryToConnectMessageReceive(TryToConnectMessage msg, RemoteClient* client) {
	int i, error=0;
	for (i=0; i<clients.size() && (msg.getColor()!=clients[i]->info.color || clients[i]->state!=2); ++i) {}
	if (i!=clients.size())
		error=1;
	for (i=0; i<clients.size() && (msg.getName()!=clients[i]->info.name || clients[i]->state!=2); ++i) {}
	if (i!=clients.size())
		error=2;
	if (game->isStarted())
		error=3;
	int count = 0;
	for (i=0;i<clients.size();++i) if (clients[i]->state==2) ++count;
	if (count==maxClientsCount)
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

void App::localClientConnectMessageReceive(ClientConnectMessage msg) {
	textEdit->setTextColor(msg.getColor());
	textEdit->append(msg.getName()+" connected");
}

void App::localClientDisconnectMessageReceive(ClientDisconnectMessage msg) {
	textEdit->setTextColor(msg.getColor());
	textEdit->append(msg.getName()+" disconnected");
}

void App::stopServer() {
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

void App::removeClient(RemoteClient* client) {
	clients.removeAt(clients.indexOf(client));
	client->deleteLater();
}