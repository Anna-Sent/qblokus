#ifndef APP_H
#define APP_H
#include "ui_racingForm.h"
#include "ui_optionsDialog.h"
#include "udpsocket.h"
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
	friend class OptDialog;
	void perror(QString);
	void pinfo(QString);
public:
	Game *game;
public slots:
	// from graphics
	void a_startGame();
	void le_sendMessage();
	void a_exit();
	void a_disconnectFromServer();
	// from local client
	void localError(QString);
	void localConnected();
	void localDisconnected();
	void localChatMessageReceive(ChatMessage);
	void localPlayersListMessageReceive(PlayersListMessage);
	void localClientConnectMessageReceive(ClientConnectMessage);
	void localClientDisconnectMessageReceive(ClientDisconnectMessage);
	void localConnectionAcceptedMessageReceive(ConnectionAcceptedMessage);
	// game signals
	void localTurnMessageReceive(TurnMessage);
	void localStartGameMessageReceive(StartGameMessage);
	void localSurrenderMessageReceive(SurrenderMessage);
signals:
	// to server
	void startGame();
	// to client
	void sendMessage(QString);
	void tolcTurnDone(QString name,QColor color,QString tile,int id,int x,int y);
	void tolcPlayerSurrendered(QString name,QColor color);
};
#endif