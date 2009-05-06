#ifndef APP_H
#define APP_H
#include "ui_racingForm.h"
#include "ui_optionsDialog.h"
#include "socket.h"
#include "udpsocket.h"
#include "messagerecv.h"
#include <iostream>
#include "game.h"
#include <QTimer>
#include <QTime>
#include "client.h"

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
	LocalClient localClient;
	TCPServer serverConnection;
	UDPSocket listener;
	QList<RemoteClient*> clients;
	QTimer timer;
	QTimer localtimer;
	int maxClientsCount;
	friend class OptDialog;
	void sendPlayersList();
	void perror(QString);
	void pinfo(QString);
	void sendToAll(Message*);
	void stopServer();
	void removeClient(RemoteClient*);
public:
	Game *game;
public slots:
	void readyReadUDP();
	void startGame();
	void ping();
	void localPingMessageReceive(PingMessage);
	void localTimerCheck();
	void sendMessage();
	void connected();
	void disconnected();
	void error();
	void exit();
	void disconnectFromServer();
	void newConnection();
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
	//from remote client
	void remoteChatMessageReceive(ChatMessage,RemoteClient*);
	void remoteTryToConnectMessageReceive(TryToConnectMessage,RemoteClient*);
	void remotePingMessageReceive(PingMessage,RemoteClient*);
	void remoteTurnMessageReceive(TurnMessage,RemoteClient*);
	void remoteSurrenderMessageReceive(SurrenderMessage,RemoteClient*);
	void remoteDisconnected(RemoteClient*);
	void remoteError(RemoteClient*);
};
#endif