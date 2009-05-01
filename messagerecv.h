#include <QObject>
#include <QList>
#include <QColor>
#include <QByteArray>
#include <QString>
#include "clientinfo.h"

enum MessageType { mtHeader, mtChat, mtPlayersList,
	mtClientConnect, mtClientDisconnect, mtServerReady,
	mtConnectionAccepted };

class TCPSocket;

class Message {
	protected:
		MessageType code;
	public:
		virtual qint64 getLength() const = 0;
		virtual void fill(const QByteArray&) = 0;
		virtual Message* next() const;
		virtual QByteArray serialize() const = 0;
		void send(TCPSocket*) const;
};

class MessageHeader : public Message {
	public:
		MessageType type;
		qint64 len;
		qint64 getLength() const { return sizeof(MessageType) + sizeof(qint64); }
		Message* next() const;
		QByteArray serialize() const;
		void fill(const QByteArray&);
};

class ComplexMessage : public Message {
	protected:
		MessageHeader header;
	public:
//		ComplexMessage(const MessageHeader& header) {this->header=header;}
		qint64 getLength() const {return header.len;}
};

class ChatMessage : public ComplexMessage {
	private:
		QString text;
		ClientInfo info;
	public:
		ChatMessage(const MessageHeader& header);
		QString getName() const {return info.name;}
		QString getText() const {return text;}
		QColor getColor() const {return info.color;}
		ChatMessage(QString name, QString text, QColor color);
		QByteArray serialize() const;
		void fill(const QByteArray&);
};

class PlayersListMessage : public ComplexMessage {
	private:
		QList<ClientInfo> list;
	public:
		PlayersListMessage(const MessageHeader& header);
		QList<ClientInfo> getList() const {return list;}
		PlayersListMessage(QList<ClientInfo>);
		QByteArray serialize() const;
		void fill(const QByteArray&);
};

class ClientMessage : public ComplexMessage {
	protected:
		ClientInfo info;
	public:
		QString getName() const {return info.name;}
		QColor getColor() const {return info.color;}
		ClientInfo getInfo() const {return info;}
		QByteArray serialize() const;
		void fill(const QByteArray&);
};

class ClientConnectMessage : public ClientMessage {
	public:
		ClientConnectMessage(const MessageHeader& header) {this->header=header;}
		ClientConnectMessage(QString, QColor);
};

class ClientDisconnectMessage : public ClientMessage {
	public:
		ClientDisconnectMessage(const MessageHeader& header) {this->header=header;}
		ClientDisconnectMessage(QString, QColor);
};

class ServerReadyMessage : public ComplexMessage {
	private:
	public:
		ServerReadyMessage(const MessageHeader& header) {this->header=header;}
		ServerReadyMessage();
		QByteArray serialize() const;
		void fill(const QByteArray&);
};

class ConnectionAcceptedMessage : public ComplexMessage {
	private:
		int errorCode;
	public:
		ConnectionAcceptedMessage(const MessageHeader& header) {this->header=header;}
		ConnectionAcceptedMessage(int errorCode);
		QByteArray serialize() const;
		void fill(const QByteArray&);
		int getCode() const {return errorCode;}
};

class MessageReceiver : public QObject {
	Q_OBJECT
	private:
		TCPSocket* socket;
		Message* current;
		QByteArray buffer;
	public:
		MessageReceiver(TCPSocket*);
		TCPSocket* getSocket() {return socket;}
	public slots:
		void readyRead();
	signals:
		void chatMessageReceive(ChatMessage);
		void playersListMessageReceive(PlayersListMessage);
		void clientConnectMessageReceive(ClientConnectMessage);
		void clientDisconnectMessageReceive(ClientDisconnectMessage);
		void getMessage(QByteArray);
		void serverReadyMessageReceive(ServerReadyMessage);
		void connectionAcceptedMessageReceive(ConnectionAcceptedMessage);
};