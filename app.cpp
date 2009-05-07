#include <QtGui>
#include "app.h"
#include <iostream>
#include <cstdlib>
#define MAGIC_NUMBER	110807
using namespace std;
//=====================================OptDialog============================================
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
	}
	switch (comboBox->currentIndex()) {
		case 0: app->localClient.setColor(Qt::red); break;
		case 1: app->localClient.setColor(Qt::darkYellow); break;
		case 2: app->localClient.setColor(Qt::green); break;
		case 3: app->localClient.setColor(Qt::blue); break;
		default: QMessageBox::warning(this, "Error", "Incorrect color"); return;
	}
	app->localClient.setNickname(leNickname->text());
	if (app->localClient.getNickname()=="") {
		QMessageBox::warning(this, "Error", "Enter nickname");
		return;
	} else if (app->localClient.getNickname().toUtf8().size()>100) {
		QMessageBox::warning(this, "Error", "Your nickname is too long");
		return;
	}
	app->textEdit->clear();
	app->listWidget->clear();
	app->lineEdit->clear();
	int port = sbPort->value();
	if (!cbCreateServer->checkState()) { // connect to some server
		QString hostname = leServerIP->text();
		if (hostname=="") {
			QMessageBox::warning(this, "Error", "Enter host name");
			return;
		}
		textEdit->clear();
		lwServersList->clear();
		servers.clear();
		close();
		app->game = new Game(app);
		//game signals
		connect(app->game, SIGNAL(turnDone(QString,QColor,QString,int,int,int)), &(app->localClient), SLOT(turnDone(QString,QColor,QString,int,int,int)));
		connect(app->game, SIGNAL(playerRetired(QString, QColor)), &(app->localClient), SLOT(playerSurrendered(QString,QColor)));
		app->show();
		app->localClient.start(hostname,port);
	} else { // create server and connect to it
		QString hostname = "localhost";
		int maxClientsCount = sbClientsCount->value();
		bool listening = app->server.start(maxClientsCount,port);
		if (listening) {
			textEdit->clear();
			lwServersList->clear();
			servers.clear();
			close();
			app->game = new Game(app);
			//game signals
			connect(app->game, SIGNAL(turnDone(QString,QColor,QString,int,int,int)), &(app->localClient), SLOT(turnDone(QString,QColor,QString,int,int,int)));
			connect(app->game, SIGNAL(playerRetired(QString, QColor)), &(app->localClient), SLOT(playerSurrendered(QString,QColor)));
			app->show();
			app->localClient.start(hostname, port);
		} else {
			QMessageBox::critical(this, "Error", app->server.getErrorString());
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
	if (timer.isActive()) { // stop pressed
		timer.stop();
		socket.close();
		pbSearch->setText("Start search");
		sbPort->setDisabled(false);
	} else { // start pressed
		lwServersList->clear();
		textEdit->clear();
		servers.clear();
		socket.bind(INADDR_ANY, 0);
		timer.start();
		sbPort->setDisabled(true);
		pbSearch->setText("Stop search");
	}
}

void OptDialog::timeout() {
	int query = MAGIC_NUMBER;
	quint16 port = sbPort->value();
	socket.writeDatagram((char*)&query, sizeof(query), INADDR_BROADCAST, port);
}
//=========================Functions======================================================
App::App(QWidget *parent) {
	setupUi(this);
	connect(&localClient, SIGNAL(lcChatMessageReceive(ChatMessage)), this, SLOT(localChatMessageReceive(ChatMessage)));
	connect(&localClient, SIGNAL(lcPlayersListMessageReceive(PlayersListMessage)), this, SLOT(localPlayersListMessageReceive(PlayersListMessage)));
	connect(&localClient, SIGNAL(lcClientConnectMessageReceive(ClientConnectMessage)), this, SLOT(localClientConnectMessageReceive(ClientConnectMessage)));
	connect(&localClient, SIGNAL(lcClientDisconnectMessageReceive(ClientDisconnectMessage)), this, SLOT(localClientDisconnectMessageReceive(ClientDisconnectMessage)));
	connect(&localClient, SIGNAL(lcConnectionAcceptedMessageReceive(ConnectionAcceptedMessage)), this, SLOT(localConnectionAcceptedMessageReceive(ConnectionAcceptedMessage)));
	connect(&localClient, SIGNAL(lcStartGameMessageReceive(StartGameMessage)), this, SLOT(localStartGameMessageReceive(StartGameMessage)));
	connect(&localClient, SIGNAL(lcRestartGameMessageReceive(RestartGameMessage)), this, SLOT(localRestartGameMessageReceive(RestartGameMessage)));
	connect(&localClient, SIGNAL(lcTurnMessageReceive(TurnMessage)), this, SLOT(localTurnMessageReceive(TurnMessage)));
	connect(&localClient, SIGNAL(lcSurrenderMessageReceive(SurrenderMessage)), this, SLOT(localSurrenderMessageReceive(SurrenderMessage)));
	connect(&localClient, SIGNAL(lcConnected()), this, SLOT(localConnected()));
	connect(&localClient, SIGNAL(lcDisconnected()), this, SLOT(localDisconnected()));
	connect(&localClient, SIGNAL(lcError(QString)), this, SLOT(localError(QString)));

	connect(actionExit, SIGNAL(activated()), this, SLOT(a_exit()));
	connect(actionStartGame, SIGNAL(activated()), this, SLOT(a_startGame()));
	connect(actionDisconnectFromServer, SIGNAL(activated()), this, SLOT(a_disconnectFromServer()));
	connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(le_sendMessage()));
	connect(lineEdit, SIGNAL(returnPressed()), lineEdit, SLOT(clear()));
	
	connect(this, SIGNAL(startGame()), &server, SLOT(startGame()));
	connect(this, SIGNAL(restartGame(QList<ClientInfo>)), &server, SLOT(restartGame(QList<ClientInfo>)));
	connect(this, SIGNAL(sendMessage(QString)), &localClient, SLOT(sendMessage(QString)));
	localClient.moveToThread(&lcw);
	lcw.start();
	dialog = new OptDialog(this);
	dialog->show();
}

