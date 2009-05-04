#include <stdlib.h>
#include "messagerecv.h"
#include "socket.h"
#include "udpsocket.h"
#include <iostream>
using namespace std;

void Message::send(TCPSocket* socket) const {
	QByteArray data = serialize();
	socket->write(data.data(), data.size());
}

MessageReceiver::MessageReceiver(TCPSocket* socket) {
	current = new MessageHeader;
	this->socket=socket;
	QObject::connect(socket,SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void MessageReceiver::readyRead() {
	int avail = socket->bytesAvailable();
	char *tmp = (char*)malloc(avail);
	int len = socket->read(tmp,avail);
	buffer.append(QByteArray(tmp,len));
	while (current->getLength()<=buffer.size())
	{
		current->fill(buffer);
		{
			ChatMessage *msg;
			if ((msg=dynamic_cast<ChatMessage*>(current))) {
					emit chatMessageReceive(*msg);
					//emit getMessage(msg->serialize());
			}
		}
		{
			PlayersListMessage *msg;
			if ((msg=dynamic_cast<PlayersListMessage*>(current))) {
				emit playersListMessageReceive(*msg);
				//emit getMessage(msg->serialize());
			}
		}
		{
			ClientConnectMessage *msg;
			if ((msg=dynamic_cast<ClientConnectMessage*>(current))) {
				emit clientConnectMessageReceive(*msg);
				//emit getMessage(msg->serialize());
			}
		}
		{
			TryToConnectMessage *msg;
			if ((msg=dynamic_cast<TryToConnectMessage*>(current))) {
				emit tryToConnectMessageReceive(*msg);
				//emit getMessage(msg->serialize());
			}
		}
		{
			ClientDisconnectMessage *msg;
			if ((msg=dynamic_cast<ClientDisconnectMessage*>(current))) {
				emit clientDisconnectMessageReceive(*msg);
				//emit getMessage(msg->serialize());
			}
		}
		{
			ServerReadyMessage *msg;
			if ((msg=dynamic_cast<ServerReadyMessage*>(current))) {
				emit serverReadyMessageReceive(*msg);
				//emit getMessage(msg->serialize());
			}
		}
		{
			ConnectionAcceptedMessage *msg;
			if ((msg=dynamic_cast<ConnectionAcceptedMessage*>(current))) {
				emit connectionAcceptedMessageReceive(*msg);
				//emit getMessage(msg->serialize());
			}
		}
		{
			PingMessage *msg;
			if ((msg=dynamic_cast<PingMessage*>(current))) {
				emit pingMessageReceive(*msg);
				//emit getMessage(msg->serialize());
			}
		}
		{
			StartGameMessage *msg;
			if ((msg=dynamic_cast<StartGameMessage*>(current))) {
				emit startGameMessageReceive(*msg);
				//emit getMessage(msg->serialize());
			}
		}
		{
			TurnMessage *msg;
			if ((msg=dynamic_cast<TurnMessage*>(current))) {
				emit turnMessageReceive(*msg);
				//emit getMessage(msg->serialize());
			}
		}
		{
			SurrenderMessage *msg;
			if ((msg=dynamic_cast<SurrenderMessage*>(current))) {
				emit surrenderMessageReceive(*msg);
				//emit getMessage(msg->serialize());
			}
		}

		buffer.remove(0,current->getLength());
		Message* old = current;
		current = current->next();
		delete old;
	}
}

Message* Message::next() const {
	return new MessageHeader;
}

void MessageHeader::fill(const QByteArray& buffer) {
	const char* data = buffer.data();
	::bcopy(data, &type, sizeof(MessageType));
	data += sizeof(MessageType);
	::bcopy(data, &len, sizeof(qint64));
}

Message* MessageHeader::next() const {
	switch(type) {
		case (mtHeader):
			return new MessageHeader;
		case (mtChat):
			return new ChatMessage(*this);
		case (mtPlayersList):
			return new PlayersListMessage(*this);
		case (mtClientConnect):
			std::cerr << "clconnmess created\n";
			return new ClientConnectMessage(*this);
		case (mtClientDisconnect):
			return new ClientDisconnectMessage(*this);
		case (mtServerReady):
			return new ServerReadyMessage(*this);
		case (mtConnectionAccepted):
			return new ConnectionAcceptedMessage(*this);
		case (mtPing):
			return new PingMessage(*this);
		case (mtTryToConnect):
			return new TryToConnectMessage(*this);
		case (mtStartGame):
			return new StartGameMessage(*this);
		case (mtTurn):
			return new TurnMessage(*this);
		case (mtSurrender):
			return new SurrenderMessage(*this);
		default:
			return NULL;
	}
}

QByteArray MessageHeader::serialize() const {
	QByteArray result;
	result.append(QByteArray::fromRawData((const char*)&type,sizeof(MessageType)));
	result.append(QByteArray::fromRawData((const char*)&len,sizeof(qint64)));
	return result;
}

ChatMessage::ChatMessage(QString name, QString text, QColor color) {
	this->info.name = name;
	this->info.color = color;
	header.len = info.size();
	header.len += (text.toUtf8().size() + sizeof(int));
	header.type = mtChat;
	this->text = text;
}

QByteArray ChatMessage::serialize() const {
	QByteArray result;
	result.append(info.serialize());
	QByteArray tmp = text.toUtf8();
	int size = tmp.size();
	result.append(QByteArray::fromRawData((const char*)&size,sizeof(int)));
	result.append(tmp);
	return header.serialize().append(result);
}

void ChatMessage::fill(const QByteArray& buffer) {
	const char* data = buffer.data();
	info.fill(data);
	data += info.size();
	int textlen = *((int*)data);
	data += sizeof(int);
	text = QString::fromUtf8(data,textlen);
}

ChatMessage::ChatMessage(const MessageHeader&header) {this->header=header;}

PlayersListMessage::PlayersListMessage(const MessageHeader&header) {this->header=header;}

PlayersListMessage::PlayersListMessage(QList<ClientInfo> list) {
	this->list = list;
	header.len = sizeof(int);
	for (int i=0; i<list.size(); header.len += list[i++].size()) {}
	header.type = mtPlayersList;
}

QByteArray PlayersListMessage::serialize() const {
	QByteArray result;
	int size = list.size();
	result.append(QByteArray::fromRawData((const char*)&size,sizeof(int)));
	for (int i=0; i<list.size(); result.append(list[i++].serialize())) {}
	return header.serialize().append(result);
}

void PlayersListMessage::fill(const QByteArray& buffer) {
	const char* data = buffer.data();
	int count = *((int*)data);
	data+=sizeof(int);
	for (int i=0; i<count; ++i) {
		ClientInfo item;
		item.fill(data);
		data += item.size();
		list.append(item);
	}
}

QByteArray ClientMessage::serialize() const {
	QByteArray result;
	result.append(info.serialize());
	return header.serialize().append(result);
}

void ClientMessage::fill(const QByteArray& buffer) {
	const char* data = buffer.data();
	info.fill(data);
}

ClientConnectMessage::ClientConnectMessage(QString name, QColor color) {
	this->info.name = name;
	this->info.color = color;
	header.len = info.size();
	header.type = mtClientConnect;
}

ClientDisconnectMessage::ClientDisconnectMessage(QString name, QColor color) {
	this->info.name = name;
	this->info.color = color;
	header.len = info.size();
	header.type = mtClientDisconnect;
}

TurnMessage::TurnMessage(QString name, QColor color,QString tile, int id, int x, int y) {
	this->info.name = name;
	this->info.color = color;
	this->id = id;
	this->x = x;
	this->y = y;
	this->mask=tile;
	header.len = info.size()+4*sizeof(int)+mask.toUtf8().size();
	header.type = mtTurn;
}

QByteArray TurnMessage::serialize() const {
	QByteArray result = info.serialize();//ClientMessage::serialize();
	result.append(QByteArray::fromRawData((const char*)&id,sizeof(int)));
	result.append(QByteArray::fromRawData((const char*)&x,sizeof(int)));
	result.append(QByteArray::fromRawData((const char*)&y,sizeof(int)));
	QByteArray tmp = mask.toUtf8();
	int size = tmp.size();
	result.append(QByteArray::fromRawData((const char*)&size,sizeof(int)));
	result.append(tmp);
	return header.serialize().append(result);
}

void TurnMessage::fill(const QByteArray& buffer) {
	const char* data = buffer.data();
	info.fill(data);
	data+=info.size();
	id = *((int*)data);
	data+=sizeof(int);
	x = *((int*)data);
	data+=sizeof(int);
	y = *((int*)data);
	data+=sizeof(int);
	int textlen = *((int*)data);
	data += sizeof(int);
	mask = QString::fromUtf8(data,textlen);
}

TryToConnectMessage::TryToConnectMessage(ClientInfo info) {
	this->info = info;
	header.len = info.size();
	header.type = mtTryToConnect;
}

ConnectionAcceptedMessage::ConnectionAcceptedMessage(int errorCode) {
	this->errorCode = errorCode;
	header.len = sizeof(int);
	header.type = mtConnectionAccepted;
}

QByteArray ConnectionAcceptedMessage::serialize() const {
	QByteArray result = QByteArray::fromRawData((const char*)(&errorCode), sizeof(int));
	std::cerr << "created: " << errorCode << endl;
	return header.serialize().append(result);
}

void ConnectionAcceptedMessage::fill(const QByteArray& buffer) {
	const char* data = buffer.data();
	//errorCode = *((int*)data);
	::bcopy(data, (void*)(&errorCode), sizeof(int));
	std::cerr << "accepted: " << errorCode << endl;
}