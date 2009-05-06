#ifndef APP_H
#define APP_H
#include "ui_racingForm.h"
#include "ui_optionsDialog.h"
//#include "socket.h"
#include "udpsocket.h"
//#include "messagerecv.h"
#include <iostream>
#include "game.h"
#include <QTimer>
#include <QTime>
#include "client.h"
#include "server.h"

class App;

class OptDialog : public QDialog, public Ui::Dialog {
	Q_OBJECT
public:
	OptDialog(App* app);
private:
	App *app;
	UDPSocket socket;
	QMap<QString, QList<ClientInfo> > servers;
	QTimer timer;
public slots:
    void connectBtnClicked();
    void searchBtnClicked();
	void toggled(bool);
	void getServersList();
	void timeout();
	void itemClicked ( QListWidgetItem * item );
};

class App : public QMainWindow, public Ui::MainWindow {
    Q_OBJECT
public:
    App(QWidget *parent = 0);
    ~App();
private:
	OptDialog *dialog;
	Server server;
	LocalClient localClient;
	QTimer localtimer;
	friend class OptDialog;
	void perror(QString);
	void pinfo(QString);
public:
	Game *game;
public slots:
	void a_startGame();
	void localPingMessageReceive(PingMessage);
	void localTimerCheck();
	void le_sendMessage();
	void connected();
	void disconnected();
	void error();
	void a_exit();
	void a_disconnectFromServer();
	void localChatMessageReceive(ChatMessage);
	void localPlayersListMessageReceive(PlayersListMessage);
	void localServerReadyMessageReceive(ServerReadyMessage);
	void localClientConnectMessageReceive(ClientConnectMessage);
	void localClientDisconnectMessageReceive(ClientDisconnectMessage);
	void localConnectionAcceptedMessageReceive(ConnectionAcceptedMessage);
	// game signals
	void localTurnMessageReceive(TurnMessage);
	void localStartGameMessageReceive(StartGameMessage);
	void localSurrenderMessageReceive(SurrenderMessage);
	void turnDone(QString,QColor,QString,int,int,int);
	void playerSurrendered(QString,QColor);
signals:
	void startGame();
};
#endif