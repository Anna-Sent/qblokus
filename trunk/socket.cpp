#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <QMetaType>
#include "socket.h"
#define BUFFER_SIZE 65535
using namespace std;

TCPSocket::TCPSocket() {
	buffer=NULL;
	buffer_size=0;
	_d=0;
}

TCPSocket::TCPSocket(int descriptor) {
	_d=descriptor;
	start();
}

TCPSocket::~TCPSocket() {
	close();
}

qint64 TCPSocket::read ( char * data, qint64 maxSize ) {
	if (!buffer) return 0;
	size_t size = maxSize>buffer_size?buffer_size:maxSize;
	::bcopy((char*)buffer+buffer_pos,data,size);
	buffer_size -= size;
	buffer_pos += size;
	if (buffer_size<=0) buffer_lock.unlock();
	return size;
}

qint64 TCPSocket::bytesAvailable () const {
	return buffer_size;
}

QString TCPSocket::errorString() const {
	char* str = strerror(errorcode);
	int len = strlen(str);
	return errorcode!=0?QString::fromUtf8(str,len):"";
}

qint64 TCPSocket::write ( const char * data, qint64 maxSize ) {
	write_lock.lock();
	int len;
	len=::send(_d,data,maxSize,0);
	errorcode=errno;
	if (len==0)
		emit error();
	else if (len<0)
		emit error();
	write_lock.unlock();
	return len;
}

void TCPSocket::connectToHost ( const QString & hostName, quint16 port ) {
	if (_d>0) return;
	this->hostName=hostName;
	this->port=port;
	start();
}

void TCPSocket::disconnectFromHost () {
	write_lock.lock();
	close();
	write_lock.unlock();
}

void TCPSocket::close() {
	if (_d > 0) {
		::shutdown(_d,2);
		::close(_d);
		_d = 0;
		buffer_lock.unlock();
		if (isRunning()) wait();
		if (buffer) {
			free(buffer);
			buffer=NULL;
		}
		emit disconnected();
	}
}

bool TCPSocket::doConnect() {
	if ((_d = ::socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		_d = 0;
		errorcode=errno;
		emit error();
		return false;
	}
	int optval = 1;
	if (::setsockopt(_d, SOL_SOCKET, SO_REUSEADDR|SO_KEEPALIVE, &optval, sizeof optval) == -1) {
		errorcode=errno;
		emit error();
		return false;
	}
	struct hostent *hp;
	if ((hp=::gethostbyname(hostName.toUtf8().data()))==0) {
		errorcode=errno;
		emit error();
		return false;
	}
	sockaddr_in _addr;
	::bzero(&_addr, sizeof(_addr));
	::bcopy(hp->h_addr, &(_addr.sin_addr), hp->h_length);
	_addr.sin_family = hp->h_addrtype;
	_addr.sin_port = htons(port);
	if (::connect(_d, (sockaddr*)&_addr, sizeof(_addr)) == -1) {
		errorcode=errno;
		emit error();
		return false;
	}
	emit connected();
	return true;
}

void TCPSocket::run() {
	bool connected=true;
	if (_d==0) connected=doConnect();
	if (!connected) return;
	buffer = ::malloc(BUFFER_SIZE);
	while(1) {
		buffer_lock.lock();
		buffer_size=::recv(_d,buffer,BUFFER_SIZE,0);
		if (buffer_size>0) {
			buffer_pos = 0;
			emit readyRead();
		} else {
			if (_d!=0) {
				errorcode=errno;
				emit error();
			}
			buffer_lock.unlock();
			return;
		}
	}
}

bool TCPSocket::isConnected() {
	return _d > 0;
}

TCPServer::TCPServer() {
	_maxPendingConnections = 5;
	_d = 0;
}

TCPServer::~TCPServer() {
	close();
}

void TCPServer::close() {
	if (_d>0) {
		::shutdown(_d,2);
		::close(_d);
		_d = 0;
		_queue.clear();
	}
}

bool TCPServer::hasPendingConnections() const {
	return 0 < _queue.size();
}

bool TCPServer::isListening() const {
	return isRunning();
}

bool TCPServer::listen(qint16 port) {
	if (_d>0) return false;
	_port = port;
	if (!doConnect()) return false;
	_queue.clear();
	start();
	return true;
}

int TCPServer::maxPendingConnections() const {
	return _maxPendingConnections;
}

TCPSocket* TCPServer::nextPendingConnection() {
	return _queue.dequeue();
}

quint16 TCPServer::serverPort() const {
	return _port;
}

void TCPServer::setMaxPendingConnections(int numConnections) {
	_maxPendingConnections = numConnections;
}

void TCPServer::incomingConnection(int socketDescriptor) {
	TCPSocket *tcpSocket = new TCPSocket(socketDescriptor);
	_queue.enqueue(tcpSocket);
	emit newConnection();
}

QString TCPServer::errorString() const {
	char* str = strerror(errno);
	int len = strlen(str);
	return QString::fromUtf8(str,len);
}

void TCPServer::run() {
	while (_d>0) {
		// ожидаем новое подключение
		sockaddr_in _addr;
		::bzero(&_addr, sizeof(_addr));
		_addr.sin_family = AF_INET;
		_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		_addr.sin_port = htons(0);
		socklen_t addrlen = sizeof(_addr);
		int d;
		if ((d = ::accept(_d, (sockaddr*)&_addr, &addrlen)) > 0)
			incomingConnection(d);
		sleep(1);
	}
}

bool TCPServer::doConnect() {
	if ((_d = ::socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		_d = 0;
		::perror(NULL);
		return false;
	}
	int optval = 1;
	if (::setsockopt(_d, SOL_SOCKET, SO_REUSEADDR|SO_KEEPALIVE, &optval, sizeof optval) == -1) {
		::perror(NULL);
		::close(_d);
		_d = 0;
		return false;
	}
	int on = 1;
	if (::ioctl(_d, FIONBIO, (char*)&on) < 0) { // set non-blocking socket
		::close(_d);
		_d = 0;
		return false;
	}
	sockaddr_in _addr;
	::bzero(&_addr, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_addr.sin_port = htons(_port);
	if (::bind(_d, (sockaddr*)&_addr, sizeof(_addr)) < 0) {
		::perror(NULL);
		::close(_d);
		_d = 0;
		return false;
	}
	if (::listen(_d, _maxPendingConnections) == -1) {
		::perror(NULL);
		::close(_d);
		_d = 0;
		return false;
	}
	return true;
}