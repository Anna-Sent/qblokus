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
	~Client() { std::cerr << "Client destructor" << std::endl; delete receiver; socket->deleteLater(); }
	Client():state(0),socket(NULL),receiver(NULL),lastpingtime(QTime::currentTime()) {}
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
	friend class OptDialog;
	void sendPlayersList();
public:
	Game *game;
public slots:
	void ping();
	void localPingMessageReceive(PingMessage);
	void remotePingMessageReceive(PingMessage);
	void sendMessage();
	void getMessageFromOtherClient(QByteArray);
	void connected();
	void disconnected();
	void otherClientDisconnected();
	void error();
	void errorFromOtherClient();
	void exit();
	void reconnectToServer();
	void disconnectFromServer();
	void newConnection();
	void chatMessageReceive(ChatMessage);
	void playersListMessageReceive(PlayersListMessage);
	void remoteClientConnectMessageReceive(ClientConnectMessage);
	void localServerReadyMessageReceive(ServerReadyMessage);
	void localClientConnectMessageReceive(ClientConnectMessage);
	void localConnectionAcceptedMessageReceive(ConnectionAcceptedMessage);
};
#endif