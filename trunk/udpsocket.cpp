#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include "udpsocket.h"
using namespace std;

UDPSocket::UDPSocket() {
	_d=0;
	::memset(&bound_addr, 0, sizeof(bound_addr));
	buffer_size = 0;
	::memset(&buffer, 0, DATAGRAM_SIZE);
	::memset(&sender_addr, 0, sizeof(sender_addr));
	port = 0;
}

UDPSocket::~UDPSocket() {
	close();
}

qint64 UDPSocket::readDatagram ( char * data, qint64 maxSize, QString * address, quint16 * port ) {
	int len = buffer_size<maxSize?buffer_size:maxSize;
	::memcpy(data, buffer, len);
	if (address)
		*address = QString::fromUtf8(inet_ntoa(sender_addr.sin_addr));
	if (port)
		*port = ntohs(sender_addr.sin_port);
	buffer_size = 0;
	buffer_lock.unlock();
	return len;
}

void UDPSocket::close() {
	if (_d > 0) {
		::shutdown(_d,2);
		::close(_d);
		_d = 0;
		buffer_lock.unlock();
		if (isRunning()) wait();
	}
}

QString UDPSocket::errorString() const {
	char* str = ::strerror(errno);
	int len = ::strlen(str);
	return QString::fromUtf8(str,len);
}

QString UDPSocket::getAddress() const { return QString::fromUtf8(inet_ntoa(bound_addr.sin_addr)); }

qint64 UDPSocket::writeDatagram ( const char * data, qint64 size, const uint32_t address, quint16 port ) {
	sockaddr_in remote_addr;
	::memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = htonl(address);
	remote_addr.sin_port = htons(port);
	socklen_t addrlen = sizeof(remote_addr);
	cerr<<"send to "<<inet_ntoa(remote_addr.sin_addr)<<":"<<remote_addr.sin_port <<"("<<port<<")"<<endl;
	return ::sendto(_d, (const void*)data, size, 0, (sockaddr*)&remote_addr, addrlen);
}

qint64 UDPSocket::writeDatagram ( const char * data, qint64 size, QString address, quint16 port ) {
	sockaddr_in remote_addr;
    struct hostent *hp;
    if ((hp=gethostbyname(address.toUtf8().data()))==0)
		return -1;
	::memset(&remote_addr, 0, sizeof(remote_addr));
	::memcpy(&(remote_addr.sin_addr), hp->h_addr, hp->h_length);
    remote_addr.sin_family = hp->h_addrtype;
    remote_addr.sin_port = htons(port);
	socklen_t addrlen = sizeof(remote_addr);
	cerr<<"send to "<<inet_ntoa(remote_addr.sin_addr)<<":"<<remote_addr.sin_port <<"("<<port<<")"<<endl;
	return ::sendto(_d, (const void*)data, size, 0, (sockaddr*)&remote_addr, addrlen);
}


bool UDPSocket::bind ( uint32_t address, quint16 port ) {
	if (_d>0) return false;
	if ((_d = ::socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		_d = 0;
		return false;
	}
	int broadcast=1;
	if (::setsockopt(_d, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
		::close(_d);
		_d = 0;
		return false;
	}
	::memset(&bound_addr, 0, sizeof(bound_addr));
	bound_addr.sin_family = AF_INET;
	bound_addr.sin_addr.s_addr = htonl(address);
	bound_addr.sin_port = htons(port);
	this->port = port;
	if (::bind(_d, (sockaddr*)&bound_addr, sizeof(bound_addr)) < 0) {
		::close(_d);
		_d = 0;
		return false;
	}
	start();
	return true;
}

void UDPSocket::run() {
	socklen_t addrlen = sizeof(sender_addr);
	while (_d>0) {
		buffer_lock.lock();
		buffer_size = ::recvfrom(_d, (void*)buffer, DATAGRAM_SIZE, 0, (sockaddr*)&sender_addr, &addrlen);
		perror(NULL);
		if (buffer_size>0) emit readyRead();
		else buffer_size = 0;
   	}
   	buffer_lock.unlock();
}