#ifndef UDPSOCKET_H_
#define UDPSOCKET_H_
#include <arpa/inet.h>
#include <QObject>
#include <QString>
#include <QThread>
#include <QMutex>
#define DATAGRAM_SIZE 512

class UDPSocket: public QThread {
	Q_OBJECT
	public:
		UDPSocket();
		virtual ~UDPSocket();
		bool hasPendingDatagrams () const {return buffer_size>0;}
		qint64 pendingDatagramSize () const {return buffer_size;}
		qint64 readDatagram ( char * data, qint64 maxSize, QString * address = 0, quint16 * port = 0 );
		qint64 writeDatagram ( const char * data, qint64 size, QString address, quint16 port );
		qint64 writeDatagram ( const char * data, qint64 size, uint32_t address, quint16 port );
		virtual void close();
		bool isConnected() {return _d>0;}
		QString errorString() const;
		QString getAddress() const;
		quint16 getPort() const { return port; }
		bool bind ( uint32_t address, quint16 port );
	private:
		int _d;
		sockaddr_in bound_addr;
		int buffer_size;
		char buffer[DATAGRAM_SIZE];
		sockaddr_in sender_addr;
		quint16 port;
		QMutex buffer_lock;
	protected:
		void run();
	signals:
		void readyRead();
};
#endif
