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
}

App::App(QWidget *parent) {
	setupUi(this);
	//qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
	local_receiver=new MessageReceiver(&clientConnection);
	connect(local_receiver, SIGNAL(chatMessageReceive(ChatMessage)), this, SLOT(chatMessageReceive(ChatMessage)));
	connect(local_receiver, SIGNAL(playersListMessageReceive(PlayersListMessage)), this, SLOT(playersListMessageReceive(PlayersListMessage)));
	connect(local_receiver, SIGNAL(serverReadyMessageReceive(ServerReadyMessage)), this, SLOT(localServerReadyMessageReceive(ServerReadyMessage)));
	connect(local_receiver, SIGNAL(clientConnectMessageReceive(ClientConnectMessage)), this, SLOT(localClientConnectMessageReceive(ClientConnectMessage)));
	connect(local_receiver, SIGNAL(connectionAcceptedMessageReceive(ConnectionAcceptedMessage)), this, SLOT(localConnectionAcceptedMessageReceive(ConnectionAcceptedMessage)));

	connect(&clientConnection, SIGNAL(connected()), this, SLOT(connected()));
	connect(&clientConnection, SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(&clientConnection, SIGNAL(error()), this, SLOT(error()));
	connect(&serverConnection, SIGNAL(newConnection()), this, SLOT(newConnection()));
	connect(a_exit, SIGNAL(activated()), this, SLOT(exit()));
	connect(actionDisconnectFromServer, SIGNAL(activated()), this, SLOT(disconnectFromServer()));
	connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
	connect(lineEdit, SIGNAL(returnPressed()), lineEdit, SLOT(clear()));
	dialog = new OptDialog(this);
	dialog->show();
}

App::~App() {
	delete dialog;
	delete local_receiver;
	clientConnection.close();
	serverConnection.close();
	for (int j=0;j<clients.size();delete clients[j++]) {}
}

void App::exit() {
	std::exit(0);
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
	for (int i=0;i<list.size();++i)
		listWidget->addItem(list[i].name);
	listWidget->sortItems();
}

void App::sendMessage() {
	if (clientConnection.isConnected()) {
		QString text = lineEdit->text();
		if (text != "") {
			ChatMessage msg(localClientName,text,localClientColor);
			msg.send(&clientConnection);
		}
	} else
		textEdit->append(QString::fromUtf8("Подключение утеряно"));
}

void App::getMessageFromOtherClient(QByteArray data) {
	QList<Client*>::iterator i;
	for (i = clients.begin(); i != clients.end(); ++i)
		if ((*i)->socket->isConnected())
			(*i)->socket->write(data.data(), data.size());
}

void App::localConnectionAcceptedMessageReceive(ConnectionAcceptedMessage msg) {
	cerr<<"get conn acc"<<endl;switch (msg.getCode()) {
		case 0: cerr << "accepted" << endl; break;
		case 1: cerr << "bad color" << endl; break;
		case 2: cerr << "bad name" << endl; break;
		default: cerr << "unknown error code " << /*msg.getCode()<<*/ endl;
	}
}

void App::disconnectFromServer() {
	clientConnection.disconnectFromHost();
	if (serverConnection.isListening()) {
		serverConnection.close();
		for (int j=0;j<clients.size();++j) {
			Client *client = clients[j];
			clients.removeAt(j);
			delete client;//clients[j];//remote_receivers[j];
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
		case 0: app->localClientColor = Qt::red; break;
		case 1: app->localClientColor = Qt::yellow; break;
		case 2: app->localClientColor = Qt::green; break;
		case 3: app->localClientColor = Qt::blue; break;
		default: QMessageBox::warning(this, "Error", "Incorrect color"); return;
	}
	app->localClientName = leNickname->text();
	if (app->localClientName=="") {
		QMessageBox::warning(this, "Error", "Enter nickname");
		return;
	}
	if (!cbCreateServer->checkState()) { // connect to some server
		QString hostname = leServerIP->text();
		if (hostname=="") {
			QMessageBox::warning(this, "Error", "Enter host name");
			return;
		}
		int port = sbPort->value();
		close();
		app->textEdit->clear();
		app->show();
		app->clientConnection.connectToHost(hostname, port);
	} else { // create server and connect to it
		QString hostname = "localhost";
		int port = sbPort->value();
		int clientscount = sbClientsCount->value();
		bool listening = app->serverConnection.listen(port);
		if (listening) {
			close();
			app->textEdit->clear();
			app->show();
			app->clientConnection.connectToHost(hostname, port);
		} else {
			QMessageBox::critical(this, "Error", app->serverConnection.errorString());
			return;
		}
	}
}

void App::connected() {
	textEdit->append("Connected");
}

void App::localServerReadyMessageReceive(ServerReadyMessage) {
	ClientConnectMessage msg(localClientName, localClientColor);
	msg.send(&clientConnection);
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

		//remote_receivers.append(rr);
		connect(s, SIGNAL(disconnected()), this, SLOT(otherClientDisconnected()));
		connect(s, SIGNAL(error()), this, SLOT(errorFromOtherClient()));
		//clients.append(s);

		Client *client = new Client;
		client->socket = s;
		client->receiver = rr;
		clients.append(client);
		cerr << clients.size() <<" size"<< endl;
		ServerReadyMessage msg;
		msg.send(s);
	}
}

void App::otherClientDisconnected() {
	TCPSocket *s = dynamic_cast<TCPSocket*>(sender());
	if (s) {
	int i;
	for (i = 0; i < clients.size(); ++i)
		if (s == clients[i]->socket)
			break;
	if (i != clients.size()) {
		//delete remote_receivers[i];
		//s->deleteLater();
		//cerr << "remove at "<<i<<" " << playersList[i].name.toUtf8().data()<<endl;
		//playersList.removeAt(i);???????????????????
		Client *client = clients[i];
		clients.removeAt(i);
		delete client;//s[i];
		cerr << clients.size() <<" size"<< endl;

		sendPlayersList();
		/*PlayersListMessage msg(playersList);
		QByteArray data = msg.serialize();
		getMessageFromOtherClient(data);*/
	} else s->deleteLater();
}
}

void App::disconnected() {
	textEdit->append("Disconnected");
}

void App::error() {
	cerr << "local errror" << endl;
	textEdit->append("local error "+clientConnection.errorString());
	clientConnection.close();
	if (serverConnection.isListening()) serverConnection.close();
	listWidget->clear();
}

void App::errorFromOtherClient() {
	TCPSocket *s = dynamic_cast<TCPSocket*>(sender());
	if (s) {
	textEdit->append("remote error "+s->errorString());
	s->close(); }
	//s->deleteLater();
}
//////////////////////////////////////////////////////////////////
void App::remoteClientConnectMessageReceive(ClientConnectMessage msg) {
	cerr<<"1111!!"<<endl;
	MessageReceiver *r = dynamic_cast<MessageReceiver*>(sender());
	if (r) {
		int j;
		for (j = 0; j< clients.size() && r!=clients[j]->receiver; ++j) {}
			//if (r == clients[j]->receiver)
				//break;cerr << "22222222" << endl;
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
				sendPlayersList();
			} else {
		cerr << clients.size() <<" size"<< endl;
				Client *client = clients[j];
				clients.removeAt(j);
				delete client;//s[j];
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

/*
	playersList.append(msg.getInfo());
	PlayersListMessage msg1(playersList);
	QByteArray data = msg1.serialize();
	getMessageFromOtherClient(data);
	cerr << "send"<<endl;*/
}