#include "socket.h"
#include "udpsocket.h"
#include "messagerecv.h"
//#include <iostream>
//#include <QTimer>
#include <QTime>
#include <QObject>
//using namespace std;

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