void App::perror(QString text) {
	textEdit->setTextColor(Qt::darkRed);
	textEdit->append(text);
}

void App::pinfo(QString text) {
	textEdit->setTextColor(Qt::darkBlue);
	textEdit->append(text);
}

App::~App() {
	delete dialog;
	server.quit();
	localClient.quit();
}
//==============================Graphics slots============================================
void App::a_startGame() {
	if (server.isRunning()) {
		if (server.getPlayersCount()==server.getMaxClientsCount()) {
			if (game) {
				if (game->isStarted()) {
					QMessageBox msgBox;
					msgBox.setText(QString::fromUtf8("Новая игра"));
					msgBox.setInformativeText(QString::fromUtf8("Начать новую игру?"));
					msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
					msgBox.setDefaultButton(QMessageBox::No);
					msgBox.setIcon(QMessageBox::Warning);
					int ret = msgBox.exec();
					switch(ret) {
						case(QMessageBox::No): return;
						case(QMessageBox::Yes): break;
					}
					delete game;
					game=new Game(this);
					QList<ClientInfo> list = server.getClients();
					QList<bool> isLocal;
					for (int i=0;i<list.size();++i)
						isLocal.append(list[i].name==localClient.getNickname()&&list[i].color==localClient.getColor());
					game->updatePlayers(list,isLocal);
					game->start();
					emit restartGame(list);
				} else {
					game->start();
					emit startGame();
				}
			}
		} else
			QMessageBox::warning(this, "Warning", "Wait for "+QString::number(server.getMaxClientsCount())+" players");
	} else
		QMessageBox::warning(this, "Warning", "Only server can start the game");
}

void App::a_exit() {
	std::exit(0);
}

void App::a_disconnectFromServer() {
	if (game&&game->isStarted()) {
		QMessageBox msgBox;
		msgBox.setText(QString::fromUtf8("Отключение"));
		msgBox.setInformativeText(QString::fromUtf8("Выйти в диалог верхнего уровня?"));
		msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::No);
		msgBox.setIcon(QMessageBox::Warning);
		int ret = msgBox.exec();
		switch(ret) {
			case(QMessageBox::No): return;
			case(QMessageBox::Yes): break;
		}
	}
	localClient.quit();
	server.quit();
	close();
	dialog->show();
}

void App::le_sendMessage() {
	if (localClient.isConnected()) {
		QString text = lineEdit->text();
		if (text != "")
			emit sendMessage(text);
	} else
		perror(QString::fromUtf8("Подключение утеряно"));
}
//===============================LocalClient slots========================================
void App::localSurrenderMessageReceive(SurrenderMessage msg) {
	game->remotePlayerRetired(msg.getName(),msg.getColor());
}

void App::localStartGameMessageReceive(StartGameMessage msg) {
	game->start();
}

void App::localRestartGameMessageReceive(RestartGameMessage msg) {
	if (game) {
		delete game;
		game=new Game(this);
		QList<ClientInfo> list = msg.getList();
		QList<bool> isLocal;
		for (int i=0;i<list.size();++i)
			isLocal.append(list[i].name==localClient.getNickname()&&list[i].color==localClient.getColor());
		game->updatePlayers(list,isLocal);
		game->start();
	}
}

void App::localTurnMessageReceive(TurnMessage msg) {
	game->turnDone(msg.getColor(),msg.getMask(),msg.getId(),msg.getX(),msg.getY());
}

void App::localChatMessageReceive(ChatMessage msg) {
	textEdit->setTextColor(msg.getColor());
	textEdit->append("("+QTime::currentTime().toString("hh:mm:ss")+") "+msg.getName()+":");
	textEdit->setTextColor(Qt::black);
	textEdit->append(msg.getText());
}

void App::localPlayersListMessageReceive(PlayersListMessage msg) {
	listWidget->clear();
	QList<ClientInfo> list = msg.getList();
	QList<bool> isLocal;
	for (int i=0;i<list.size();++i) {
		listWidget->addItem(list[i].name);
		isLocal.append(list[i].name==localClient.getNickname()&&list[i].color==localClient.getColor());
	}
	listWidget->sortItems();
	game->updatePlayers(list,isLocal);
}

void App::localConnectionAcceptedMessageReceive(ConnectionAcceptedMessage msg) {
	int code = msg.getCode();
	if (code != 0) {
		switch (msg.getCode()) {
			case 1: perror(QString::fromUtf8("Этот цвет уже используется")); break;
			case 2: perror(QString::fromUtf8("Этот ник уже используется")); break;
			case 4: perror(QString::fromUtf8("К игре уже подключились ")+
				QString::number(server.getMaxClientsCount())
				+QString::fromUtf8(" игрока")); break;
		}
		localClient.quit();
	}
}

void App::localConnected() {
	pinfo("Connected");
}

void App::localDisconnected() {
	pinfo("Disconnected");
	listWidget->clear();
	if (game) {delete game;game=NULL;}
}

void App::localError(QString msg) {
	perror("Local error: "+msg);
	localClient.quit();
	server.quit();
}

void App::localClientConnectMessageReceive(ClientConnectMessage msg) {
	textEdit->setTextColor(msg.getColor());
	textEdit->append(msg.getName()+" connected");
}

void App::localClientDisconnectMessageReceive(ClientDisconnectMessage msg) {
	textEdit->setTextColor(msg.getColor());
	textEdit->append(msg.getName()+" disconnected");
}