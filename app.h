#ifndef APP_H
#define APP_H
#include "ui_racingForm.h"
#include "ui_optionsDialog.h"
#include "socket.h"
#include "messagerecv.h"

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
};

class Client {
public:
	TCPSocket* socket;
	MessageReceiver* receiver;
	ClientInfo info;
	~Client() { delete receiver; socket->deleteLater(); }
};

class App : public QMainWindow, public Ui::MainWindow {
    Q_OBJECT
public:
    App(QWidget *parent = 0);
    ~App();
private:
	OptDialog *dialog;
	Client localClient;
	//TCPSocket clientConnection;
	TCPServer serverConnection;
	//MessageReceiver *local_receiver;
	//QString localClientName;
	//QColor localClientColor;
	QList<Client*> clients;
	friend class OptDialog;
	void sendPlayersList();
public slots:
	void sendMessage();
	void getMessageFromOtherClient(QByteArray);
	void connected();
	void disconnected();
	void otherClientDisconnected();
	void error();
	void errorFromOtherClient();
	void exit();
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
