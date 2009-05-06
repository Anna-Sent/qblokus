#ifndef SOCKET_H_
#define SOCKET_H_

#include <QObject>
#include <QString>
#include <QThread>
#include <QMutex>
#include <QQueue>

class TCPServer;

class TCPSocket: public QThread
{
	Q_OBJECT
	public:
		TCPSocket();
		virtual ~TCPSocket();
		qint64 read ( char * data, qint64 maxSize );
		qint64 write ( const char * data, qint64 maxSize );
		virtual qint64 bytesAvailable () const;
		void connectToHost ( const QString & hostName, quint16 port );
		void disconnectFromHost ();
		virtual void close();
		bool isConnected();
		QString errorString() const;
		
		QString getHostname() const { return hostName; }
		quint16 getPort() const { return port; }
	private:
		int _d;
		QString hostName;
		QMutex buffer_lock;
		QMutex write_lock;
		int buffer_size, buffer_pos;
		void *buffer;
		quint16 port;
		TCPSocket(int descriptor);
		friend class TCPServer;
	protected:
		void run();
		bool doConnect();
	signals:
		void readyRead();
		void connected();
		void disconnected();
		void error();
};

class TCPServer: public QThread {
	Q_OBJECT
	public:
		TCPServer();
		virtual ~TCPServer();
		void close();
		virtual bool hasPendingConnections() const;
		bool isListening() const;
		bool listen(qint16 port);
		int maxPendingConnections() const;
		virtual TCPSocket* nextPendingConnection();
		quint16 serverPort() const;
		void setMaxPendingConnections(int numConnections);
		bool setSocketDescriptor() const;
		QString errorString() const;
	private:
		int _d;
		int _maxPendingConnections;
		qint16 _port;
		QQueue<TCPSocket*> _queue;
	protected:
		void run();
		bool doConnect();
		virtual void incomingConnection(int socketDescriptor);
	signals:
		void newConnection();
};
#endif

