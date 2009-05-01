#include <QtGui>
#include "app.h"
#include <iostream>
#include <cstdlib>

using namespace std;

OptDialog::OptDialog(App* app) {
	setupUi(this);
	this->app = app;
	connect(pbSearch, SIGNAL(clicked()), this, SLOT(searchBtnClicked()));
	connect(pbConnect, SIGNAL(clicked()), this, SLOT(connectBtnClicked()));
	connect(cbCreateServer, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
}

App::App(QWidget *parent) {
	setupUi(this);
	game = new Game(this);
	timer.setInterval(5000);
	localtimer.setInterval(5000);
	connect(&timer, SIGNAL(timeout()), this, SLOT(ping()));
	connect(&localtimer, SIGNAL(timeout()), this, SLOT(localTimerCheck()));
	//qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
	TCPSocket *clientConnection = new TCPSocket;
	MessageReceiver *local_receiver=new MessageReceiver(clientConnection);
	connect(local_receiver, SIGNAL(chatMessageReceive(ChatMessage)), this, SLOT(chatMessageReceive(ChatMessage)));
	connect(local_receiver, SIGNAL(playersListMessageReceive(PlayersListMessage)), this, SLOT(playersListMessageReceive(PlayersListMessage)));
	connect(local_receiver, SIGNAL(serverReadyMessageReceive(ServerReadyMessage)), this, SLOT(localServerReadyMessageReceive(ServerReadyMessage)));
	connect(local_receiver, SIGNAL(clientConnectMessageReceive(ClientConnectMessage)), this, SLOT(localClientConnectMessageReceive(ClientConnectMessage)));
	connect(local_receiver, SIGNAL(connectionAcceptedMessageReceive(ConnectionAcceptedMessage)), this, SLOT(localConnectionAcceptedMessageReceive(ConnectionAcceptedMessage)));
	connect(local_receiver, SIGNAL(pingMessageReceive(PingMessage)), this, SLOT(localPingMessageReceive(PingMessage)));
	connect(clientConnection, SIGNAL(connected()), this, SLOT(connected()));
	connect(clientConnection, SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(clientConnection, SIGNAL(error()), this, SLOT(error()));
	connect(&serverConnection, SIGNAL(newConnection()), this, SLOT(newConnection()));
	localClient.socket = clientConnection;
	localClient.receiver = local_receiver;
	connect(a_exit, SIGNAL(activated()), this, SLOT(exit()));
	connect(actionDisconnectFromServer, SIGNAL(activated()), this, SLOT(disconnectFromServer()));
	connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
	connect(lineEdit, SIGNAL(returnPressed()), lineEdit, SLOT(clear()));
	connect(actionReconnectToServer, SIGNAL(activated()), this, SLOT(reconnectToServer()));
	dialog = new OptDialog(this);
	dialog->show();
}

void App::ping() {
	for (int i=0; i < clients.size(); ++i) {
		PingMessage msg;
		msg.send(clients[i]->socket);
		cerr << "send ping" << endl;
		//QTime cur = QTime::currentTime();
		QTime last = clients[i]->lastpingtime;
		int elapsed = last.elapsed();
		cerr << "elapsed " << elapsed << endl;
		if (elapsed > 15000)
			clients[i]->socket->close();// client shut down
		//qint64 delta = last.msec() + last.second()*1000 + last.minute()*1000*60 + last.hour()
	}
}

void App::localPingMessageReceive(PingMessage msg) {
	msg.send(localClient.socket);
	localClient.lastpingtime.start();
	cerr << "get ping" << endl;
}

void App::localTimerCheck() {
	int elapsed = localClient.lastpingtime.elapsed();
	if (elapsed > 15000)
		localClient.socket->close();
}

void App::remotePingMessageReceive(PingMessage) {
	MessageReceiver *r = dynamic_cast<MessageReceiver*>(sender());
	if (r) {
		int j;
		for (j = 0; j< clients.size() && r!=clients[j]->receiver; ++j) {}
		if (j != clients.size()) {
			clients[j]->lastpingtime.start();// = QTime::currentTime();
		}
	}
}

App::~App() {
	delete dialog;
	//delete local_receiver;
	//clientConnection.close();
	serverConnection.close();
	timer.stop();
	for (int j=0;j<clients.size();delete clients[j++]) {}
}

void App::exit() {
	std::exit(0);
}

void OptDialog::toggled(bool checked) {
	if (checked)
		pbConnect->setText("Create server and connect to it");
	else
		pbConnect->setText("Connect to server");
}

void App::reconnectToServer() {
	// disconnect
	bool isServer = false;
	QString hostname = localClient.socket->getHostname();
	quint16 port = localClient.socket->getPort();
	localClient.socket->disconnectFromHost();
	localtimer.stop();
	if (serverConnection.isListening()) {
		serverConnection.close();
		timer.stop();
		while (clients.size()>0) {
			Client *client = clients[0];
			clients.removeAt(0);
			client->socket->close();
			client->deleteLater();
		}
		clients.clear();
		isServer = true;
	}
	// connect
	if (isServer) {
		bool listening = serverConnection.listen(port);
		if (listening) {
			timer.start();
		} else {
			QMessageBox::critical(this, "Error", serverConnection.errorString());
			return;
		}
	}
	localClient.socket->connectToHost(hostname, port);
	localtimer.start();
}

void App::chatMessageReceive(ChatMessage msg) {
	textEdit->setTextColor(msg.getColor());
	textEdit->append("("+QTime::currentTime().toString("hh:mm:ss")+") "+msg.getName()+":");
	textEdit->setTextColor(Qt::black);
	textEdit->append(msg.getText());
}

void App::playersListMessageReceive(PlayersListMessage msg) {
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
		textEdit->append(QString::fromUtf8("Подключение утеряно"));
}

void App::getMessageFromOtherClient(QByteArray data) {
	QList<Client*>::iterator i;
	for (i = clients.begin(); i != clients.end(); ++i)
		if ((*i)->socket->isConnected()&&(*i)->state==2)
			(*i)->socket->write(data.data(), data.size());
}

void App::localConnectionAcceptedMessageReceive(ConnectionAcceptedMessage msg) {
	int code = msg.getCode();
	if (code != 0) {
		switch (msg.getCode()) {
			case 1: textEdit->append(QString::fromUtf8("Этот цвет уже используется")); break;
			case 2: textEdit->append(QString::fromUtf8("Этот ник уже используется")); break;
		}
		localClient.socket->close();//disconnectFromHost();
		localtimer.stop();
//		default: cerr << "unknown error code " << /*msg.getCode()<<*/ endl;
	}
}

void App::disconnectFromServer() {
	localClient.socket->disconnectFromHost();
	localtimer.stop();
	if (serverConnection.isListening()) {
		serverConnection.close();
		timer.stop();
		while (clients.size()>0) {
			Client *client = clients[0];
			clients.removeAt(0);
			client->socket->close();
			client->deleteLater(); //delete client;//clients[j];//remote_receivers[j];
			//clients[j]->close();
			//clients[j]->deleteLater();
			//lete playersList[j];
		}		cerr << clients.size() <<" size disconnect from servers"<< endl;
		clients.clear();
		//remote_receivers.clear();
		//playersList.clear();
		cerr<<"close server"<<endl;
	}
	close();
	dialog->show();
}

void OptDialog::searchBtnClicked() {
	int port = sbPort->value();
}

void OptDialog::connectBtnClicked() {
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
		app->show();
		app->localClient.socket->connectToHost(hostname, port);
		app->localtimer.start();
	} else { // create server and connect to it
		QString hostname = "localhost";
		int port = sbPort->value();
		int clientscount = sbClientsCount->value();
		bool listening = app->serverConnection.listen(port);
		app->timer.start();
		if (listening) {
			close();
			app->show();
			app->localClient.socket->connectToHost(hostname, port);
			app->localtimer.start();
		} else {
			QMessageBox::critical(this, "Error", app->serverConnection.errorString());
			return;
		}
	}
	//app->game = new Game(app);
}

void App::connected() {
	textEdit->append("Connected");
}

void App::localServerReadyMessageReceive(ServerReadyMessage) {
	ClientConnectMessage msg(localClient.info.name, localClient.info.color);
	msg.send(localClient.socket);
}

void App::sendPlayersList() {
	QList<ClientInfo> list;
	for (int i=0;i<clients.size();++i)
		list.append(clients[i]->info);
	PlayersListMessage msg(list);
	getMessageFromOtherClient(msg.serialize());
}

void App::newConnection() {
	textEdit->append("Connected yet another client");
	cerr << "NEW CONNECTION" << endl;
	if (serverConnection.hasPendingConnections()) {
		TCPSocket* s = serverConnection.nextPendingConnection();
		MessageReceiver *rr = new MessageReceiver(s);
		connect(rr, SIGNAL(getMessage(QByteArray)), this, SLOT(getMessageFromOtherClient(QByteArray)));
		connect(rr, SIGNAL(clientConnectMessageReceive(ClientConnectMessage)), this, SLOT(remoteClientConnectMessageReceive(ClientConnectMessage)));
		connect(rr, SIGNAL(pingMessageReceive(PingMessage)), this, SLOT(remotePingMessageReceive(PingMessage)));
		//remote_receivers.append(rr);
		connect(s, SIGNAL(disconnected()), this, SLOT(otherClientDisconnected()));
		connect(s, SIGNAL(error()), this, SLOT(errorFromOtherClient()));
		//clients.append(s);

		Client *client = new Client;
		client->socket = s;
		client->receiver = rr;
		client->state = 1;
		clients.append(client);
		cerr << clients.size() <<" size"<< endl;
		ServerReadyMessage msg;
		msg.send(s);
	}
}

void App::otherClientDisconnected() {
	cerr <<"enter otherClientDisconnected"<<endl;
	TCPSocket *s = dynamic_cast<TCPSocket*>(sender());
	if (s) {
	int i;
	for (i = 0; i < clients.size(); ++i)
		if (s == clients[i]->socket)
			break;
	if (i != clients.size()) {
		Client *client = clients[i];
		clients.removeAt(i);
		client->deleteLater();
		cerr << clients.size() <<" size"<< endl;

		sendPlayersList();
	} else s->deleteLater();
}	cerr <<"exit otherClientDisconnected"<<endl;
}

void App::disconnected() {
	textEdit->append("Disconnected");
	listWidget->clear();
}

void App::error() {
	cerr <<"enter error"<<endl;
	textEdit->append("local error "+localClient.socket->errorString());
	localClient.socket->close();
	localtimer.stop();
	if (serverConnection.isListening()) { serverConnection.close(); timer.stop(); }
	cerr <<"exit error"<<endl;
}

void App::errorFromOtherClient() {
	cerr <<"enter errorFromOtherClient"<<endl;
	TCPSocket *s = dynamic_cast<TCPSocket*>(sender());
	if (s) {
	textEdit->append("remote error "+s->errorString());
	s->close(); }
	cerr <<"exit errorFromOtherClient"<<endl;
}
//////////////////////////////////////////////////////////////////
void App::remoteClientConnectMessageReceive(ClientConnectMessage msg) {
	cerr<<"1111!!"<<endl;
	MessageReceiver *r = dynamic_cast<MessageReceiver*>(sender());
	if (r) {
		int j;
		for (j = 0; j< clients.size() && r!=clients[j]->receiver; ++j) {}
		if (j != clients.size()) {
			textEdit->setTextColor(msg.getColor());
			textEdit->append(msg.getName()+" connected remote!!!!!!!!!");
			int i, error=0;
			for (i=0; i<clients.size()&&msg.getColor()!=clients[i]->info.color; ++i) {}
			if (i!=clients.size()) {
				error=1;
			}
			for (i=0; i<clients.size()&&msg.getName()!=clients[i]->info.name; ++i) {}
			if (i!=clients.size()) {
				error=2;
			}cerr<<"33333"<<endl;
			cerr << "error " << error << endl;
			ConnectionAcceptedMessage msg1(error);cerr<<"3.5"<< clients[j]->socket<<endl;
			msg1.send(clients[j]->socket);cerr<<"44444"<<endl;
			if (!error) {cerr <<"33333" <<endl;
				clients[j]->info.name = msg.getName();
				clients[j]->info.color = msg.getColor();
				clients[j]->state = 2;
				//game->addPlayer(msg.getName(),msg.getColor(),ptNetwork);
				sendPlayersList();
			} else {
		cerr << clients.size() <<" size"<< endl;
				Client *client = clients[j];
				clients.removeAt(j);
				client->socket->close(); client->deleteLater(); //delete client;//s[j];
		cerr << clients.size() <<" size"<< endl;
			}
			cerr <<"received rem"<<endl;
		}
	}
} //from client to server

void App::localClientConnectMessageReceive(ClientConnectMessage msg) {
	cerr <<"received loc"<<endl;
	textEdit->setTextColor(msg.getColor());
	textEdit->append(msg.getName()+" connected local!!!!!!");
}