#ifndef APP_H
#define APP_H
#include "ui_racingForm.h"
#include "ui_optionsDialog.h"
#include "socket.h"
#include "messagerecv.h"
#include <iostream>
#include "game.h"
#include <QTimer>
#include <QTime>

class App;

class OptDialog : public QDialog, public Ui::Dialog {
	Q_OBJECT
public:
	OptDialog(App* app);
private:
	App *app;
public slots:
    void connectBtnClicked();
    void searchBtnClicked();
	void toggled(bool);
};

class Client : public QObject {
	Q_OBJECT
public:
	int state; // for remote client
	QTime lastpingtime; // for remote client
	TCPSocket* socket;
	MessageReceiver* receiver;
	ClientInfo info;
	~Client() { delete receiver; socket->deleteLater(); }
	Client():state(0),lastpingtime(QTime::currentTime()),socket(NULL),receiver(NULL) {}
};

class App : public QMainWindow, public Ui::MainWindow {
    Q_OBJECT
public:
    App(QWidget *parent = 0);
    ~App();
private:
	OptDialog *dialog;
	Client localClient;
	TCPServer serverConnection;
	QList<Client*> clients;
	QTimer timer;
	QTimer localtimer;
	friend class OptDialog;
	void sendPlayersList();
	void perror(QString);
	void pinfo(QString);
	void sendToAll(Message*);
public:
	Game *game;
public slots:
	void startGame();
	void ping();
	void localPingMessageReceive(PingMessage);
	void remotePingMessageReceive(PingMessage);
	void localTimerCheck();
	void sendMessage();
	void remoteChatMessageReceive(ChatMessage);//getMessageFromOtherClient(QByteArray);
	void connected();
	void disconnected();
	void remoteDisconnected();
	void error();
	void remoteError();
	void exit();
	void disconnectFromServer();
	void newConnection();
	void localChatMessageReceive(ChatMessage);
	void localPlayersListMessageReceive(PlayersListMessage);
	void remoteTryToConnectMessageReceive(TryToConnectMessage);
	void localServerReadyMessageReceive(ServerReadyMessage);
	void localClientConnectMessageReceive(ClientConnectMessage);
	void localClientDisconnectMessageReceive(ClientDisconnectMessage);
	void localConnectionAcceptedMessageReceive(ConnectionAcceptedMessage);
	// game signals
	void localTurnMessageReceive(TurnMessage);
	void localStartGameMessageReceive(StartGameMessage);
	void remoteTurnMessageReceive(TurnMessage);
	
	void turnDone(QString,QColor,int,int,int);
	//void remoteStartGameMessageReceive(StartGameMessage);
};
#endif