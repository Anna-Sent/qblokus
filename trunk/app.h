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

class LocalClient : public QObject {
	Q_OBJECT
public:
	QTime lastpingtime; // for remote and local client
	TCPSocket* socket;
	MessageReceiver* receiver;
	ClientInfo info;
public:
	~LocalClient() { delete receiver; socket->deleteLater(); }
	LocalClient():lastpingtime(QTime::currentTime()),socket(NULL),receiver(NULL) {}
};

class RemoteClient : public QObject {
	Q_OBJECT
public:
	int state; // for remote client
	QTime lastpingtime; // for remote and local client
	TCPSocket* socket;
	MessageReceiver* receiver;
	ClientInfo info;
public:
	~RemoteClient() { delete receiver; socket->deleteLater(); }
	RemoteClient():state(0),lastpingtime(QTime::currentTime()),socket(NULL),receiver(NULL) {}
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
	void removeClient(int);
public:
	Game *game;
public slots:
	void readyReadUDP();
	void startGame();
	void ping();
	void localPingMessageReceive(PingMessage);
	void remotePingMessageReceive(PingMessage);
	void localTimerCheck();
	void sendMessage();
	void remoteChatMessageReceive(ChatMessage);
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
	void localSurrenderMessageReceive(SurrenderMessage);
	void remoteSurrenderMessageReceive(SurrenderMessage);

	void turnDone(QString,QColor,QString,int,int,int);
	void playerSurrendered(QString,QColor);
};
#endif